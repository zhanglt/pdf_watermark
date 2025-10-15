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
 * @file devicebroadcaster.h
 * @brief 设备广播器类定义
 *
 * 本文件定义了DeviceBroadcaster类，它是设备发现机制的核心组件。
 * DeviceBroadcaster使用UDP广播协议在局域网内发现和通告设备，
 * 实现了无需配置的自动设备发现功能。
 *
 * 核心功能：
 * - 定期向局域网广播本机设备信息
 * - 接收来自其他设备的广播消息
 * - 解析并验证广播数据
 * - 通过信号通知发现的新设备
 *
 * 工作原理：
 * 1. 绑定UDP端口，既用于发送也用于接收
 * 2. 启动定时器，定期发送广播包（心跳机制）
 * 3. 监听UDP端口，接收其他设备的广播
 * 4. 解析收到的JSON格式设备信息
 * 5. 发出信号通知DeviceListModel更新设备列表
 *
 * 广播协议：
 * - 协议：UDP
 * - 端口：从Settings读取（广播端口）
 * - 数据格式：JSON（包含设备ID、名称、操作系统、端口）
 * - 广播间隔：从Settings读取（通常为几秒）
 *
 * 心跳机制：
 * - 定期发送广播保持在线状态
 * - 其他设备根据最后接收时间判断设备是否在线
 * - 长时间未收到广播的设备会被标记为离线
 *
 * 网络拓扑：
 * - 支持多个网卡接口
 * - 向每个网卡的广播地址发送广播包
 * - 自动适应不同的网络环境
 *
 * 与其他组件的关系：
 * - DeviceBroadcaster：发现设备（本类）
 * - DeviceListModel：维护设备列表
 * - TransferServer：接受文件传输连接
 * - Sender：向发现的设备发送文件
 */

#ifndef DEVICEBROADCASTER_H
#define DEVICEBROADCASTER_H

#include <QObject>
#include <QTimer>
#include <QtNetwork>

#include "../model/device.h"

/**
 * @class DeviceBroadcaster
 * @brief 设备广播器，用于局域网设备发现
 *
 * DeviceBroadcaster实现了基于UDP广播的设备发现机制。
 * 它周期性地向局域网广播本机设备信息，同时监听并处理来自其他设备的广播，
 * 实现了一个去中心化的设备发现系统。
 *
 * 工作原理：
 * 1. 构造时绑定UDP端口，准备接收广播
 * 2. start()启动定时器，开始周期性广播
 * 3. 每个广播周期（默认几秒）：
 *    - 获取本机所有网卡的广播地址
 *    - 构造包含设备信息的JSON包
 *    - 向所有广播地址发送UDP数据包
 * 4. 同时监听UDP端口，接收其他设备的广播：
 *    - 解析JSON数据提取设备信息
 *    - 验证端口号是否匹配（防止端口冲突）
 *    - 发出broadcastReceived信号通知设备列表
 *
 * 广播数据格式（JSON）：
 * @code
 * {
 *     "id": "唯一设备ID",
 *     "name": "设备名称",
 *     "os": "操作系统名称",
 *     "port": 广播端口号
 * }
 * @endcode
 *
 * 安全性：
 * - 验证广播端口号，只接受相同端口的广播
 * - 检查JSON格式完整性，忽略格式错误的数据
 * - 使用ShareAddress模式允许多个实例共存
 *
 * 性能优化：
 * - 使用紧凑JSON格式减少数据包大小
 * - 只广播必要的设备信息
 * - 定时器间隔可配置，平衡实时性和网络负载
 *
 * 网络适配：
 * - 自动检测所有可用网卡
 * - 只向支持广播的网卡发送数据
 * - 适应多网卡、多网段环境
 *
 * 使用示例：
 * @code
 * DeviceBroadcaster* broadcaster = new DeviceBroadcaster(this);
 * connect(broadcaster, &DeviceBroadcaster::broadcastReceived,
 *         deviceList, &DeviceListModel::addDevice);
 * broadcaster->start();  // 开始广播和监听
 * @endcode
 */
class DeviceBroadcaster : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief DeviceBroadcaster 构造函数
     * @param parent 父对象指针
     *
     * 初始化设备广播器：
     * - 创建并配置定时器（用于周期性广播）
     * - 创建并绑定UDP socket（用于发送和接收广播）
     * - 连接相关信号槽
     *
     * 关键配置：
     * - UDP socket绑定到广播端口（从Settings读取）
     * - 使用ShareAddress模式，允许多个进程共享同一端口
     * - 连接readyRead信号到processBroadcast槽
     * - 连接定时器timeout信号到sendBroadcast槽
     *
     * 注意：构造函数不会启动广播，需要调用start()才开始工作。
     */
    explicit DeviceBroadcaster(QObject *parent = nullptr);

