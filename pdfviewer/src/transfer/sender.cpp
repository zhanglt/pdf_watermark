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
 * @file sender.cpp
 * @brief 文件发送端实现
 *
 * 本文件实现了Sender类的所有功能，包括：
 * - 建立到接收端的TCP连接
 * - 发送文件头信息和文件数据
 * - 处理发送过程中的各种状态控制
 * - 响应接收端的控制命令
 * - 监控发送进度并更新UI
 *
 * 发送端工作流程：
 * 1. 构造Sender对象，传入接收端信息和文件路径
 * 2. 调用start()打开文件并连接到接收端
 * 3. 连接成功后发送Header包（文件元信息）
 * 4. 循环发送Data包（分块发送文件内容）
 * 5. 发送完毕后发送Finish包
 * 6. 关闭文件和连接
 *
 * 流量控制机制：
 * - 使用bytesWritten信号驱动发送流程
 * - 每次只发送固定大小的数据块（默认64KB）
 * - 等待上一块数据发送完毕才发送下一块
 * - 避免socket发送缓冲区积压过多数据
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QtDebug>

#include "../ui/settings.h"
#include "sender.h"

/**
 * @brief Sender 构造函数
 * @param receiver 接收端设备信息
 * @param folderName 目标文件夹名称
 * @param filePath 要发送的文件路径
 * @param parent 父对象指针
 *
 * 初始化发送端对象的所有成员变量：
 * - 保存接收端设备信息和文件路径
 * - 从Settings读取缓冲区大小并分配缓冲区
 * - 初始化所有状态标志为false
 * - 设置传输类型为Upload（上传）
 * - 设置对端设备信息
 *
 * 注意：构造函数不打开文件或建立连接，需要调用start()。
 */
Sender::Sender(const Device& receiver, const QString& folderName, const QString& filePath, QObject* parent)
    : Transfer(nullptr, parent),  // socket在start()中创建，这里传nullptr
      mReceiverDev(receiver),
      mFilePath(filePath),
      mFolderName(folderName)
{
    // 初始化文件大小和剩余字节数为-1（表示尚未初始化）
    mFileSize = -1;
    mBytesRemaining = -1;

    // 从设置中读取文件缓冲区大小（通常为64KB）
    mFileBuffSize = Settings::instance()->getFileBufferSize();
    // 预分配缓冲区，避免每次读取时重新分配
    mFileBuff.resize(mFileBuffSize);

    // 初始化所有状态标志
    mCancelled = false;              // 未取消
    mPaused = false;                 // 未暂停
    mPausedByReceiver = false;       // 未被接收端暂停
    mIsHeaderSent = false;           // 尚未发送Header

    // 设置传输信息：类型为上传，对端为接收设备
    mInfo->setTransferType(TransferType::Upload);
    mInfo->setPeer(receiver);
}

/**
 * @brief 开始发送文件
 * @return 成功返回true，失败返回false
 *
 * 启动文件发送流程，执行以下操作：
 * 1. 设置传输信息中的文件路径
 * 2. 打开要发送的文件（只读模式）
 * 3. 获取文件大小并初始化剩余字节数
 * 4. 发送fileOpened信号通知UI
 * 5. 创建TCP socket并连接到接收端
 * 6. 连接相关信号槽（bytesWritten, connected, disconnected）
 * 7. 设置传输状态为Waiting（等待连接）
 *
 * 返回条件：
 * - 成功：文件打开成功且socket创建成功
 * - 失败：文件打开失败或文件大小为0
 */
bool Sender::start()
{
    // 设置传输信息中的文件路径
    mInfo->setFilePath(mFilePath);

    // 创建文件对象并尝试打开（只读模式）
    mFile = new QFile(mFilePath, this);
    bool ok = mFile->open(QIODevice::ReadOnly);

    if (ok) {
        // 文件打开成功，获取文件大小
        mFileSize = mFile->size();
        mInfo->setDataSize(mFileSize);       // 设置传输数据大小
        mBytesRemaining = mFileSize;         // 初始化剩余字节数
        emit mInfo->fileOpened();            // 通知UI文件已打开
    }

    // 只有当文件大小大于0时才建立连接
    if (mFileSize > 0) {
        // 获取接收端的IP地址
        QHostAddress receiverAddress = mReceiverDev.getAddress();

        // 创建TCP socket并设置到基类
        setSocket(new QTcpSocket(this));

        // 连接到接收端的传输端口
        mSocket->connectToHost(receiverAddress,
                              Settings::instance()->getTransferPort(),
                              QAbstractSocket::ReadWrite);

        // 设置状态为等待连接
        mInfo->setState(TransferState::Waiting);

        // 连接socket的信号到槽函数
        connect(mSocket, &QTcpSocket::bytesWritten, this, &Sender::onBytesWritten);   // 数据写入完成
        connect(mSocket, &QTcpSocket::connected, this, &Sender::onConnected);         // 连接成功
        connect(mSocket, &QTcpSocket::disconnected, this, &Sender::onDisconnected);   // 连接断开
    }

    // 返回成功条件：文件打开成功且socket创建成功
    return ok && mSocket;
}

