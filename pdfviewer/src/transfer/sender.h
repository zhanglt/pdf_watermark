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
 * @file sender.h
 * @brief 文件发送端类定义
 *
 * 本文件定义了Sender类，它继承自Transfer基类，实现了文件发送端的功能。
 * Sender负责读取本地文件并通过TCP连接发送到接收端。
 *
 * 核心功能：
 * - 建立到接收端的TCP连接
 * - 发送文件头信息（文件名、大小等）
 * - 分块读取并发送文件数据
 * - 处理发送过程中的暂停、恢复、取消操作
 * - 响应接收端的控制命令（暂停/恢复/取消）
 * - 监控发送进度并更新状态
 *
 * 发送流程：
 * 1. start() - 打开文件，创建socket，连接到接收端
 * 2. onConnected() - 连接成功后发送文件头
 * 3. sendHeader() - 发送包含文件元信息的Header包
 * 4. sendData() - 循环读取文件块并发送Data包
 * 5. finish() - 发送完成后发送Finish包，关闭文件
 *
 * 状态控制：
 * - 支持用户主动暂停/恢复/取消
 * - 支持接收端请求暂停/取消
 * - 通过标志位控制发送流程
 */

#ifndef SENDER_H
#define SENDER_H

#include "transfer.h"
#include "../model/device.h"

/**
 * @class Sender
 * @brief 文件发送端类
 *
 * Sender继承自Transfer，实现了文件发送端的完整功能。
 * 它负责主动连接到接收端，并将本地文件通过TCP协议发送过去。
 *
 * 工作原理：
 * 1. 构造时接收文件路径和接收端设备信息
 * 2. start()时打开文件并连接到接收端
 * 3. 连接成功后先发送Header包（包含文件名、大小等）
 * 4. 然后循环发送Data包，每次发送一个固定大小的数据块
 * 5. 发送完毕后发送Finish包通知接收端
 * 6. 过程中可响应暂停、恢复、取消等控制命令
 *
 * 性能优化：
 * - 使用缓冲区批量读取文件，减少IO操作
 * - 利用Qt的异步写入机制，避免阻塞
 * - 通过bytesWritten信号控制发送速度，防止内存积压
 *
 * 错误处理：
 * - 文件打开失败
 * - 网络连接失败或断开
 * - 接收端取消传输
 *
 * 使用示例：
 * @code
 * Device receiver = ...;  // 接收端设备信息
 * Sender* sender = new Sender(receiver, "MyFolder", "/path/to/file.txt");
 * sender->start();  // 开始发送
 * @endcode
 */
class Sender : public Transfer
{
public:
    /**
     * @brief Sender 构造函数
     * @param receiver 接收端设备信息（包含IP地址、端口等）
     * @param folderName 目标文件夹名称（接收端保存文件的相对路径）
     * @param filePath 要发送的文件完整路径
     * @param parent 父对象指针
     *
     * 初始化发送端对象，保存接收端信息和文件路径。
     * 设置传输类型为Upload，并配置文件缓冲区大小。
     *
     * 注意：构造函数不会打开文件或建立连接，需要调用start()才会开始传输。
     */
    Sender(const Device& receiver, const QString& folderName, const QString& filePath, QObject* parent = nullptr);

    /**
     * @brief 开始发送文件
     * @return 成功返回true，失败返回false
     *
     * 执行以下操作：
     * 1. 打开要发送的文件（只读模式）
     * 2. 获取文件大小并初始化传输信息
     * 3. 创建TCP socket并连接到接收端
     * 4. 连接相关信号槽（bytesWritten, connected, disconnected）
     * 5. 设置传输状态为Waiting（等待连接）
     *
     * 失败原因：
     * - 文件不存在或无法打开
     * - 文件大小为0
     * - socket创建失败
     */
    bool start();

    /**
     * @brief 获取接收端设备信息
     * @return 接收端设备对象
     *
     * 返回接收端的设备信息，包括设备名称、IP地址等。
     */
    Device getReceiver() const { return mReceiverDev; }

    /**
     * @brief 恢复传输（重写基类方法）
     *
     * 从暂停状态恢复发送。
     * 检查当前状态是否允许恢复，如果可以则：
     * - 恢复到之前的状态（通常是Transfering）
     * - 清除暂停标志
     * - 继续调用sendData()发送剩余数据
     */
    void resume() override;

    /**
     * @brief 暂停传输（重写基类方法）
     *
     * 暂停当前传输。
     * 检查当前状态是否允许暂停，如果可以则：
     * - 设置状态为Paused
     * - 设置暂停标志，阻止sendData()继续发送
     */
    void pause() override;

