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
 * @file transfer.cpp
 * @brief 文件传输基类实现
 *
 * 本文件实现了Transfer基类的核心功能，包括：
 * - TCP数据包的接收、解析和发送
 * - 数据包协议的实现（包头+数据格式）
 * - 数据包分发和处理框架
 * - 传输控制接口（暂停、恢复、取消）
 *
 * 数据包处理流程：
 * 1. onReadyRead() 接收TCP数据到缓冲区
 * 2. 解析包头获取包大小和包类型
 * 3. 提取完整数据包并调用 processPacket()
 * 4. processPacket() 根据类型分发到具体处理函数
 * 5. 派生类重写具体处理函数实现业务逻辑
 */

#include "transfer.h"

/**
 * @brief Transfer 构造函数
 * @param socket TCP套接字，用于网络通信（可为nullptr）
 * @param parent 父对象指针
 *
 * 初始化传输对象的各个成员变量：
 * - 创建TransferInfo对象用于管理传输状态和进度
 * - 设置TCP socket（如果提供）
 * - 初始化数据包解析相关变量
 * - 计算包头大小（PacketType 1字节 + PacketSize 4字节 = 5字节）
 */
Transfer::Transfer(QTcpSocket* socket, QObject* parent)
    : QObject(parent), mFile(nullptr), mSocket(nullptr),
      mPacketSize(-1)  // -1表示当前没有正在接收的数据包
{
    // 创建传输信息对象，用于管理状态、进度等信息
    mInfo = new TransferInfo(this, this);

    // 设置socket并连接信号（如果socket不为空）
    setSocket(socket);

    // 计算数据包头部大小：类型字段(1字节) + 大小字段(4字节)
    mHeaderSize = sizeof(PacketType) + sizeof(mPacketSize);
}

/**
 * @brief 恢复传输
 *
 * 从暂停状态恢复传输。
 * 基类提供空实现，派生类需要重写此方法实现具体的恢复逻辑：
 * - Sender：继续发送剩余数据，发送Resume包通知接收方
 * - Receiver：发送Resume包通知发送方继续发送
 */
void Transfer::resume()
{
    // 空实现，由派生类重写
}

/**
 * @brief 暂停传输
 *
 * 暂停当前正在进行的传输。
 * 基类提供空实现，派生类需要重写此方法实现具体的暂停逻辑：
 * - Sender：停止发送数据，发送Pause包通知接收方
 * - Receiver：发送Pause包通知发送方停止发送
 */
void Transfer::pause()
{
    // 空实现，由派生类重写
}

/**
 * @brief 取消传输
 *
 * 取消当前传输，清理资源。
 * 基类提供空实现，派生类需要重写此方法实现具体的取消逻辑：
 * - Sender：停止发送，发送Cancel包，断开连接
 * - Receiver：发送Cancel包，删除未完成的文件，断开连接
 */
void Transfer::cancel()
{
    // 空实现，由派生类重写
}

/**
 * @brief Socket数据就绪槽函数
 *
 * 当TCP socket接收到数据时自动触发，实现了完整的数据包接收和解析流程。
 *
 * 工作流程：
 * 1. 检查传输状态，如果已取消则忽略后续数据
 * 2. 从socket读取所有可用数据追加到接收缓冲区
 * 3. 循环处理缓冲区中的数据包（支持粘包处理）：
 *    a. 如果尚未读取包大小，先从缓冲区提取4字节的包大小
 *    b. 检查缓冲区是否包含完整的数据包（类型+数据）
 *    c. 如果数据完整，提取包类型和数据内容
 *    d. 调用processPacket()分发处理
 *    e. 从缓冲区移除已处理的数据
 *    f. 重置包大小为-1，准备接收下一个包
 * 4. 如果数据不完整，退出循环等待下次readyRead
 *
 * 数据包解析细节：
 * - 包大小字段（4字节）：表示后续数据的长度（包类型1字节+实际数据N字节）
 * - 包类型字段（1字节）：PacketType枚举值
 * - 包数据字段（N字节）：实际的数据内容
 *
 * 粘包处理：
 * TCP是流式协议，一次readyRead可能接收到多个完整包或不完整包。
 * 通过while循环和缓冲区管理，可以正确处理各种情况：
 * - 一次接收多个完整包：循环处理直到缓冲区数据不足
 * - 接收不完整包：保留在缓冲区，等待下次接收
 * - 接收部分包+完整包：先处理完整包，保留不完整部分
 */