/**
 * @brief 恢复传输
 *
 * 从暂停状态恢复发送。
 * 检查当前状态是否允许恢复（通过mInfo->canResume()），如果可以：
 * 1. 将状态恢复到暂停前的状态（通常是Transfering）
 * 2. 清除mPaused标志
 * 3. 继续调用sendData()发送剩余数据
 *
 * canResume()返回true的条件：
 * - 当前状态为Paused（已暂停）
 */
void Sender::resume()
{
    if (mInfo->canResume()) {
        // 恢复到暂停前的状态
        mInfo->setState(mInfo->getLastState());
        // 清除暂停标志
        mPaused = false;
        // 继续发送数据
        sendData();
    }
}

/**
 * @brief 暂停传输
 *
 * 暂停当前传输。
 * 检查当前状态是否允许暂停（通过mInfo->canPause()），如果可以：
 * 1. 设置状态为Paused
 * 2. 设置mPaused标志为true
 *
 * mPaused标志会在sendData()中被检查，阻止继续发送数据。
 *
 * canPause()返回true的条件：
 * - 当前状态为Transfering（正在传输）
 */
void Sender::pause()
{
    if (mInfo->canPause()) {
        // 设置状态为暂停
        mInfo->setState(TransferState::Paused);
        // 设置暂停标志，sendData()会检查此标志
        mPaused = true;
    }
}

/**
 * @brief 取消传输
 *
 * 取消当前传输。
 * 检查当前状态是否允许取消（通过mInfo->canCancel()），如果可以：
 * 1. 发送Cancel包通知接收端
 * 2. 设置状态为Cancelled
 * 3. 重置进度为0
 * 4. 设置mCancelled标志为true
 *
 * mCancelled标志会在sendData()中被检查，阻止继续发送数据。
 *
 * canCancel()返回true的条件：
 * - 状态为Waiting、Transfering或Paused
 */
void Sender::cancel()
{
    if (mInfo->canCancel()) {
        // 发送取消包通知接收端
        writePacket(0, PacketType::Cancel, QByteArray());
        // 设置状态为已取消
        mInfo->setState(TransferState::Cancelled);
        // 重置进度为0
        mInfo->setProgress(0);
        // 设置取消标志，sendData()会检查此标志
        mCancelled = true;
    }
}

/**
 * @brief Socket连接成功槽函数
 *
 * 当TCP连接建立成功后自动触发。
 * 执行以下操作：
 * 1. 设置传输状态为Transfering（正在传输）
 * 2. 调用sendHeader()发送文件头信息
 *
 * 此时连接已建立，可以开始发送数据。
 */
void Sender::onConnected()
{
    // 设置状态为正在传输
    mInfo->setState(TransferState::Transfering);
    // 发送文件头信息包
    sendHeader();
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
 * - 接收端主动断开连接
 */
void Sender::onDisconnected()
{
    // 设置状态为已断开
    mInfo->setState(TransferState::Disconnected);
    // 发送错误信号通知UI
    emit mInfo->errorOcurred(tr("Receiver disconnected"));
}

/**
 * @brief Socket写入完成槽函数
 * @param bytes 本次写入的字节数
 *
 * 当socket的发送缓冲区中的数据被成功写入网络时触发。
 * 这是流量控制的关键函数，实现了"写完一块再发下一块"的机制。
 *
 * 工作原理：
 * - 检查socket的发送缓冲区是否已清空（bytesToWrite() == 0）
 * - 如果已清空，调用sendData()继续发送下一块数据
 * - 如果未清空，等待下次bytesWritten信号
 *
 * 这种机制可以：
 * - 防止socket缓冲区积压过多数据导致内存占用过高
 * - 避免发送速度过快导致接收端来不及处理
 * - 实现流畅的流量控制
 */
void Sender::onBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);  // 不关心具体写入了多少字节

    // 检查socket发送缓冲区是否已清空
    if (!mSocket->bytesToWrite()) {
        // 缓冲区已清空，继续发送下一块数据
        sendData();
    }
}

/**
 * @brief 完成传输
 *
 * 文件发送完成后的清理工作。
 * 执行以下操作：
 * 1. 关闭文件
 * 2. 设置状态为Finish（已完成）
 * 3. 发送done()信号通知UI层
 * 4. 发送Finish包通知接收端
 *
 * 发送Finish包后，接收端会关闭文件并断开连接。
 */
void Sender::finish()
{
    // 关闭文件
    mFile->close();
    // 设置状态为已完成
    mInfo->setState(TransferState::Finish);
    // 通知UI层传输完成
    emit mInfo->done();

    // 发送Finish包通知接收端
    writePacket(0, PacketType::Finish, QByteArray());
}