    /**
     * @brief 取消传输（重写基类方法）
     *
     * 取消当前传输。
     * 检查当前状态是否允许取消，如果可以则：
     * - 发送Cancel包通知接收端
     * - 设置状态为Cancelled
     * - 重置进度为0
     * - 设置取消标志，阻止后续操作
     */
    void cancel() override;

private Q_SLOTS:
    /**
     * @brief Socket写入完成槽函数
     * @param bytes 本次写入的字节数
     *
     * 当socket的发送缓冲区中的数据被成功写入网络时触发。
     * 检查socket缓冲区是否已清空，如果是则继续发送下一块数据。
     *
     * 这是流量控制的关键：只有当前数据发送完毕才发送新数据，
     * 避免socket缓冲区积压过多数据导致内存占用过高。
     */
    void onBytesWritten(qint64 bytes);

    /**
     * @brief Socket连接成功槽函数
     *
     * 当TCP连接建立成功后触发。
     * - 设置传输状态为Transfering（正在传输）
     * - 调用sendHeader()发送文件头信息
     */
    void onConnected();

    /**
     * @brief Socket断开连接槽函数
     *
     * 当TCP连接断开时触发（可能是正常断开或异常断开）。
     * - 设置状态为Disconnected
     * - 发送错误信号通知UI层
     */
    void onDisconnected();

private:
    /**
     * @brief 完成传输
     *
     * 传输完成后的清理工作：
     * - 关闭文件
     * - 设置状态为Finish
     * - 发送done()信号通知完成
     * - 发送Finish包通知接收端
     */
    void finish();

    /**
     * @brief 发送文件数据
     *
     * 核心发送逻辑，循环调用直到文件发送完毕。
     *
     * 工作流程：
     * 1. 检查是否需要停止发送（剩余字节=0、已取消、被暂停）
     * 2. 从文件读取一块数据到缓冲区（默认64KB）
     * 3. 如果是最后一块，调整缓冲区大小
     * 4. 将数据封装成Data包发送
     * 5. 更新剩余字节数和传输进度
     * 6. 如果全部发送完毕，调用finish()
     *
     * 注意：此函数不会循环发送，每次只发送一块。
     * 通过onBytesWritten()信号驱动下一次发送，实现流量控制。
     */
    void sendData();

    /**
     * @brief 发送文件头信息
     *
     * 构造并发送包含文件元信息的Header包。
     *
     * Header包内容（JSON格式）：
     * - name：文件名
     * - folder：目标文件夹名称
     * - size：文件大小（字节）
     *
     * 发送Header包后设置mIsHeaderSent标志为true。
     */
    void sendHeader();

    /**
     * @brief 处理取消传输包（重写基类方法）
     * @param data 包数据（通常为空）
     *
     * 接收端发送的取消请求。
     * - 设置状态为Cancelled
     * - 重置进度为0
     * - 断开socket连接
     * - 设置取消标志
     */
    void processCancelPacket(QByteArray& data) override;

    /**
     * @brief 处理暂停传输包（重写基类方法）
     * @param data 包数据（通常为空）
     *
     * 接收端发送的暂停请求。
     * 设置mPausedByReceiver标志为true，阻止sendData()继续发送。
     */
    void processPausePacket(QByteArray& data) override;

    /**
     * @brief 处理恢复传输包（重写基类方法）
     * @param data 包数据（通常为空）
     *
     * 接收端发送的恢复请求。
     * - 清除mPausedByReceiver标志
     * - 如果Header已发送，继续发送数据
     * - 如果Header未发送，发送Header包
     */
    void processResumePacket(QByteArray& data) override;

    Device mReceiverDev;        ///< 接收端设备信息（IP、端口、名称等）
    QString mFilePath;          ///< 要发送的文件完整路径
    QString mFolderName;        ///< 接收端保存文件的目标文件夹名称
    qint64 mFileSize;           ///< 文件总大小（字节）
    qint64 mBytesRemaining;     ///< 剩余未发送的字节数

    QByteArray mFileBuff;       ///< 文件读取缓冲区，存储每次读取的数据块
    qint32 mFileBuffSize;       ///< 缓冲区大小（从Settings读取，通常为64KB）

    bool mCancelled;            ///< 是否已取消（用户主动取消或接收端取消）
    bool mPaused;               ///< 是否已暂停（用户主动暂停）
    bool mPausedByReceiver;     ///< 是否被接收端暂停
    bool mIsHeaderSent;         ///< Header包是否已发送
};

#endif // SENDER_H
