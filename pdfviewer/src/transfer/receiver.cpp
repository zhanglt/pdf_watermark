/*
    LANShare - LAN file transfer.
    Copyright (C) 2016 Abdul Aris R. <abdularisrahmanudin10@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8，支持中文

/**
 * @file receiver.cpp
 * @brief 文件接收端实现
 *
 * 本文件实现了Receiver类的所有功能，包括：
 * - 接收并解析文件头信息
 * - 创建目标文件和文件夹
 * - 接收文件数据并写入文件
 * - 处理接收过程中的各种状态控制
 * - 监控接收进度并更新UI
 *
 * 接收端工作流程：
 * 1. TransferServer接受连接后创建Receiver对象
 * 2. 自动开始接收数据（无需调用start）
 * 3. 接收Header包，解析文件信息并创建文件
 * 4. 循环接收Data包，将数据写入文件
 * 5. 接收Finish包后关闭文件并完成
 *
 * 与发送端的协作：
 * - 被动接受连接（发送端主动连接）
 * - 接收发送端发送的各种数据包
 * - 发送控制包（Pause/Resume/Cancel）到发送端
 * - 响应发送端的控制请求
 */

#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

#include "../ui/util.h"
#include "receiver.h"
#include "../ui/settings.h"

/**
 * @brief Receiver 构造函数
 * @param sender 发送端设备信息
 * @param socket 已建立的TCP连接
 * @param parent 父对象指针
 *
 * 初始化接收端对象：
 * - 调用基类构造函数并传入socket（已由TransferServer建立）
 * - 保存发送端设备信息
 * - 初始化文件大小和已接收字节数为0
 * - 设置初始状态为Waiting（等待接收Header）
 * - 连接disconnected信号以处理连接断开
 * - 设置传输类型为Download（下载）
 * - 设置对端设备信息
 *
 * 注意：构造完成后即可开始接收，无需调用start()。
 * socket的readyRead信号已在基类Transfer中连接。
 */
Receiver::Receiver(const Device& sender, QTcpSocket* socket, QObject* parent)
    : Transfer(socket, parent),  // 传入已建立的socket到基类
      mSenderDev(sender),
      mFileSize(0),
      mBytesRead(0)
{
    // 设置初始状态为等待（等待接收Header包）
    mInfo->setState(TransferState::Waiting);

    // 连接socket的disconnected信号
    connect(mSocket, &QTcpSocket::disconnected, this, &Receiver::onDisconnected);

    // 设置传输信息：类型为下载，对端为发送设备
    mInfo->setTransferType(TransferType::Download);
    mInfo->setPeer(sender);
}

/**
 * @brief 恢复传输
 *
 * 从暂停状态恢复接收。
 * 检查当前状态是否允许恢复（通过mInfo->canResume()），如果可以：
 * 1. 将状态恢复到暂停前的状态（通常是Transfering）
 * 2. 发送Resume包通知发送端继续发送数据
 *
 * 发送端收到Resume包后会继续发送剩余的数据。
 *
 * canResume()返回true的条件：
 * - 当前状态为Paused（已暂停）
 */
void Receiver::resume()
{
    if (mInfo->canResume()) {
        // 恢复到暂停前的状态
        mInfo->setState(mInfo->getLastState());
        // 发送Resume包通知发送端继续发送
        writePacket(0, PacketType::Resume, QByteArray());
    }
}

/**
 * @brief 暂停传输
 *
 * 暂停当前传输。
 * 检查当前状态是否允许暂停（通过mInfo->canPause()），如果可以：
 * 1. 设置状态为Paused
 * 2. 发送Pause包通知发送端停止发送数据
 *
 * 发送端收到Pause包后会停止发送数据，直到收到Resume包。
 *
 * canPause()返回true的条件：
 * - 当前状态为Transfering（正在传输）
 */
