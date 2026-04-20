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
 * @file transferserver.h
 * @brief TCP文件传输服务器类定义
 *
 * 本文件定义了TransferServer类，它是文件传输功能的服务端核心组件。
 * TransferServer负责监听TCP端口，接受来自其他设备的文件传输连接请求，
 * 并为每个连接创建独立的Receiver对象来处理文件接收。
 *
 * 核心功能：
 * - 在指定端口启动TCP服务器监听
 * - 接受来自局域网内其他设备的连接请求
 * - 为每个新连接创建Receiver对象处理文件接收
 * - 管理所有活动的Receiver对象生命周期
 * - 通知UI层有新的文件接收任务
 *
 * 工作流程：
 * 1. listen() - 在指定端口开始监听
 * 2. 当有设备连接时，QTcpServer触发newConnection信号
 * 3. onNewConnection() - 处理新连接，创建Receiver对象
 * 4. 发出newReceiverAdded信号通知UI层更新界面
 *
 * 多连接处理：
 * - 支持同时接受多个设备的文件传输
 * - 每个连接都有独立的Receiver对象处理
 * - 使用QVector管理所有Receiver对象
 *
 * 与其他组件的关系：
 * - DeviceBroadcaster：发现局域网内的设备
 * - TransferServer：接受文件传输连接（本类）
 * - Receiver：处理具体的文件接收任务
 * - Sender：主动向其他设备发送文件
 */

#ifndef TRANSFERSERVER_H
#define TRANSFERSERVER_H

#include <QTcpServer>
#include <QObject>
#include <QTimer>

#include "receiver.h"
#include "pullsender.h"
#include "../model/devicelistmodel.h"

/**
 * @class TransferServer
 * @brief TCP文件传输服务器
 *
 * TransferServer封装了Qt的QTcpServer，专门用于接收文件传输连接。
 * 它在后台持续监听TCP端口，当其他设备要发送文件时建立连接，
 * 并为每个连接创建独立的Receiver对象来处理文件接收任务。
 *
 * 工作原理：
 * 1. 应用启动时，MainWindow创建TransferServer对象
 * 2. 调用listen()在指定端口（从Settings读取）开始监听
 * 3. 当其他设备通过ReceiverSelectorDialog选择本设备并发送文件时：
 *    - 发送端（Sender）连接到本机的传输端口
 *    - QTcpServer检测到新连接，触发newConnection信号
 *    - onNewConnection()获取socket并创建Receiver对象
 *    - 发出newReceiverAdded信号，MainWindow将Receiver添加到传输列表
 * 4. Receiver对象独立处理文件接收，直到传输完成或取消
 *
 * 端口配置：
 * - 传输端口从Settings::getTransferPort()获取
 * - 默认端口与广播端口不同，避免冲突
 * - 如果端口被占用，listen()会失败
 *
 * 多任务并发：
 * - 可以同时接收来自多个设备的文件
 * - 每个Receiver在独立线程中运行（由Qt的事件循环管理）
 * - 多个Receiver共享传输端口，由操作系统的TCP栈管理连接
 *
 * 设备识别：
 * - 通过socket的peerAddress()获取发送端IP
 * - 从DeviceListModel中查找对应的设备信息
 * - 设备信息包括设备名称、操作系统等
 *
 * 使用示例：
 * @code
 * DeviceListModel* deviceList = ...;
 * TransferServer* server = new TransferServer(deviceList);
 * if (server->listen(QHostAddress::Any)) {
 *     qDebug() << "服务器已启动，等待连接...";
 *     connect(server, &TransferServer::newReceiverAdded,
 *             this, &MainWindow::onNewReceiver);
 * }
 * @endcode
 */
// 前向声明
class TransferServer;

/**
 * @class ProtocolIdentifier
 * @brief 协议识别器，用于识别连接类型（Push或Pull）
 *
 * 当新连接到来时，先创建ProtocolIdentifier来读取第一个数据包，
 * 根据数据包类型判断是Push协议（普通文件传输）还是Pull协议（共享浏览）。
 */
class ProtocolIdentifier : public QObject {
    Q_OBJECT
public:
    ProtocolIdentifier(QTcpSocket* socket, TransferServer* server);
    ~ProtocolIdentifier();

    void start();

private Q_SLOTS:
    void onReadyRead();
    void onDisconnected();
    void onTimeout();

private:
    QTcpSocket* mSocket;
    TransferServer* mServer;
    QTimer* mTimeoutTimer;
    QByteArray mBuffer;
};

class TransferServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief TransferServer 构造函数
     * @param devList 设备列表模型指针，用于根据IP地址查找设备信息
     * @param parent 父对象指针
     *
     * 初始化TCP服务器：
     * - 保存设备列表模型的引用
     * - 创建QTcpServer对象
     * - 连接QTcpServer的newConnection信号到onNewConnection槽
     * - 初始化Receiver对象列表
     *
     * 注意：构造函数不会启动服务器，需要调用listen()才开始监听。
     */
    explicit TransferServer(DeviceListModel* devList, QObject *parent = nullptr);

    /**
     * @brief 开始监听TCP端口
     * @param addr 监听地址，默认为QHostAddress::Any（监听所有网卡）
     * @return 成功返回true，失败返回false
     *
     * 在指定地址和端口上启动TCP服务器监听。
     *
     * 端口号从Settings::getTransferPort()获取，通常与广播端口不同。
     * 监听地址QHostAddress::Any表示接受所有网卡接口的连接请求，
     * 这样无论本机有多少个IP地址，其他设备都可以连接。
     *
     * 失败原因：
     * - 端口被其他程序占用
     * - 没有权限监听该端口（如1024以下的端口）
     * - 防火墙阻止
     *
     * 调用时机：
     * - 应用启动时由MainWindow调用
     * - 设置变更后重新启动服务器
     */
    bool listen(const QHostAddress& addr = QHostAddress::Any);

    /**
     * @brief 关闭传输服务器
     *
     * 停止监听新的连接请求，释放端口。
     * 现有的传输连接不受影响，会继续进行。
     *
     * 用途：
     * - 端口配置更改后需要重新监听
     * - 应用退出时清理资源
     */
    void close();

Q_SIGNALS:
    /**
     * @brief 新接收任务创建信号
     * @param receiver 新创建的Receiver对象指针
     *
     * 当有新的文件传输连接建立并创建Receiver对象后触发。
     *
     * 触发时机：
     * - 其他设备发起文件传输连接
     * - socket连接成功建立
     * - Receiver对象创建完成
     *
     * 用途：
     * - 通知MainWindow有新的接收任务
     * - MainWindow将Receiver添加到传输列表UI
     * - 连接Receiver的信号以更新传输进度
     *
     * 注意：Receiver的所有权转移给接收方（通常是MainWindow），
     * 由接收方负责管理Receiver的生命周期。
     */
    void newReceiverAdded(Receiver* receiver);

    /**
     * @brief 新的Pull发送任务创建信号
     * @param sender 新创建的PullSender对象指针
     *
     * 当有Pull协议连接建立并创建PullSender对象后触发。
     */
    void newPullSenderAdded(PullSender* sender);

private Q_SLOTS:
    /**
     * @brief 处理新TCP连接
     *
     * 当QTcpServer接受到新连接时触发此槽函数。
     *
     * 处理流程：
     * 1. 调用mServer->nextPendingConnection()获取新连接的socket
     * 2. 从socket->peerAddress()获取对端（发送方）的IP地址
     * 3. 通过DeviceListModel查找对应的设备信息
     * 4. 创建Receiver对象，传入设备信息和socket
     * 5. 将Receiver添加到mReceivers列表管理
     * 6. 发出newReceiverAdded信号通知UI层
     *
     * 错误处理：
     * - 如果socket为空（不应该发生），不做任何操作
     * - 如果在设备列表中找不到对应设备，仍然创建Receiver（使用默认设备信息）
     *
     * 注意：此函数在主线程中执行，但文件接收操作由Receiver在其内部处理，
     * 利用Qt的信号槽机制实现异步IO，不会阻塞主线程。
     */
    void onNewConnection();

public:
    /**
     * @brief 处理已识别的Push协议连接
     * @param socket TCP连接
     * @param initialData 已读取的初始数据
     */
    void handlePushProtocol(QTcpSocket* socket, const QByteArray& initialData);

    /**
     * @brief 处理已识别的Pull协议连接
     * @param socket TCP连接
     * @param initialData 已读取的初始数据
     */
    void handlePullProtocol(QTcpSocket* socket, const QByteArray& initialData);

private:
    DeviceListModel* mDevList;          ///< 设备列表模型，用于根据IP地址查找设备信息
    QTcpServer* mServer;                ///< TCP服务器对象，负责监听和接受连接
    QVector<Receiver*> mReceivers;      ///< 所有Receiver对象的列表，管理接收任务生命周期
    QVector<PullSender*> mPullSenders;  ///< 所有PullSender对象的列表
    QVector<ProtocolIdentifier*> mIdentifiers; ///< 正在识别协议的连接

    friend class ProtocolIdentifier;
};

#endif // TRANSFERSERVER_H
