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
 * @file devicelistmodel.cpp
 * @brief 设备列表模型类实现文件
 *
 * 本文件实现了 DeviceListModel 类的所有方法，包括 Qt 模型接口、设备管理和查询功能。
 * 核心功能是通过监听 DeviceBroadcaster 的信号，自动维护设备列表，并为视图提供数据。
 */

#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8，支持中文

#include <QPixmap>

#include "devicelistmodel.h"
#include "../ui/settings.h"

/**
 * @brief 构造函数实现
 *
 * 初始化模型并连接到 DeviceBroadcaster 的 broadcastReceived 信号。
 * 当收到设备广播时，会自动调用 onBCReceived 槽函数更新设备列表。
 */
DeviceListModel::DeviceListModel(DeviceBroadcaster* deviceBC, QObject* parent)
    : QAbstractListModel(parent)
{
    mDBC = deviceBC;
    if (!mDBC) {
        mDBC = new DeviceBroadcaster(this);
    }

    connect(mDBC, &DeviceBroadcaster::broadcastReceived, this, &DeviceListModel::onBCReceived);
}

/**
 * @brief 接收设备广播的槽函数实现
 *
 * 当接收到设备广播时：
 * 1. 首先过滤掉本机设备（通过 ID 比较）
 * 2. 检查设备是否已在列表中（通过 ID 查找）
 * 3. 如果是新设备，添加到列表末尾，并通知视图更新
 *
 * 使用 beginInsertRows/endInsertRows 确保视图正确更新显示
 */
void DeviceListModel::onBCReceived(const Device &fromDevice)
{
    QString id = fromDevice.getId();
    QString myId = Settings::instance()->getMyDevice().getId();

    qDebug() << "DeviceListModel: Received broadcast from device:"
             << fromDevice.getName()
             << "ID:" << id
             << "IP:" << fromDevice.getAddress().toString();

    // 过滤本机设备
    if (id == myId) {
        qDebug() << "  Ignoring self (my ID:" << myId << ")";
        return;
    }

    // 检查设备是否已存在
    bool found = false;
    int existingIndex = -1;
    for(int i = 0; i < mDevices.size(); ++i) {
        if (mDevices[i].getId() == id) {
            found = true;
            existingIndex = i;
            break;
        }
    }

    if (found) {
        // 更新已存在设备的信息（IP地址可能变化）
        qDebug() << "  Device already in list at index" << existingIndex << ", updating info";
        mDevices[existingIndex] = fromDevice;
        QModelIndex idx = index(existingIndex);
        emit dataChanged(idx, idx);
    } else {
        // 添加新设备
        qDebug() << "  New device discovered! Adding to list at position" << mDevices.size();
        beginInsertRows(QModelIndex(), mDevices.size(), mDevices.size());
        mDevices.push_back(fromDevice);
        endInsertRows();
        qDebug() << "  Total devices in list now:" << mDevices.size();
    }
}

/**
 * @brief 获取所有设备
 *
 * @return 设备列表的副本
 */
QVector<Device> DeviceListModel::getDevices() const
{
    return mDevices;
}

/**
 * @brief 设置设备列表
 *
 * 使用 beginResetModel/endResetModel 通知视图数据已完全改变，
 * 视图会重新查询所有数据并刷新显示。
 */
void DeviceListModel::setDevices(const QVector<Device> &devices)
{
    beginResetModel();
    mDevices = devices;
    endResetModel();
}

/**
 * @brief 获取指定索引的数据（Qt 模型接口实现）
 *
 * 根据不同的角色返回不同的数据：
 * - DisplayRole：显示文本，格式为 "设备名 (操作系统)"
 * - ToolTipRole：鼠标悬停时的提示信息，包含设备详细信息
 * - DecorationRole：显示图标，根据操作系统返回对应的图标
 *
 * 视图在渲染每一项时会多次调用此方法，每次传入不同的 role
 */
QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        Device dev = mDevices[index.row()];
        switch (role) {
        case Qt::DisplayRole : {
            // 显示文本："设备名 (操作系统)"
            return dev.getName() + "  (" + dev.getOSName() + ")";
        }
        case Qt::ToolTipRole : {
            // 工具提示：显示设备详细信息（HTML 格式）
            QString str = dev.getId() + "<br>" +
                          dev.getName() + " (" + dev.getOSName() + ")<br>" +
                          dev.getAddress().toString();
            return str;
        }
        case Qt::DecorationRole : {
            // 图标：根据操作系统返回对应图标
            QString os = dev.getOSName();
            if (os == "Linux") {
                return QPixmap(":/img/linux.png");
            } else if (os == "Windows") {
                return QPixmap(":/img/windows.png");
            } else if (os == "Mac OSX") {
                return QPixmap(":/img/osx.png");
            }
        }
        }

    }

    return QVariant();
}

/**
 * @brief 获取行数（Qt 模型接口实现）
 *
 * 返回设备列表中的设备数量，视图据此确定显示多少行
 */
int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mDevices.size();
}

/**
 * @brief 刷新设备列表
 *
 * 清空当前设备列表并通知视图。
 * 之后需要重新进行设备发现才能再次填充列表。
 */
void DeviceListModel::refresh()
{
    qDebug() << "DeviceListModel: Refreshing device list, clearing" << mDevices.size() << "devices";
    beginResetModel();
    mDevices.clear();
    endResetModel();
}

/**
 * @brief 触发设备发现广播
 *
 * 立即发送一次广播，让其他设备知道本机存在。
 * 其他设备收到广播后也会回应，从而更新设备列表。
 */
void DeviceListModel::triggerBroadcast()
{
    qDebug() << "DeviceListModel: Triggering broadcast to refresh device discovery";
    if (mDBC) {
        mDBC->sendBroadcast();
    } else {
        qDebug() << "DeviceListModel: Warning - DeviceBroadcaster is null";
    }
}

/**
 * @brief 根据索引获取设备
 *
 * @param index 设备索引（0-based）
 * @return 设备对象，索引无效时返回空的 Device
 */
Device DeviceListModel::device(int index) const
{
    if (index < 0 || index >= mDevices.size()) {
        return Device();
    }

    return mDevices.at(index);
}

/**
 * @brief 根据设备 ID 获取设备
 *
 * 遍历设备列表查找匹配的设备
 *
 * @param id 设备唯一 ID
 * @return 设备对象，未找到时返回空的 Device
 */
Device DeviceListModel::device(const QString &id) const
{
    for (Device dev : mDevices) {
        if (dev.getId() == id) {
            return dev;
        }
    }

    return Device();
}

/**
 * @brief 根据 IP 地址获取设备
 *
 * 遍历设备列表查找匹配的设备
 *
 * @param address IP 地址
 * @return 设备对象，未找到时返回空的 Device
 */
Device DeviceListModel::device(const QHostAddress &address) const
{
    for (Device dev : mDevices) {
        if (dev.getAddress() == address) {
            return dev;
        }
    }

    return Device();
}