/**
 * @brief 发送文件数据
 *
 * 核心发送逻辑，每次发送一块文件数据。
 * 此函数不会循环发送，通过onBytesWritten()信号驱动下一次发送。
 *
 * 工作流程：
 * 1. 检查是否需要停止发送（剩余字节=0、已取消、被暂停）
 * 2. 如果是最后一块数据，调整缓冲区大小
 * 3. 从文件读取一块数据到缓冲区
 * 4. 更新剩余字节数
 * 5. 计算并更新传输进度（百分比）
 * 6. 将数据封装成Data包发送
 * 7. 如果全部发送完毕，调用finish()
 *
 * 停止条件：
 * - mBytesRemaining == 0：文件已发送完毕
 * - mCancelled：用户或接收端取消了传输
 * - mPausedByReceiver：接收端请求暂停
 * - mPaused：用户主动暂停
 *
 * 错误处理：
 * - 如果读取文件失败，发送errorOcurred信号
 */
void Sender::sendData()
{
    // 检查是否需要停止发送
    if (!mBytesRemaining || mCancelled || mPausedByReceiver || mPaused)
        return;

    // 如果剩余字节数小于缓冲区大小，调整缓冲区大小
    // 避免最后一块发送过多的空数据
    if (mBytesRemaining < mFileBuffSize) {
        mFileBuff.resize(mBytesRemaining);
        mFileBuffSize = mFileBuff.size();
    }

    // 从文件读取一块数据到缓冲区
    qint64 bytesRead = mFile->read(mFileBuff.data(), mFileBuffSize);
    if (bytesRead == -1) {
        // 读取失败，发送错误信号
        emit mInfo->errorOcurred(tr("Error while reading file."));
        return;
    }

    // 更新剩余字节数
    mBytesRemaining -= bytesRead;
    if (mBytesRemaining < 0)
        mBytesRemaining = 0;

    // 计算并更新传输进度（百分比）
    // 进度 = (已发送字节数 / 文件总大小) * 100
    mInfo->setProgress( (int) ((mFileSize-mBytesRemaining) * 100 / mFileSize) );

    // 将数据封装成Data包发送
    writePacket(mFileBuffSize, PacketType::Data, mFileBuff);

    // 如果全部发送完毕，调用finish()
    if (!mBytesRemaining) {
        finish();
    }
}

/**
 * @brief 发送文件头信息
 *
 * 构造并发送包含文件元信息的Header包。
 * 这是传输的第一步，接收端需要根据这些信息创建目标文件。
 *
 * Header包内容（JSON格式）：
 * - name：文件名（从文件路径中提取）
 * - folder：目标文件夹名称（接收端保存路径）
 * - size：文件大小（字节数）
 *
 * 发送Header包后设置mIsHeaderSent标志为true。
 * 接收端收到Header包后会创建文件并准备接收数据。
 */
void Sender::sendHeader()
{
    // 从文件路径中提取文件名
    QString fName = QDir(mFile->fileName()).dirName();

    // 构造JSON对象，包含文件元信息
    QJsonObject obj( QJsonObject::fromVariantMap({
                                    {"name", fName},           // 文件名
                                    {"folder", mFolderName },  // 目标文件夹
                                    {"size", mFileSize}        // 文件大小
                                }));

    // 将JSON对象转换为字节数组
    QByteArray headerData( QJsonDocument(obj).toJson() );

    // 发送Header包
    writePacket(headerData.size(), PacketType::Header, headerData);

    // 标记Header已发送
    mIsHeaderSent = true;
}

/**
 * @brief 处理取消传输包
 * @param data 包数据（通常为空）
 *
 * 接收端发送的取消请求。
 * 执行以下操作：
 * 1. 设置状态为Cancelled
 * 2. 重置进度为0
 * 3. 断开socket连接
 * 4. 设置mCancelled标志为true
 *
 * 设置mCancelled后，sendData()会停止发送数据。
 */
void Sender::processCancelPacket(QByteArray& data)
{
    Q_UNUSED(data);

    // 设置状态为已取消
    mInfo->setState(TransferState::Cancelled);
    // 重置进度
    mInfo->setProgress(0);
    // 断开连接
    mSocket->disconnectFromHost();
    // 设置取消标志
    mCancelled = true;
}

/**
 * @brief 处理暂停传输包
 * @param data 包数据（通常为空）
 *
 * 接收端发送的暂停请求。
 * 设置mPausedByReceiver标志为true。
 *
 * sendData()会检查此标志，阻止继续发送数据。
 * 当接收端发送Resume包时，会清除此标志并继续发送。
 */
void Sender::processPausePacket(QByteArray& data)
{
    Q_UNUSED(data);

    // 设置被接收端暂停标志
    mPausedByReceiver = true;
}

/**
 * @brief 处理恢复传输包
 * @param data 包数据（通常为空）
 *
 * 接收端发送的恢复请求。
 * 执行以下操作：
 * 1. 清除mPausedByReceiver标志
 * 2. 如果Header已发送，继续发送数据
 * 3. 如果Header未发送，先发送Header
 *
 * 这种判断是为了处理连接刚建立时接收端可能发送Resume包的情况。
 */
void Sender::processResumePacket(QByteArray& data)
{
    Q_UNUSED(data);

    // 清除被接收端暂停标志
    mPausedByReceiver = false;

    // 根据Header发送状态决定下一步操作
    if (mIsHeaderSent)
        sendData();      // Header已发送，继续发送数据
    else
        sendHeader();    // Header未发送，先发送Header
}

