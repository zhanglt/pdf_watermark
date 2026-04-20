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

/**
 * @file receiver.h
 * @brief 文件接收端类定义
 *
 * 本文件定义了Receiver类，它继承自Transfer基类，实现了文件接收端的功能。
 * Receiver负责接收来自发送端的文件数据并保存到本地。
 *
 * 核心功能：
 * - 被动接受发送端的TCP连接
 * - 接收并解析文件头信息（文件名、大小等）
 * - 创建目标文件并写入接收到的数据
 * - 处理接收过程中的暂停、恢复、取消操作
 * - 向发送端发送控制命令（暂停/恢复/取消）
 * - 监控接收进度并更新状态
 *
 * 接收流程：
 * 1. 构造时接受已建立的socket连接
 * 2. 接收Header包并解析文件信息
 * 3. 根据文件信息创建目标文件
 * 4. 循环接收Data包并写入文件
 * 5. 接收Finish包后关闭文件
 *
 * 与Sender的区别：
 * - Sender主动连接，Receiver被动接受连接
 * - Sender读取文件发送，Receiver接收数据写入文件
 * - Sender发送Header，Receiver解析Header
 */

#ifndef RECEIVER_H
#define RECEIVER_H

#include "transfer.h"
#include "../model/device.h"

/**
 * @class Receiver
 * @brief 文件接收端类
 *
 * Receiver继承自Transfer，实现了文件接收端的完整功能。
 * 它被动接受发送端的连接，接收文件数据并保存到本地。
 *
 * 工作原理：
 * 1. 构造时接受一个已建立的socket连接
 * 2. 接收Header包，解析文件名、大小等信息
 * 3. 在下载目录创建目标文件
 * 4. 循环接收Data包，将数据写入文件
 * 5. 接收Finish包后关闭文件并完成传输
 * 6. 过程中可响应暂停、恢复、取消等控制命令
 *
 * 文件保存策略：
 * - 保存到用户设置的下载目录
 * - 如果Header中指定了文件夹，创建子文件夹
 * - 如果文件已存在且未启用覆盖，自动重命名（添加数字后缀）
 *
 * 错误处理：
 * - 文件创建失败
 * - 文件写入失败
 * - 网络连接断开
 * - 发送端取消传输
 *
 * 使用示例：
 * @code
 * // TransferServer接受到连接后创建Receiver
 * QTcpSocket* socket = server->nextPendingConnection();
 * Device sender = ...;  // 发送端设备信息
 * Receiver* receiver = new Receiver(sender, socket);
 * // 接收会自动开始，无需调用start()
 * @endcode
 */
class Receiver : public Transfer
{
public:
    /**
     * @brief Receiver 构造函数
     * @param sender 发送端设备信息（包含IP地址、设备名等）
     * @param socket 已建立的TCP连接
     * @param parent 父对象指针
     *
     * 初始化接收端对象：
     * - 保存发送端设备信息
     * - 设置TCP socket（由TransferServer传入）
     * - 初始化文件大小和已接收字节数
     * - 设置传输类型为Download
     * - 连接disconnected信号
     * - 设置初始状态为Waiting
     *
     * 注意：构造函数完成后即可开始接收数据，无需调用start()。
     */
    Receiver(const Device& sender, QTcpSocket* socket, QObject* parent = nullptr);

    /**
     * @brief 获取发送端设备信息
     * @return 发送端设备对象
     *
     * 返回发送端的设备信息，包括设备名称、IP地址等。
     */
    inline Device getSender() const { return mSenderDev; }

    /**
     * @brief 获取文件总大小
     * @return 文件大小（字节数）
     *
     * 返回从Header包中获取的文件总大小。
     * 在接收Header包之前返回0。
     */
    inline qint64 getReceivedFileSize() const { return mFileSize; }

    /**
     * @brief 获取已写入的字节数
     * @return 已写入字节数
     *
     * 返回已接收并写入文件的字节数，用于计算进度。
     */
    inline qint64 getBytesWritten() const { return mBytesRead; }

    /**
     * @brief 恢复传输（重写基类方法）
     *
     * 从暂停状态恢复接收。
     * 检查当前状态是否允许恢复，如果可以：
     * - 恢复到之前的状态（通常是Transfering）
     * - 发送Resume包通知发送端继续发送
     */
    void resume() override;

    /**
     * @brief 暂停传输（重写基类方法）
     *
     * 暂停当前传输。
     * 检查当前状态是否允许暂停，如果可以：
     * - 设置状态为Paused
     * - 发送Pause包通知发送端停止发送
     */
    void pause() override;

    /**
     * @brief 取消传输（重写基类方法）
     *
     * 取消当前传输。
     * 检查当前状态是否允许取消，如果可以：
     * - 设置状态为Cancelled
     * - 重置进度为0
     * - 清空接收缓冲区
     * - 发送Cancel包通知发送端
     * - 删除未完成的文件
     */
    void cancel() override;

private Q_SLOTS:
    /**
     * @brief Socket断开连接槽函数
     *
     * 当TCP连接断开时触发（正常断开或异常断开）。
     * - 设置状态为Disconnected
     * - 发送错误信号通知UI层
     */
    void onDisconnected();

private:
    /**
     * @brief 处理文件头信息包（重写基类方法）
     * @param data 包含文件元信息的JSON数据
     *
     * 解析Header包中的文件信息并创建目标文件。
     *
     * 处理流程：
     * 1. 从JSON数据中解析文件名、大小、文件夹名
     * 2. 确定目标文件路径（下载目录 + 文件夹 + 文件名）
     * 3. 如果目标文件夹不存在，创建文件夹
     * 4. 如果文件已存在且未启用覆盖，重命名文件
     * 5. 创建并打开目标文件（写入模式）
     * 6. 设置状态为Transfering
     * 7. 发送fileOpened信号通知UI
     *
     * 错误处理：
     * - 如果文件创建失败，发送errorOcurred信号
     */
    void processHeaderPacket(QByteArray& data) override;

    /**
     * @brief 处理文件数据包（重写基类方法）
     * @param data 文件数据块
     *
     * 将接收到的数据写入文件。
     *
     * 处理流程：
     * 1. 检查文件对象是否有效
     * 2. 检查已接收字节数是否超过文件大小
     * 3. 将数据写入文件
     * 4. 更新已接收字节数
     * 5. 计算并更新传输进度（百分比）
     *
     * 安全检查：
     * - 确保不会写入超过文件大小的数据
     * - 防止缓冲区溢出
     */
    void processDataPacket(QByteArray& data) override;

    /**
     * @brief 处理传输完成包（重写基类方法）
     * @param data 包数据（通常为空）
     *
     * 处理传输完成通知。
     *
     * 处理流程：
     * 1. 设置状态为Finish
     * 2. 关闭文件
     * 3. 断开socket连接
     * 4. 发送done()信号通知UI层
     */
    void processFinishPacket(QByteArray& data) override;

    /**
     * @brief 处理取消传输包（重写基类方法）
     * @param data 包数据（通常为空）
     *
     * 发送端发送的取消请求。
     *
     * 处理流程：
     * 1. 设置状态为Cancelled
     * 2. 重置进度为0
     * 3. 清空接收缓冲区
     * 4. 删除未完成的文件
     * 5. 断开socket连接
     */
    void processCancelPacket(QByteArray& data) override;

    Device mSenderDev;      ///< 发送端设备信息（IP、端口、名称等）
    qint64 mFileSize;       ///< 文件总大小（字节），从Header包中获取
    qint64 mBytesRead;      ///< 已接收并写入文件的字节数
};

#endif // RECEIVER_H
