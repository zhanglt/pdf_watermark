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
 * @file devicelistmodel.h
 * @brief 设备列表模型类头文件
 *
 * 本文件定义了 DeviceListModel 类，这是一个基于 QAbstractListModel 的 Qt 模型类，
 * 用于在 UI 层展示局域网中发现的设备列表。
 *
 * DeviceListModel 遵循 Qt 的模型-视图(Model-View)架构：
 * - 模型(Model)：DeviceListModel，管理设备数据
 * - 视图(View)：QListView 或 QComboBox，显示设备列表
 * - 数据源：DeviceBroadcaster，通过 UDP 广播发现设备
 *
 * 主要功能：
 * - 自动监听 DeviceBroadcaster 的广播消息，动态添加发现的设备
 * - 提供 Qt 标准模型接口（rowCount、data）供视图使用
 * - 支持按索引、ID、地址查询设备
 * - 提供刷新和手动设置设备列表的功能
 */

#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractListModel>

#include "../transfer/devicebroadcaster.h"
#include "device.h"

/**
 * @class DeviceListModel
 * @brief 设备列表模型类
 *
 * DeviceListModel 是一个基于 QAbstractListModel 的 Qt 模型类，用于管理和展示
 * 局域网中发现的设备列表。它实现了 Qt 模型-视图架构中的模型角色。
 *
 * 工作原理：
 * 1. 通过 DeviceBroadcaster 监听局域网中的设备广播
 * 2. 当收到广播时，自动将新设备添加到列表中
 * 3. 通过 Qt 信号机制通知视图更新显示
 * 4. 视图调用 data() 方法获取显示内容
 *
 * 数据角色支持：
 * - Qt::DisplayRole：返回设备名称和操作系统（格式："设备名 (操作系统)"）
 * - Qt::ToolTipRole：返回设备详细信息（ID、名称、操作系统、IP地址）
 * - Qt::DecorationRole：返回操作系统图标（Windows/Linux/Mac OSX）
 *
 * 典型使用场景：
 * @code
 * // 创建模型并绑定到视图
 * DeviceBroadcaster* broadcaster = new DeviceBroadcaster();
 * DeviceListModel* model = new DeviceListModel(broadcaster);
 * QListView* view = new QListView();
 * view->setModel(model);
 *
 * // 用户选择设备后获取设备信息
 * int index = view->currentIndex().row();
 * Device selectedDevice = model->device(index);
 * @endcode
 */
class DeviceListModel : public QAbstractListModel
{
public:
    /**
     * @brief 构造函数
     * @param deviceBC DeviceBroadcaster 对象指针，用于接收设备广播
     * @param parent Qt 父对象，用于对象树管理
     *
     * 创建设备列表模型并连接到 DeviceBroadcaster 的信号。
     * 如果传入 nullptr，会自动创建一个新的 DeviceBroadcaster。
     */
    DeviceListModel(DeviceBroadcaster* deviceBC, QObject* parent = nullptr);

    // === Qt 模型接口实现 ===

    /**
     * @brief 获取指定索引的数据（Qt 模型接口）
     * @param index 模型索引
     * @param role 数据角色
     * @return QVariant 包装的数据
     *
     * 根据不同的角色返回不同的数据：
     * - Qt::DisplayRole：返回 "设备名 (操作系统)" 格式的字符串
     * - Qt::ToolTipRole：返回设备详细信息的 HTML 字符串
     * - Qt::DecorationRole：返回操作系统对应的图标 (QPixmap)
     *
     * 视图会调用此方法获取显示内容
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief 获取行数（Qt 模型接口）
     * @param parent 父索引（列表模型中未使用）
     * @return 设备列表中的设备数量
     *
     * 视图会调用此方法确定显示多少行
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // === 设备列表管理 ===

    /**
     * @brief 刷新设备列表
     *
     * 清空当前设备列表，重新开始扫描。
     * 调用此方法后，需要重新进行设备发现。
     */
    void refresh();

    /**
     * @brief 触发设备发现广播
     *
     * 立即发送一次广播，让其他设备知道本机存在，
     * 同时也会触发其他设备回应广播。
     */
    void triggerBroadcast();

    // === 设备查询 ===

    /**
     * @brief 根据索引获取设备
     * @param index 设备在列表中的索引（0-based）
     * @return Device 对象，如果索引无效返回空的 Device
     */
    Device device(int index) const;

    /**
     * @brief 根据设备 ID 获取设备
     * @param id 设备唯一 ID（UUID 格式）
     * @return Device 对象，如果未找到返回空的 Device
     */
    Device device(const QString& id) const;

    /**
     * @brief 根据 IP 地址获取设备
     * @param address 设备的 IP 地址
     * @return Device 对象，如果未找到返回空的 Device
     */
    Device device(const QHostAddress& address) const;

    // === 批量操作 ===

    /**
     * @brief 获取所有设备
     * @return 包含所有设备的向量
     */
    QVector<Device> getDevices() const;

    /**
     * @brief 设置设备列表
     * @param getDevices 新的设备列表
     *
     * 替换当前的设备列表，会触发模型重置，通知视图刷新
     */
    void setDevices(const QVector<Device> &getDevices);

private Q_SLOTS:
    /**
     * @brief 接收到设备广播的槽函数
     * @param fromDevice 广播发送方的设备信息
     *
     * 当 DeviceBroadcaster 接收到广播时调用。
     * 会过滤掉本机设备和已存在的设备，只添加新设备到列表中。
     */
    void onBCReceived(const Device &fromDevice);

private:
    DeviceBroadcaster* mDBC;    ///< 设备广播器指针，用于接收设备广播
    QVector<Device> mDevices;   ///< 设备列表，存储所有发现的设备
};

#endif // DEVICELISTMODEL_H
