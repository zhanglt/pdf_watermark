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
 * @file transferserver.cpp
 * @brief TCP文件传输服务器实现
 *
 * 本文件实现了TransferServer类的所有功能。
 * TransferServer是文件接收功能的核心，它在后台持续监听TCP端口，
 * 等待来自局域网内其他设备的文件传输连接。
 *
 * 实现要点：
 * - 使用QTcpServer实现TCP服务器监听
 * - 自动处理新连接，为每个连接创建Receiver对象
 * - 维护所有活动Receiver的列表
 * - 通过信号通知UI层更新界面
 *
 * 线程模型：
 * - TransferServer运行在主线程
 * - 所有Receiver也在主线程中创建和运行
 * - 利用Qt的事件循环实现异步IO，避免阻塞
 * - 文件IO使用缓冲机制，不会阻塞网络操作
 */

#include "transferserver.h"

#include "../ui/settings.h"
#include <QtDebug>

/**
 * @brief TransferServer 构造函数
 * @param devList 设备列表模型指针
 * @param parent 父对象指针
 *
 * 初始化TCP文件传输服务器：
 * 1. 保存设备列表模型的引用（用于根据IP查找设备信息）
 * 2. 创建QTcpServer对象并设置父对象（自动内存管理）
 * 3. 连接newConnection信号到onNewConnection槽（监听新连接）
 * 4. 初始化Receiver列表（空列表）
 *
 * 注意：此时服务器尚未启动，需要调用listen()才会开始监听端口。
 */
TransferServer::TransferServer(DeviceListModel* devList, QObject *parent) : QObject(parent)
{
    mDevList = devList;                    // 保存设备列表引用
    mServer = new QTcpServer(this);        // 创建TCP服务器，设置父对象实现自动内存管理

    // 连接QTcpServer的新连接信号到我们的处理槽函数
    connect(mServer, &QTcpServer::newConnection, this, &TransferServer::onNewConnection);
}

/**
 * @brief 开始监听TCP端口
 * @param addr 监听地址，默认为QHostAddress::Any
 * @return 成功返回true，失败返回false
 *
 * 在指定地址和端口上启动TCP服务器监听。
 *
 * 实现细节：
 * - 监听地址默认为QHostAddress::Any，表示监听所有网卡接口
 * - 监听端口从Settings单例获取（通常与广播端口不同）
 * - 调用QTcpServer::listen()启动监听
 * - 如果端口已被占用或无权限，listen()会返回false
 *
 * 端口说明：
 * - 广播端口：用于设备发现（UDP）
 * - 传输端口：用于文件传输（TCP，本方法监听的端口）
 * - 两者必须不同，避免冲突
 *
 * 典型调用：
 * - MainWindow初始化时调用
 * - 设置对话框修改端口后重新调用
 */
bool TransferServer::listen(const QHostAddress &addr)
{
    // 从设置中获取传输端口，在指定地址上开始监听
    return mServer->listen(addr, Settings::instance()->getTransferPort());
}

/**
 * @brief 关闭传输服务器
 *
 * 停止监听新的TCP连接，释放占用的端口。
 * 注意：这只是停止接受新连接，现有的传输任务会继续进行。
 */
void TransferServer::close()
{
    if (mServer && mServer->isListening()) {
        mServer->close();
    }
}

/**
 * @brief 处理新的TCP连接
 *
 * 当有其他设备连接到本机传输端口时，QTcpServer触发newConnection信号，
 * 此槽函数被调用来处理新建立的连接。
 *
 * 处理流程：
 * 1. 从QTcpServer获取待处理的socket连接
 * 2. 创建ProtocolIdentifier来识别协议类型
 * 3. ProtocolIdentifier会读取第一个数据包并判断协议
 * 4. 根据协议类型创建Receiver（Push）或PullSender（Pull）
 *
 * 协议识别：
 * - PacketType::Header → Push协议（普通文件传输）
 * - PacketType::Command → Pull协议（共享浏览）
 */
void TransferServer::onNewConnection()
{
    // 获取下一个待处理的连接
    QTcpSocket* socket = mServer->nextPendingConnection();

    if (socket) {
        // 创建协议识别器来判断连接类型
        ProtocolIdentifier* identifier = new ProtocolIdentifier(socket, this);
        mIdentifiers.push_back(identifier);
        identifier->start();
    }
}

/**
 * @brief 处理已识别的Push协议连接
 * @param socket TCP连接
 * @param initialData 已读取的初始数据
 */