void Receiver::pause()
{
    if (mInfo->canPause()) {
        // 设置状态为暂停
        mInfo->setState(TransferState::Paused);
        // 发送Pause包通知发送端停止发送
        writePacket(0, PacketType::Pause, QByteArray());
    }
}

/**
 * @brief 取消传输
 *
 * 取消当前传输。
 * 检查当前状态是否允许取消（通过mInfo->canCancel()），如果可以：
 * 1. 设置状态为Cancelled
 * 2. 重置进度为0
 * 3. 清空接收缓冲区（丢弃未处理的数据）
 * 4. 发送Cancel包通知发送端
 * 5. 删除未完成的文件
 *
 * 删除文件是为了避免留下不完整的文件。
 *
 * canCancel()返回true的条件：
 * - 状态为Waiting、Transfering或Paused
 */
void Receiver::cancel()
{
    if (mInfo->canCancel()) {
        // 设置状态为已取消
        mInfo->setState(TransferState::Cancelled);
        // 重置进度为0
        mInfo->setProgress(0);
        // 清空接收缓冲区
        clearReadBuffer();
        // 发送Cancel包通知发送端
        writePacket(0, PacketType::Cancel, QByteArray());
        // 删除未完成的文件
        mFile->remove();
    }
}

/**
 * @brief Socket断开连接槽函数
 *
 * 当TCP连接断开时触发（正常断开或异常断开）。
 * 执行以下操作：
 * 1. 设置状态为Disconnected（已断开）
 * 2. 发送errorOcurred信号通知UI层
 *
 * 断开原因可能是：
 * - 传输完成后正常断开
 * - 网络故障导致异常断开
 * - 发送端主动断开连接
 */
void Receiver::onDisconnected()
{
    // 设置状态为已断开
    mInfo->setState(TransferState::Disconnected);
    // 发送错误信号通知UI
    emit mInfo->errorOcurred("Sender disconnected");
}

/**
 * @brief 处理文件头信息包
 * @param data 包含文件元信息的JSON数据
 *
 * 解析Header包中的文件信息并创建目标文件。
 * 这是接收端收到的第一个数据包，包含文件的元信息。
 *
 * 处理流程：
 * 1. 从JSON数据中解析文件信息（名称、大小、文件夹）
 * 2. 设置传输数据大小
 * 3. 确定目标文件路径：下载目录 + 文件夹 + 文件名
 * 4. 如果目标文件夹不存在，创建文件夹及其父目录
 * 5. 如果文件已存在且未启用覆盖，生成唯一文件名（添加数字后缀）
 * 6. 创建并打开目标文件（写入模式）
 * 7. 设置状态为Transfering（正在传输）
 * 8. 发送fileOpened信号通知UI
 *
 * JSON数据格式：
 * {
 *   "name": "文件名",
 *   "size": 文件大小（字节数）,
 *   "folder": "目标文件夹名"（可选）
 * }
 *
 * 文件路径生成示例：
 * - 下载目录: C:/Downloads
 * - 文件夹名: MyFolder
 * - 文件名: test.txt
 * - 最终路径: C:/Downloads/MyFolder/test.txt
 *
 * 错误处理：
 * - 如果文件创建失败，发送errorOcurred信号
 */