void Transfer::onReadyRead()
{
    // 如果传输已被取消，忽略后续接收的数据
    if (mInfo->getState() == TransferState::Cancelled)
        return;

    // 从socket读取所有可用数据，追加到接收缓冲区
    mBuff.append(mSocket->readAll());

    // 循环处理缓冲区中的所有完整数据包
    // 只要缓冲区数据大于等于包头大小，就可能存在完整的包
    while (mBuff.size() >= mHeaderSize) {
        // 步骤1：读取包大小（如果还没有读取）
        if (mPacketSize < 0) {
            // 从缓冲区前4个字节提取包大小
            memcpy(&mPacketSize, mBuff.constData(), sizeof(mPacketSize));
            // 从缓冲区移除已读取的包大小字段
            mBuff.remove(0, sizeof(mPacketSize));
        }

        // 步骤2：检查缓冲区是否包含完整的包数据（类型+数据）
        if (mBuff.size() > mPacketSize) {
            // 提取包类型（第1个字节）
            PacketType type = static_cast<PacketType>(mBuff.at(0));

            // 提取包数据（从第2个字节开始，长度为mPacketSize）
            QByteArray data = mBuff.mid(1, mPacketSize);

            // 步骤3：分发数据包到相应的处理函数
            processPacket(data, type);

            // 步骤4：从缓冲区移除已处理的数据（类型1字节+数据N字节）
            mBuff.remove(0, mPacketSize + 1);

            // 步骤5：重置包大小，准备接收下一个数据包
            mPacketSize = -1;
        }
        else {
            // 数据不完整，退出循环等待下次readyRead接收更多数据
            break;
        }
    }
}

/**
 * @brief 发送数据包
 * @param packetDataSize 数据部分的大小（字节数）
 * @param type 数据包类型
 * @param data 要发送的数据内容
 *
 * 按照协议格式组装并发送数据包。
 *
 * 数据包格式（发送顺序）：
 * 1. 包大小（4字节）：packetDataSize的值
 * 2. 包类型（1字节）：type枚举值
 * 3. 包数据（N字节）：实际数据内容
 *
 * 注意：
 * - 包大小字段的值等于 sizeof(type) + data.size()
 * - 使用reinterpret_cast将整数转换为字节流
 * - Qt的QTcpSocket会自动处理数据的实际发送
 */
void Transfer::writePacket(qint32 packetDataSize, PacketType type, const QByteArray &data)
{
    if (mSocket) {
        // 发送包大小（4字节）
        mSocket->write(reinterpret_cast<const char*>(&packetDataSize), sizeof(packetDataSize));
        // 发送包类型（1字节）
        mSocket->write(reinterpret_cast<const char*>(&type), sizeof(type));
        // 发送包数据（N字节）
        mSocket->write(data);
    }
}

/**
 * @brief 处理接收到的数据包（分发器）
 * @param data 数据包内容（不包含包头）
 * @param type 数据包类型
 *
 * 根据数据包类型，将数据分发到相应的处理函数。
 * 这是一个核心的分发器方法，实现了策略模式。
 *
 * 支持的包类型和对应的处理函数：
 * - Header：processHeaderPacket() - 处理文件头信息
 * - Data：processDataPacket() - 处理文件数据
 * - Finish：processFinishPacket() - 处理传输完成
 * - Cancel：processCancelPacket() - 处理取消请求
 * - Pause：processPausePacket() - 处理暂停请求
 * - Resume：processResumePacket() - 处理恢复请求
 */
void Transfer::processPacket(QByteArray &data, PacketType type)
{
    switch (type) {
    case PacketType::Header : processHeaderPacket(data); break;
    case PacketType::Data : processDataPacket(data); break;
    case PacketType::Finish : processFinishPacket(data); break;
    case PacketType::Cancel : processCancelPacket(data); break;
    case PacketType::Pause : processPausePacket(data); break;
    case PacketType::Resume : processResumePacket(data); break;
    }
}

/**
 * @brief 处理文件头信息包（空实现）
 * @param data 包含文件元信息的数据（通常是JSON格式）
 *
 * 基类提供空实现，派生类需要重写此方法。
 * Receiver会在此方法中解析文件信息并创建目标文件。
 * Sender通常不需要处理此包。
 */
void Transfer::processHeaderPacket(QByteArray& data)
{
    Q_UNUSED(data);
}

/**
 * @brief 处理文件数据包（空实现）
 * @param data 文件数据块
 *
 * 基类提供空实现，派生类需要重写此方法。
 * Receiver会在此方法中将数据写入文件。
 * Sender通常不需要处理此包。
 */
