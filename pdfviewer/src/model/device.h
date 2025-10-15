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
 * @file device.h
 * @brief 设备信息类头文件
 *
 * 本文件定义了 Device 类，用于表示局域网中的一个设备节点。
 * Device 类封装了设备的基本信息，包括唯一ID、名称、IP地址和操作系统类型。
 *
 * 在 LAN Share 中，每个运行程序的计算机都被抽象为一个 Device 对象，
 * 通过 UDP 广播在局域网中发现彼此，并建立点对点的文件传输连接。
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <QtNetwork/QHostAddress>
#include <QObject>
#include <QMetaType>

/**
 * @class Device
 * @brief 设备信息类
 *
 * Device 类表示局域网中的一个设备节点（计算机）。
 * 每个设备包含以下信息：
 * - 唯一ID：UUID格式的字符串，用于区分不同设备
 * - 设备名称：用户可见的设备名称（通常为主机名）
 * - IP地址：设备在局域网中的IPv4地址
 * - 操作系统：设备运行的操作系统名称（Windows/Linux/Mac）
 *
 * 设备对象用于：
 * - 设备发现：通过 UDP 广播交换设备信息
 * - 设备列表：在 UI 中显示可用的传输目标
 * - 文件传输：建立 TCP 连接时识别对方设备
 */
class Device
{
public:
    /**
     * @brief 默认构造函数
     *
     * 创建一个空的设备对象，所有字段为默认值
     */
    explicit Device() = default;

    /**
     * @brief 完整构造函数
     * @param id 设备唯一ID（UUID格式）
     * @param name 设备名称
     * @param osName 操作系统名称
     * @param addr IP地址
     *
     * 使用指定的参数创建设备对象
     */
    Device(const QString &id, const QString &name, const QString &osName, const QHostAddress &addr);

    // === Getter 方法 ===

    /**
     * @brief 获取设备唯一ID
     * @return 设备ID字符串（UUID格式）
     */
    inline QString getId() const { return mId; }

    /**
     * @brief 获取设备名称
     * @return 设备名称字符串
     */
    inline QString getName() const { return mName; }

    /**
     * @brief 获取设备IP地址
     * @return QHostAddress 对象
     */
    inline QHostAddress getAddress() const { return mAddress; }

    /**
     * @brief 获取操作系统名称
     * @return 操作系统名称字符串
     */
    inline QString getOSName() const { return mOSName; }

    /**
     * @brief 检查设备信息是否有效
     * @return true 表示所有必需字段都已设置，false 表示有字段为空
     *
     * 有效条件：ID、名称、操作系统都不为空，且IP地址不为Null
     */
    bool isValid() const;

    // === Setter 方法 ===

    /**
     * @brief 设置设备唯一ID
     * @param id 设备ID字符串
     */
    void setId(const QString& id);

    /**
     * @brief 设置设备名称
     * @param name 设备名称字符串
     */
    void setName(const QString& name);

    /**
     * @brief 设置设备IP地址
     * @param address IP地址对象
     */
    void setAddress(const QHostAddress& address);

    /**
     * @brief 设置操作系统名称
     * @param osName 操作系统名称字符串
     */
    void setOSName(const QString& osName);

    // === 操作符重载 ===

    /**
     * @brief 相等比较操作符
     * @param other 另一个设备对象
     * @return true 表示两个设备的ID、名称和地址都相同
     *
     * 用于判断两个设备对象是否表示同一台设备
     */
    bool operator==(const Device& other) const;

    /**
     * @brief 不等比较操作符
     * @param other 另一个设备对象
     * @return true 表示两个设备不同
     */
    bool operator!=(const Device& other) const;

private:
    QString mId{""};                              ///< 设备唯一ID（UUID格式）
    QString mName{""};                            ///< 设备名称
    QString mOSName{""};                          ///< 操作系统名称
    QHostAddress mAddress{QHostAddress::Null};    ///< 设备IP地址
};

// 声明 Device 类型为 Qt 元类型，以便在信号槽中使用
Q_DECLARE_METATYPE(Device)

#endif // DEVICE_H
