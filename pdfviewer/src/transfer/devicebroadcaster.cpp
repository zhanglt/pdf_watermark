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
 * @file devicebroadcaster.cpp
 * @brief 设备广播器实现
 *
 * 本文件实现了DeviceBroadcaster类的所有功能。
 * DeviceBroadcaster是局域网设备发现的核心组件，通过UDP广播协议
 * 实现设备的自动发现和在线状态维护。
 *
 * 实现要点：
 * - 使用QUdpSocket实现UDP广播的发送和接收
 * - 使用QTimer实现周期性心跳广播
 * - 使用JSON格式封装设备信息
 * - 支持多网卡环境，向所有网段广播
 *
 * 网络协议：
 * - 传输层：UDP（无连接、不可靠但快速）
 * - 应用层：JSON格式的设备信息
 * - 端口：可配置（默认与TCP传输端口不同）
 * - 广播方式：向网段广播地址发送（如192.168.1.255）
 *
 * 心跳机制：
 * - 定期广播本机信息，通知其他设备"我在线"
 * - 接收其他设备的广播，发现新设备或更新设备状态
 * - 设备列表通过超时机制判断设备离线
 */

#include "devicebroadcaster.h"
#include "../ui/settings.h"

#include <cassert>

/**
 * @brief DeviceBroadcaster 构造函数
 * @param parent 父对象指针
 *
 * 初始化设备广播器的网络和定时器组件。
 *
 * 初始化步骤：
 * 1. 连接定时器timeout信号到sendBroadcast槽（周期性发送广播）
 * 2. 连接UDP socket的readyRead信号到processBroadcast槽（接收广播）
 * 3. 绑定UDP socket到广播端口
 *
 * UDP绑定配置：
 * - 端口：从Settings获取（通常与TCP传输端口不同）
 * - ShareAddress模式：允许多个应用/进程共享同一端口
 *   这在开发调试时很有用，可以同时运行多个实例
 *
 * 注意：
 * - 构造完成后仅完成初始化，未开始广播
 * - 需要调用start()才会启动定时器和发送广播
 * - UDP socket已绑定，可以立即接收其他设备的广播
 */
DeviceBroadcaster::DeviceBroadcaster(QObject *parent) : QObject(parent)
{
    // 连接定时器信号：定时器超时时发送广播
    connect(&mTimer, &QTimer::timeout, this, &DeviceBroadcaster::sendBroadcast);

    // 连接UDP socket信号：有数据到达时处理广播
    connect(&mUdpSock, &QUdpSocket::readyRead, this, &DeviceBroadcaster::processBroadcast);

    // 绑定UDP socket到广播端口
    // ShareAddress模式允许多个应用共享端口（用于开发调试）
    mUdpSock.bind(Settings::instance()->getBroadcastPort(), QUdpSocket::ShareAddress);
}

/**
 * @brief 启动设备广播器
 *
 * 开始周期性广播本机设备信息。
 *
 * 执行操作：
 * 1. 立即发送一次广播
 *    - 让局域网内其他设备快速发现本机
 *    - 不用等待第一个定时器周期
 * 2. 启动定时器（如果尚未启动）
 *    - 间隔由Settings::getBroadcastInterval()决定
 *    - 典型值为2-5秒
 *
 * 定时器管理：
 * - 检查isActive()避免重复启动
 * - 如果已在运行，不会重新启动
 * - 这样可以安全地多次调用start()
 *
 * 调用时机：
 * - MainWindow初始化完成后调用
 * - 网络设置变更后可重新调用
 * - 从休眠恢复后可调用以重新宣告在线
 */
void DeviceBroadcaster::start()
{
    qDebug() << "DeviceBroadcaster: Starting device discovery...";

    // 立即发送一次广播，快速通知其他设备
    sendBroadcast();

    // 启动定时器开始周期性广播（如果尚未启动）
    if (!mTimer.isActive()) {
        int interval = Settings::instance()->getBroadcastInterval();
        qDebug() << "  Starting broadcast timer with interval" << interval << "ms";
        mTimer.start(interval);
    }
}