void TransferServer::handlePushProtocol(QTcpSocket* socket, const QByteArray& initialData)
{
    // 根据socket的对端IP地址从设备列表中查找设备信息
    Device dev = mDevList->device(socket->peerAddress());

    // 创建Receiver对象来处理此连接的文件接收任务
    Receiver* rec = new Receiver(dev, socket);

    // 如果有初始数据，先处理它
    if (!initialData.isEmpty()) {
        rec->processInitialBuffer(initialData);
    }

    // 将Receiver添加到列表中管理
    mReceivers.push_back(rec);

    // 发出信号通知UI层有新的接收任务
    emit newReceiverAdded(rec);
}

/**
 * @brief 处理已识别的Pull协议连接
 * @param socket TCP连接
 * @param initialData 已读取的初始数据
 */
void TransferServer::handlePullProtocol(QTcpSocket* socket, const QByteArray& initialData)
{
    // 创建PullSender对象来处理Pull协议请求
    PullSender* sender = new PullSender(socket, this);

    // 如果有初始数据，先处理它
    if (!initialData.isEmpty()) {
        sender->processInitialBuffer(initialData);
    }

    // 将PullSender添加到列表中管理
    mPullSenders.push_back(sender);

    // 发出信号通知UI层有新的Pull发送任务
    emit newPullSenderAdded(sender);
}

/**
 * @brief ProtocolIdentifier构造函数
 * @param socket TCP连接
 * @param server TransferServer指针
 */
ProtocolIdentifier::ProtocolIdentifier(QTcpSocket* socket, TransferServer* server)
    : QObject(server), mSocket(socket), mServer(server)
{
    mSocket->setParent(this);

    // 设置超时定时器（5秒内必须收到第一个数据包）
    mTimeoutTimer = new QTimer(this);
    mTimeoutTimer->setSingleShot(true);
    mTimeoutTimer->setInterval(5000);

    connect(mSocket, &QTcpSocket::readyRead, this, &ProtocolIdentifier::onReadyRead);
    connect(mSocket, &QTcpSocket::disconnected, this, &ProtocolIdentifier::onDisconnected);
    connect(mTimeoutTimer, &QTimer::timeout, this, &ProtocolIdentifier::onTimeout);
}

/**
 * @brief ProtocolIdentifier析构函数
 */
ProtocolIdentifier::~ProtocolIdentifier()
{
    // 从识别器列表中移除自己
    mServer->mIdentifiers.removeOne(this);
}

/**
 * @brief 开始协议识别
 */
void ProtocolIdentifier::start()
{
    mTimeoutTimer->start();
    // 检查是否已有数据可读
    if (mSocket->bytesAvailable() > 0) {
        onReadyRead();
    }
}

/**
 * @brief 处理socket数据可读事件
 */
void ProtocolIdentifier::onReadyRead()
{
    // 读取所有可用数据
    mBuffer.append(mSocket->readAll());

    // 检查是否有足够的数据来识别协议（至少需要包头：4字节包大小 + 1字节包类型）
    if (mBuffer.size() >= 5) { // qint32 size + PacketType (1 byte)
        // 解析包头
        qint32 packetSize = *reinterpret_cast<const qint32*>(mBuffer.constData());
        PacketType packetType = static_cast<PacketType>(mBuffer.at(4));  // 包类型在第5个字节

        // 根据包类型判断协议
        if (packetType == PacketType::Header) {
            // Push协议（普通文件传输）
            mTimeoutTimer->stop();
            mSocket->setParent(nullptr);  // 断开与identifier的关联
            mServer->handlePushProtocol(mSocket, mBuffer);
            deleteLater();
        } else if (packetType == PacketType::Command) {
            // Pull协议（共享浏览）
            mTimeoutTimer->stop();
            mSocket->setParent(nullptr);  // 断开与identifier的关联
            mServer->handlePullProtocol(mSocket, mBuffer);
            deleteLater();
        } else {
            // 未知协议，断开连接
            qWarning() << "Unknown protocol type:" << static_cast<int>(packetType);
            mSocket->disconnectFromHost();
            deleteLater();
        }
    }
}

/**
 * @brief 处理连接断开事件
 */
void ProtocolIdentifier::onDisconnected()
{
    deleteLater();
}

/**
 * @brief 处理超时事件
 */
void ProtocolIdentifier::onTimeout()
{
    qWarning() << "Protocol identification timeout";
    mSocket->disconnectFromHost();
    deleteLater();
}
