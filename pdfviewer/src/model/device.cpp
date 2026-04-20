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
 * @file device.cpp
 * @brief 设备信息类实现
 *
 * 本文件实现了 Device 类的所有方法，
 * 包括构造函数、getter/setter 方法和操作符重载。
 */

#include "device.h"

/**
 * @brief 完整构造函数
 * @param id 设备唯一ID
 * @param name 设备名称
 * @param osName 操作系统名称
 * @param addr IP地址
 *
 * 使用成员初始化列表初始化所有成员变量
 */
Device::Device(const QString &id, const QString &name, const QString &osName, const QHostAddress &addr)
: mId{id}, mName{name}, mOSName{osName}, mAddress{addr}
{
}

/**
 * @brief 检查设备信息是否有效
 * @return true 表示所有字段都有效，false 表示有字段为空或无效
 *
 * 验证条件：
 * - ID 不为空字符串
 * - 名称不为空字符串
 * - 操作系统不为空字符串
 * - IP地址不为 Null
 *
 * 只有所有条件都满足时才认为设备有效
 */
bool Device::isValid() const
{
    return (mId != "" && mName != "" && mOSName != "" && mAddress != QHostAddress::Null);
}

/**
 * @brief 设置设备唯一ID
 * @param id 新的设备ID
 */
void Device::setId(const QString& id)
{
    mId = id;
}

/**
 * @brief 设置设备名称
 * @param name 新的设备名称
 */
void Device::setName(const QString& name)
{
    mName = name;
}

/**
 * @brief 设置设备IP地址
 * @param address 新的IP地址
 */
void Device::setAddress(const QHostAddress& address)
{
    mAddress = address;
}

/**
 * @brief 设置操作系统名称
 * @param osName 新的操作系统名称
 */
void Device::setOSName(const QString& osName)
{
    mOSName = osName;
}

/**
 * @brief 相等比较操作符
 * @param other 另一个设备对象
 * @return true 表示两个设备相同
 *
 * 比较标准：ID、名称和地址都相同
 * 注意：操作系统名称不参与比较
 */
bool Device::operator==(const Device& other) const
{
    return mId == other.getId() && mName == other.getName() && mAddress == other.getAddress();
}

/**
 * @brief 不等比较操作符
 * @param other 另一个设备对象
 * @return true 表示两个设备不同
 *
 * 通过取反相等比较结果实现
 */
bool Device::operator!=(const Device& other) const
{
    return !((*this) == other);
}
