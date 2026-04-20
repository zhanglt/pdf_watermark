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
 * @file transfer.h
 * @brief 文件传输基类定义
 *
 * 本文件定义了LAN Share的网络传输核心类Transfer，它是Sender（发送端）和
 * Receiver（接收端）的基类，实现了TCP协议的文件传输功能。
 *
 * 核心功能：
 * - 定义了文件传输的数据包格式和类型
 * - 实现了TCP数据包的读取、解析和发送
 * - 管理传输状态和进度信息
 * - 提供传输控制接口（暂停、恢复、取消）
 *
 * 数据包协议格式：
 * [包大小(4字节)] + [包类型(1字节)] + [包数据(N字节)]
 *
 * 传输流程：
 * 1. Header包：发送文件元信息（名称、大小、路径等）
 * 2. Data包：分块传输文件数据
 * 3. Finish包：传输完成通知
 * 4. 中间可插入 Pause/Resume/Cancel 包进行控制
 */

#ifndef TRANSFER_H
#define TRANSFER_H

#include <QFile>
#include <QTcpSocket>
#include <QObject>

#include "../model/device.h"
#include "../model/transferinfo.h"

/**
 * @brief 数据包类型枚举
 *
 * 定义了传输协议中支持的所有数据包类型。
 * 每个数据包都以一个字节的类型标识符开头，用于区分不同的操作。
 */
enum class PacketType : char
{
    Header = 0x01,      ///< 文件头信息包，包含文件名、大小等元数据
    Data,               ///< 文件数据包，包含实际的文件内容
    Finish,             ///< 传输完成包，表示文件传输已完成
    Cancel,             ///< 取消传输包，终止当前传输
    Pause,              ///< 暂停传输包，暂停数据发送
    Resume,             ///< 恢复传输包，继续暂停的传输

    // 新增类型用于共享-拉取功能
    Command = 0x10,     ///< 命令请求包，用于共享浏览功能
    Response = 0x11     ///< 命令响应包，返回命令执行结果
};

/**
 * @brief 共享浏览命令类型枚举
 *
 * 用于共享-拉取功能，定义了可以执行的命令类型。
 * 这些命令允许远程浏览和获取共享的文件。
 */
enum class CommandType : char {
    LIST = 0x01,        ///< 列出可用的共享列表
    BROWSE = 0x02,      ///< 浏览共享文件夹的内容
    GET = 0x03,         ///< 获取（下载）指定文件
    SEARCH = 0x04,      ///< 在共享中搜索文件
    INFO = 0x05         ///< 获取文件或共享的详细信息
};

/**
 * @class Transfer
 * @brief 文件传输基类
 *
 * Transfer是文件传输功能的核心抽象类，实现了基于TCP协议的文件传输机制。
 * 它定义了数据包的发送、接收、解析流程，并管理传输状态。
 *
 * 主要职责：
 * 1. TCP Socket管理：建立连接、读取/写入数据
 * 2. 数据包协议：按照自定义格式打包和解包数据
 * 3. 状态管理：维护传输状态（等待、传输中、暂停、完成等）
 * 4. 流程控制：提供暂停、恢复、取消等控制接口
 *
 * 派生类：
 * - Sender：发送端实现，负责读取文件并发送
 * - Receiver：接收端实现，负责接收数据并写入文件
 *
 * 数据包格式说明：
 * - 包大小（4字节）：后续数据的字节数（不包含这4个字节本身）
 * - 包类型（1字节）：PacketType枚举值
 * - 包数据（N字节）：实际数据内容，根据包类型而定
 *
 * 使用示例：
 * @code
 * // 派生类需要实现具体的数据包处理逻辑
 * void MyReceiver::processHeaderPacket(QByteArray& data) {
 *     // 解析文件信息
 *     // 创建目标文件
 *     // 准备接收数据
 * }
 * @endcode
 */
class Transfer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Transfer 构造函数
     * @param socket TCP套接字，用于网络通信（可为nullptr，由派生类后续设置）
     * @param parent 父对象指针
     *
     * 初始化传输对象，设置socket连接和传输信息。
     * 如果socket不为空，会自动连接readyRead信号以接收数据。
     */
    Transfer(QTcpSocket* socket, QObject* parent = nullptr);

    /**
     * @brief 获取文件对象
     * @return 当前传输的文件对象指针
     *
     * 返回正在传输的文件对象，用于读取或写入文件数据。
     */
    inline QFile* getFile() const { return mFile; }

    /**
     * @brief 获取Socket对象
     * @return TCP套接字指针
     *
     * 返回用于网络通信的TCP socket对象。
     */
    inline QTcpSocket* getSocket() const { return mSocket; }

    /**
     * @brief 获取传输信息对象
     * @return 传输信息对象指针
     *
     * 返回TransferInfo对象，包含传输状态、进度、文件信息等。
     * UI层可以通过此对象监听传输进度和状态变化。
     */
    inline TransferInfo* getTransferInfo() const { return mInfo; }

    /**
     * @brief 恢复传输
     *
     * 从暂停状态恢复传输。
     * 默认为空实现，由派生类根据具体需求实现。
     */
    virtual void resume();

    /**
     * @brief 暂停传输
     *
     * 暂停当前正在进行的传输。
     * 默认为空实现，由派生类根据具体需求实现。
     */
    virtual void pause();

    /**
     * @brief 取消传输
     *
     * 取消当前传输，清理资源。
     * 默认为空实现，由派生类根据具体需求实现。
     */
    virtual void cancel();

    /**
     * @brief 处理初始数据包
     * @param buffer 包含初始数据包的缓冲区
     *
     * 用于处理协议识别后的初始数据包。
     * 主要用于TransferServer在识别协议后传递已读取的数据。
     */
    void processInitialBuffer(const QByteArray& buffer);