/**
 * @brief 发送设备广播
 *
 * 向局域网所有可达网段广播本机设备信息。
 *
 * 工作流程：
 * 1. 从Settings获取广播端口号
 * 2. 从Settings获取本机设备信息（ID、名称、操作系统）
 * 3. 构造包含设备信息的JSON对象
 *    - id：设备唯一标识符（UUID）
 *    - name：设备名称（计算机名）
 *    - os：操作系统名称（Windows/Linux/macOS）
 *    - port：广播端口（用于验证）
 * 4. 将JSON转换为紧凑格式的字节数组
 *    - 使用Compact格式减小数据包大小
 *    - 移除不必要的空格和换行
 * 5. 获取所有网卡的广播地址
 * 6. 向每个广播地址发送UDP数据包
 *
 * 多网卡处理：
 * - 遍历所有广播地址，向每个地址发送
 * - 确保所有网段的设备都能收到广播
 * - 例如：同时有WiFi（192.168.1.x）和以太网（192.168.0.x）
 *
 * 性能考虑：
 * - 广播数据很小（通常几十到一百多字节）
 * - UDP无连接，发送速度快
 * - 即使有多个网卡，总开销也很小
 */
void DeviceBroadcaster::sendBroadcast()
{
    // 获取广播端口号（同一局域网内所有设备必须使用相同端口）
    int port = Settings::instance()->getBroadcastPort();

    // 获取本机设备信息
    Device dev = Settings::instance()->getMyDevice();

    // 构造JSON对象，包含设备的所有必要信息
    // 使用QVariantMap快速构造，然后转换为QJsonObject
    QJsonObject obj(QJsonObject::fromVariantMap({
                                                    {"id", dev.getId()},        // 设备唯一ID
                                                    {"name", dev.getName()},    // 设备名称
                                                    {"os", dev.getOSName()},    // 操作系统
                                                    {"port", port}              // 广播端口
                                                }));

    // 获取本机所有网卡的广播地址
    QVector<QHostAddress> addresses = getBroadcastAddressFromInterfaces();

    // 将JSON对象转换为紧凑格式的字节数组（移除空格和换行）
    QByteArray data(QJsonDocument(obj).toJson(QJsonDocument::Compact));

    qDebug() << "DeviceBroadcaster: Sending broadcast to" << addresses.size() << "addresses"
             << "on port" << port;
    qDebug() << "  Broadcast data:" << data;

    // 向每个广播地址发送UDP数据包
    // 这样可以覆盖所有网段，确保所有设备都能收到
    foreach (QHostAddress address, addresses) {
        qDebug() << "  Sending to:" << address.toString();
        qint64 bytes = mUdpSock.writeDatagram(data, address, port);
        if (bytes < 0) {
            qDebug() << "    Failed to send broadcast:" << mUdpSock.errorString();
        }
    }
}

/**
 * @brief 处理接收到的设备广播
 *
 * 当UDP socket接收到广播数据时，此函数被调用来处理数据。
 *
 * 处理流程：
 * 1. 循环处理所有待处理的UDP数据包
 *    - 可能同时收到多个设备的广播
 *    - hasPendingDatagrams()检查是否还有数据
 * 2. 获取数据包大小并分配缓冲区
 *    - 使用断言确保大小在int范围内
 * 3. 读取数据包内容和发送方IP地址
 * 4. 解析JSON数据提取设备信息
 * 5. 验证数据完整性和有效性：
 *    - 检查JSON是否包含所有4个必需字段
 *    - 验证端口号是否与本机设置一致
 * 6. 构造Device对象并发出broadcastReceived信号
 *
 * 数据验证：
 * - 字段数量检查：必须正好4个字段（id, name, os, port）
 * - 端口号验证：只接受相同端口的广播，防止串扰
 * - JSON格式检查：自动忽略格式错误的数据
 *
 * 错误处理：
 * - 如果JSON解析失败，obj.keys().length()会返回0，被忽略
 * - 如果端口号不匹配，不发出信号（可能是其他应用）
 * - 静默忽略错误，不影响后续数据包处理
 *
 * 性能优化：
 * - 使用while循环批量处理，避免遗漏数据包
 * - 数据验证快速失败，减少无效数据的处理开销
 */