void Transfer::processDataPacket(QByteArray& data)
{
    Q_UNUSED(data);
}

/**
 * @brief 处理传输完成包（空实现）
 * @param data 包数据（通常为空）
 *
 * 基类提供空实现，派生类需要重写此方法。
 * Receiver会在此方法中关闭文件、更新状态、发送完成信号。
 * Sender通常不需要处理此包。
 */
void Transfer::processFinishPacket(QByteArray& data)
{
    Q_UNUSED(data);
}

/**
 * @brief 处理取消传输包（空实现）
 * @param data 包数据（通常为空）
 *
 * 基类提供空实现，派生类需要重写此方法。
 * - Sender：接收到此包表示接收方取消，需要停止发送并断开连接
 * - Receiver：接收到此包表示发送方取消，需要删除未完成的文件
 */
void Transfer::processCancelPacket(QByteArray& data)
{
    Q_UNUSED(data);
}

/**
 * @brief 处理暂停传输包（空实现）
 * @param data 包数据（通常为空）
 *
 * 基类提供空实现，派生类需要重写此方法。
 * Sender接收到此包后需要停止发送数据，设置暂停标志。
 */
void Transfer::processPausePacket(QByteArray& data)
{
    Q_UNUSED(data);
}

/**
 * @brief 处理恢复传输包（空实现）
 * @param data 包数据（通常为空）
 *
 * 基类提供空实现，派生类需要重写此方法。
 * Sender接收到此包后需要继续发送剩余数据。
 */
void Transfer::processResumePacket(QByteArray& data)
{
    Q_UNUSED(data);
}

/**
 * @brief 清空读取缓冲区
 *
 * 清除接收缓冲区中的所有数据，并重置包大小标志为-1。
 * 用于以下场景：
 * - 取消传输时清理状态
 * - 错误恢复时重置缓冲区
 * - 避免缓冲区数据污染新的传输
 */
void Transfer::clearReadBuffer()
{
    mBuff.clear();           // 清空缓冲区
    mPacketSize = -1;        // 重置包大小为-1，表示没有正在接收的包
}

/**
 * @brief 设置TCP套接字
 * @param socket TCP套接字指针
 *
 * 设置用于传输的socket，并自动连接readyRead信号。
 * 当socket接收到数据时，会自动调用onReadyRead槽函数处理。
 *
 * 注意：
 * - 如果socket为nullptr，不进行任何操作
 * - 只能设置一次socket，不支持中途更换
 */
void Transfer::setSocket(QTcpSocket *socket)
{
    if (socket) {
        mSocket = socket;
        // 连接socket的readyRead信号到onReadyRead槽
        // 当socket有数据可读时，自动触发数据包解析流程
        connect(mSocket, &QTcpSocket::readyRead, this, &Transfer::onReadyRead);
    }
}

/**
 * @brief 处理初始数据包
 * @param buffer 包含初始数据包的缓冲区
 *
 * 用于处理协议识别后的初始数据包。
 * 主要用于TransferServer在识别协议后传递已读取的数据。
 */
void Transfer::processInitialBuffer(const QByteArray& buffer)
{
    // 将初始数据添加到缓冲区
    mBuff.append(buffer);

    // 处理缓冲区中的数据包（使用与onReadyRead相同的逻辑）
    while (mBuff.size() >= mHeaderSize) {
        // 步骤1：读取包大小（如果还没有读取）
        if (mPacketSize < 0) {
            // 从缓冲区前4个字节提取包大小
            memcpy(&mPacketSize, mBuff.constData(), sizeof(mPacketSize));
            // 从缓冲区移除已读取的包大小字段
            mBuff.remove(0, sizeof(mPacketSize));
        }

        // 步骤2：检查缓冲区是否包含完整的包数据（类型+数据）
        if (mBuff.size() > mPacketSize) {
            // 提取包类型（第1个字节）
            PacketType type = static_cast<PacketType>(mBuff.at(0));

            // 提取包数据（从第2个字节开始，长度为mPacketSize）
            QByteArray data = mBuff.mid(1, mPacketSize);

            // 步骤3：分发数据包到相应的处理函数
            processPacket(data, type);

            // 步骤4：从缓冲区移除已处理的数据（类型1字节+数据N字节）
            mBuff.remove(0, mPacketSize + 1);

            // 步骤5：重置包大小，准备接收下一个数据包
            mPacketSize = -1;
        }
        else {
            // 数据不完整，退出循环等待下次readyRead接收更多数据
            break;
        }
    }
}