protected:
    /**
     * @brief 清空读取缓冲区
     *
     * 清除接收缓冲区中的所有数据，并重置包大小计数器。
     * 用于取消传输或错误恢复时清理状态。
     */
    void clearReadBuffer();

    /**
     * @brief 设置TCP套接字
     * @param socket TCP套接字指针
     *
     * 设置用于传输的socket，并自动连接readyRead信号。
     * 当socket有数据可读时，会触发onReadyRead槽函数。
     */
    void setSocket(QTcpSocket* socket);

    /**
     * @brief 处理接收到的数据包
     * @param data 数据包内容（不包含包头）
     * @param type 数据包类型
     *
     * 根据数据包类型，分发到相应的处理函数。
     * 这是数据包处理的核心分发器。
     */
    virtual void processPacket(QByteArray& data, PacketType type);

    /**
     * @brief 处理文件头信息包
     * @param data 包含文件元信息的JSON数据
     *
     * 解析文件头信息（文件名、大小、路径等）。
     * 默认为空实现，由派生类实现具体逻辑。
     */
    virtual void processHeaderPacket(QByteArray& data);

    /**
     * @brief 处理文件数据包
     * @param data 文件数据块
     *
     * 处理文件的实际数据内容。
     * Receiver中会将数据写入文件，Sender中不需要处理。
     */
    virtual void processDataPacket(QByteArray& data);

    /**
     * @brief 处理传输完成包
     * @param data 包数据（通常为空）
     *
     * 处理传输完成通知，关闭文件，更新状态。
     */
    virtual void processFinishPacket(QByteArray& data);

    /**
     * @brief 处理取消传输包
     * @param data 包数据（通常为空）
     *
     * 处理对方发送的取消请求，终止传输并清理资源。
     */
    virtual void processCancelPacket(QByteArray& data);

    /**
     * @brief 处理暂停传输包
     * @param data 包数据（通常为空）
     *
     * 处理对方发送的暂停请求，停止发送数据。
     */
    virtual void processPausePacket(QByteArray& data);

    /**
     * @brief 处理恢复传输包
     * @param data 包数据（通常为空）
     *
     * 处理对方发送的恢复请求，继续发送数据。
     */
    virtual void processResumePacket(QByteArray& data);

    /**
     * @brief 发送数据包
     * @param packetDataSize 数据部分的大小（字节数）
     * @param type 数据包类型
     * @param data 要发送的数据内容
     *
     * 按照协议格式打包并发送数据：
     * 1. 发送包大小（4字节）
     * 2. 发送包类型（1字节）
     * 3. 发送数据内容（N字节）
     */
    virtual void writePacket(qint32 packetDataSize, PacketType type, const QByteArray& data);

    QFile* mFile;               ///< 传输的文件对象（发送时读取，接收时写入）
    QTcpSocket* mSocket;        ///< TCP套接字，用于网络通信
    TransferInfo* mInfo;        ///< 传输信息对象，管理状态和进度

private Q_SLOTS:
    /**
     * @brief Socket数据就绪槽函数
     *
     * 当socket有数据可读时被触发，负责：
     * 1. 从socket读取所有可用数据到缓冲区
     * 2. 解析数据包头（包大小和类型）
     * 3. 当接收到完整数据包时，调用processPacket处理
     * 4. 支持粘包处理，一次可能接收多个完整包
     *
     * 数据包解析流程：
     * - 先读取4字节的包大小
     * - 再读取1字节的包类型
     * - 最后读取指定大小的数据内容
     * - 如果缓冲区数据不足，等待下次readyRead
     */
    void onReadyRead();

private:
    QByteArray mBuff;           ///< 接收数据缓冲区，存储从socket读取的原始数据
    qint32 mPacketSize;         ///< 当前数据包的大小（-1表示尚未读取包头）
    int mHeaderSize;            ///< 包头大小（包大小字段+包类型字段 = 5字节）
};

#endif // TRANSFER_H