void DeviceBroadcaster::processBroadcast()
{
    // 循环处理所有待处理的UDP数据包
    // 因为可能同时收到多个设备的广播
    while (mUdpSock.hasPendingDatagrams()) {
        QByteArray data;

        // 获取待读取数据包的大小
        qint64 datagramSize = mUdpSock.pendingDatagramSize();

        // 断言检查：确保数据包大小在int类型范围内
        // UDP数据包通常很小，这应该总是成立
        assert(datagramSize <= std::numeric_limits <int>::max());

        // 调整缓冲区大小以容纳数据包
        data.resize(static_cast<int>(datagramSize));

        // 用于存储发送方IP地址
        QHostAddress sender;

        // 读取UDP数据包，同时获取发送方地址
        // sender将被设置为发送此广播的设备的IP地址
        mUdpSock.readDatagram(data.data(), data.size(), &sender);

        qDebug() << "DeviceBroadcaster: Received UDP datagram from" << sender.toString()
                 << "size:" << data.size();

        // 将接收到的字节数组解析为JSON对象
        QJsonObject obj = QJsonDocument::fromJson(data).object();

        qDebug() << "  JSON keys:" << obj.keys() << "Port:" << obj.value("port").toString();

        // 验证JSON数据的完整性：必须包含所有4个必需字段
        if (obj.keys().length() == 4) {
            // 验证端口号：只接受相同端口的广播
            // 这防止了不同配置的LANShare实例或其他应用的干扰
            if (obj.value("port").toVariant().value<quint16>() ==
                    Settings::instance()->getBroadcastPort()) {

                // 从JSON对象构造Device对象
                // 包括：设备ID、名称、操作系统、IP地址
                Device device{obj.value("id").toString(),      // 设备唯一ID
                              obj.value("name").toString(),    // 设备名称
                              obj.value("os").toString(),      // 操作系统
                              sender};                         // IP地址（从UDP包头获取）

                qDebug() << "  Device discovered:" << device.getName()
                         << "ID:" << device.getId()
                         << "IP:" << device.getAddress().toString();
                        // << "OS:" << device.getOS();

                // 发出信号通知设备列表有新设备或设备更新
                // DeviceListModel会接收此信号并更新设备列表
                emit broadcastReceived(device);
            } else {
                qDebug() << "  Port mismatch:" << obj.value("port").toVariant().value<quint16>()
                         << "expected:" << Settings::instance()->getBroadcastPort();
            }
        } else {
            qDebug() << "  Invalid JSON format, expected 4 keys, got" << obj.keys().length();
        }
    }
}

/**
 * @brief 获取所有网卡的广播地址
 * @return 广播地址列表
 *
 * 扫描本机所有网络接口，提取支持广播的接口的广播地址。
 *
 * 实现步骤：
 * 1. 调用QNetworkInterface::allInterfaces()获取所有网卡
 *    - 包括物理网卡（WiFi、以太网）
 *    - 包括虚拟网卡（VPN、虚拟机）
 *    - 包括回环接口（loopback）
 * 2. 遍历每个网卡接口
 * 3. 检查接口是否支持广播（CanBroadcast标志）
 *    - 回环接口不支持广播
 *    - 点对点接口（PPP）不支持广播
 *    - 以太网和WiFi支持广播
 * 4. 对每个支持广播的接口，遍历其地址条目
 *    - 一个接口可能有多个IP地址（IPv4、IPv6）
 *    - 每个地址有对应的广播地址
 * 5. 提取非空的广播地址并添加到列表
 *
 * 广播地址说明：
 * - IPv4广播地址由IP地址和子网掩码计算得出
 * - 例如：192.168.1.100/24 的广播地址是 192.168.1.255
 * - 发送到广播地址的数据包会被该子网所有主机接收
 *
 * 多网卡场景：
 * - 笔记本同时连接WiFi和以太网：两个广播地址
 * - 虚拟机桥接网卡：添加虚拟网卡的广播地址
 * - 多网段环境：确保所有网段都能收到广播
 *
 * 返回值示例：
 * - 单网卡：[192.168.1.255]
 * - 双网卡：[192.168.1.255, 192.168.0.255]
 * - 复杂环境：[192.168.1.255, 192.168.0.255, 172.16.0.255]
 *
 * 用途：
 * - sendBroadcast()使用返回的地址列表发送广播
 * - 确保本机在所有连接的网段上都能被发现
 */
QVector<QHostAddress> DeviceBroadcaster::getBroadcastAddressFromInterfaces()
{
    QVector<QHostAddress> addresses;  // 存储所有广播地址

    // 遍历本机所有网络接口
    foreach (QNetworkInterface iface, QNetworkInterface::allInterfaces()) {
        // 检查接口是否支持广播
        // CanBroadcast标志表示接口支持广播通信
        if (iface.flags() & QNetworkInterface::CanBroadcast) {
            // 遍历该接口上的所有地址条目
            // 一个接口可能配置多个IP地址
            foreach (QNetworkAddressEntry addressEntry, iface.addressEntries()) {
                // 获取广播地址并检查是否有效
                // 如果广播地址不为空，添加到列表
                if (!addressEntry.broadcast().isNull()) {
                    addresses.push_back(addressEntry.broadcast());
                }
            }
        }
    }

    return addresses;  // 返回所有找到的广播地址
}