Q_SIGNALS:
    /**
     * @brief 接收到设备广播信号
     * @param fromDevice 发送广播的设备信息
     *
     * 当接收并成功解析其他设备的广播消息后触发。
     *
     * 触发时机：
     * - 接收到UDP广播数据包
     * - JSON解析成功且包含所有必需字段
     * - 广播端口号验证通过
     * - 不是本机发送的广播（由DeviceListModel过滤）
     *
     * 用途：
     * - 通知DeviceListModel有新设备或设备更新
     * - DeviceListModel根据设备ID更新或添加设备
     * - 更新设备的最后在线时间
     *
     * 参数fromDevice包含：
     * - 设备唯一ID
     * - 设备名称
     * - 操作系统名称
     * - 设备IP地址（从UDP包头提取）
     */
    void broadcastReceived(const Device& fromDevice);

public Q_SLOTS:
    /**
     * @brief 启动设备广播器
     *
     * 开始周期性广播和监听。
     *
     * 执行操作：
     * 1. 立即发送一次广播（让其他设备快速发现本机）
     * 2. 启动定时器，按配置的间隔周期性广播
     *
     * 定时器配置：
     * - 间隔时间从Settings::getBroadcastInterval()获取
     * - 典型值为2-5秒，平衡实时性和网络负载
     * - 如果定时器已在运行，不会重复启动
     *
     * 调用时机：
     * - 应用启动后由MainWindow调用
     * - 网络配置变更后可重新调用
     */
    void start();

    /**
     * @brief 发送设备广播
     *
     * 向局域网所有可达网段发送本机设备信息。
     *
     * 工作流程：
     * 1. 从Settings获取本机设备信息（ID、名称、操作系统）
     * 2. 构造JSON对象包含设备信息和广播端口
     * 3. 将JSON转换为紧凑格式的字节数组
     * 4. 获取所有网卡的广播地址
     * 5. 向每个广播地址发送UDP数据包
     *
     * 广播地址：
     * - 通过getBroadcastAddressFromInterfaces()获取
     * - 对每个支持广播的网卡接口，提取其广播地址
     * - 例如：192.168.1.255, 192.168.0.255等
     *
     * 数据格式：
     * - JSON紧凑格式（无空格和换行）
     * - 包含4个字段：id, name, os, port
     * - 典型大小：几十到一百多字节
     *
     * 调用场景：
     * - start()调用时立即执行一次
     * - 定时器周期性触发
     * - 可手动调用强制发送广播
     */
    void sendBroadcast();

private Q_SLOTS:
    /**
     * @brief 处理接收到的广播数据
     *
     * 当UDP socket接收到数据时触发（readyRead信号）。
     *
     * 处理流程：
     * 1. 循环处理所有待处理的数据包（hasPendingDatagrams）
     * 2. 读取数据包内容和发送方IP地址
     * 3. 解析JSON数据
     * 4. 验证JSON格式（必须包含4个字段）
     * 5. 验证端口号（必须与本机广播端口一致）
     * 6. 构造Device对象
     * 7. 发出broadcastReceived信号
     *
     * 数据验证：
     * - 检查JSON是否包含所有必需字段（id, name, os, port）
     * - 验证端口号是否与本机设置一致
     * - 忽略格式错误或端口不匹配的广播
     *
     * 错误处理：
     * - JSON解析失败：忽略该数据包
     * - 字段缺失：忽略该数据包
     * - 端口不匹配：忽略该数据包（可能是其他应用）
     *
     * 注意：此函数可能在一次调用中处理多个数据包，
     * 因为UDP接收是异步的，可能积累多个待处理包。
     */
    void processBroadcast();

private:
    /**
     * @brief 获取所有网卡的广播地址
     * @return 广播地址列表
     *
     * 扫描本机所有网络接口，提取支持广播的接口的广播地址。
     *
     * 实现步骤：
     * 1. 调用QNetworkInterface::allInterfaces()获取所有网卡
     * 2. 遍历每个网卡，检查是否支持广播（CanBroadcast标志）
     * 3. 对每个支持广播的网卡，遍历其地址条目
     * 4. 提取非空的广播地址
     * 5. 返回所有广播地址的列表
     *
     * 广播地址说明：
     * - 对于IPv4地址（如192.168.1.100/24），广播地址是192.168.1.255
     * - 对于不同网段，有不同的广播地址
     * - 向广播地址发送的数据包会被该网段所有主机接收
     *
     * 多网卡处理：
     * - 笔记本可能有WiFi和以太网两个接口
     * - 虚拟机可能有虚拟网卡
     * - 此方法会找到所有接口的广播地址
     *
     * 返回示例：
     * - [192.168.1.255, 192.168.0.255, 172.16.0.255]
     *
     * 用途：
     * - sendBroadcast()使用此列表向所有网段发送广播
     * - 确保多网卡环境下所有网段的设备都能被发现
     */
    QVector<QHostAddress> getBroadcastAddressFromInterfaces();

    QTimer mTimer;          ///< 定时器，用于周期性发送广播包（心跳机制）
    QUdpSocket mUdpSock;    ///< UDP socket，既用于发送广播也用于接收广播
};

#endif // DEVICEBROADCASTER_H