void Receiver::processHeaderPacket(QByteArray& data)
{
    // 从JSON数据解析文件信息
    QJsonObject obj = QJsonDocument::fromJson(data).object();

    // 获取文件大小
    mFileSize = obj.value("size").toVariant().value<qint64>();
    mInfo->setDataSize(mFileSize);

    // 获取文件名和文件夹名
    QString fileName = obj.value("name").toString();
    QString folderName = obj.value("folder").toString();

    // 确定目标文件夹路径
    QString dstFolderPath = Settings::instance()->getDownloadDir();
    if (!folderName.isEmpty())
        dstFolderPath = dstFolderPath + QDir::separator() + folderName;

    // 如果目标文件夹不存在，创建文件夹
    QDir dir(dstFolderPath);
    if (!dir.exists()) {
        dir.mkpath(dstFolderPath);  // 递归创建所有父目录
    }

    // 构造完整的目标文件路径
    QString dstFilePath = dstFolderPath + QDir::separator() + fileName;

    // 如果未启用覆盖选项且文件已存在，生成唯一文件名
    // 例如：test.txt -> test(1).txt -> test(2).txt
    if (!Settings::instance()->getReplaceExistingFile()) {
        dstFilePath = Util::getUniqueFileName(fileName, dstFolderPath);
    }

    // 设置传输信息中的文件路径
    mInfo->setFilePath(dstFilePath);

    // 创建文件对象并尝试打开（写入模式）
    mFile = new QFile(dstFilePath, this);
    if (mFile->open(QIODevice::WriteOnly)) {
        // 文件打开成功，设置状态为正在传输
        mInfo->setState(TransferState::Transfering);
        // 通知UI文件已打开
        emit mInfo->fileOpened();
    }
    else {
        // 文件打开失败，发送错误信号
        emit mInfo->errorOcurred(tr("Failed to write ") + dstFilePath);
    }
}

/**
 * @brief 处理文件数据包
 * @param data 文件数据块
 *
 * 将接收到的数据写入文件并更新进度。
 *
 * 处理流程：
 * 1. 检查文件对象是否有效
 * 2. 检查写入后的总字节数是否超过文件大小（防止缓冲区溢出）
 * 3. 将数据写入文件
 * 4. 更新已接收字节数
 * 5. 计算并更新传输进度（百分比）
 *
 * 安全检查：
 * - 确保 mBytesRead + data.size() <= mFileSize
 * - 防止恶意发送端发送超过预期大小的数据
 * - 如果检查失败，不写入数据也不更新进度
 *
 * 进度计算：
 * 进度 = (已接收字节数 / 文件总大小) * 100
 */
void Receiver::processDataPacket(QByteArray& data)
{
    // 安全检查：文件有效且不会写入超过文件大小的数据
    if (mFile && mBytesRead + data.size() <= mFileSize) {
        // 将数据写入文件
        mFile->write(data);

        // 更新已接收字节数
        mBytesRead += data.size();

        // 计算并更新传输进度（百分比）
        mInfo->setProgress( (int)(mBytesRead * 100 / mFileSize) );
    }
}

/**
 * @brief 处理传输完成包
 * @param data 包数据（通常为空）
 *
 * 处理发送端发送的传输完成通知。
 *
 * 处理流程：
 * 1. 设置状态为Finish（已完成）
 * 2. 关闭文件
 * 3. 断开socket连接
 * 4. 发送done()信号通知UI层
 *
 * 此时文件已完整接收并保存到磁盘。
 */
void Receiver::processFinishPacket(QByteArray& data)
{
    Q_UNUSED(data);

    // 设置状态为已完成
    mInfo->setState(TransferState::Finish);
    // 关闭文件
    mFile->close();
    // 断开连接
    mSocket->disconnectFromHost();
    // 通知UI层传输完成
    emit mInfo->done();
}

/**
 * @brief 处理取消传输包
 * @param data 包数据（通常为空）
 *
 * 处理发送端发送的取消请求。
 *
 * 处理流程：
 * 1. 设置状态为Cancelled（已取消）
 * 2. 重置进度为0
 * 3. 清空接收缓冲区（丢弃未处理的数据）
 * 4. 删除未完成的文件
 * 5. 断开socket连接
 *
 * 删除文件是为了避免留下不完整的文件。
 * 用户可以看到传输已取消，不会误以为文件已完整接收。
 */
void Receiver::processCancelPacket(QByteArray& data)
{
    Q_UNUSED(data);

    // 设置状态为已取消
    mInfo->setState(TransferState::Cancelled);
    // 重置进度为0
    mInfo->setProgress(0);
    // 清空接收缓冲区
    clearReadBuffer();
    // 删除未完成的文件
    mFile->remove();
    // 断开连接
    mSocket->disconnectFromHost();
}
