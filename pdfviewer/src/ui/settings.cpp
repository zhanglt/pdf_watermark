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
 * @file settings.cpp
 * @brief 应用程序设置管理类实现
 *
 * 本文件实现了 Settings 类的所有方法，包括：
 * - 单例模式的初始化
 * - 本地设备信息的自动检测和配置
 * - 从配置文件加载设置
 * - 保存设置到配置文件
 * - 各种配置参数的 getter 和 setter 方法
 * - 重置为默认设置
 */

#include <QNetworkInterface>
#include <QHostInfo>
#include <QUuid>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>

#include "settings.h"

// 默认配置参数定义
#define DefaultBroadcastPort        6817        // 默认UDP广播端口
#define DefaultTransferPort         6818        // 默认TCP传输端口
#define DefaultBroadcastInterval    5000        // 默认广播间隔：5秒
#define DefaultFileBufferSize       98304       // 默认文件缓冲区大小：96 KB
#define MaxFileBufferSize           1024*1024   // 最大缓冲区大小：1 MB

// 初始化单例实例指针
Settings* Settings::obj = new Settings;

/**
 * @brief Settings 构造函数
 *
 * 初始化流程：
 * 1. 创建设备对象并生成唯一ID（UUID）
 * 2. 检测本地IPv4地址
 * 3. 设置操作系统名称
 * 4. 从配置文件加载用户设置
 *
 * 地址检测：遍历所有网络接口，找到第一个非回环的IPv4地址作为设备地址
 */
Settings::Settings()
{
    // 创建本地设备对象
    mThisDevice = Device();

    // 为设备生成唯一ID（UUID格式）
    mThisDevice.setId(QUuid::createUuid().toString());

    // 初始设置为本地回环地址，后续会被真实IP替换
    mThisDevice.setAddress(QHostAddress::LocalHost);

    // 设置操作系统名称（由宏 OS_NAME 定义）
    mThisDevice.setOSName(OS_NAME);

    // 遍历所有网络接口，查找有效的IPv4地址
    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        // 检查是否为IPv4协议且不是回环地址（127.0.0.1）
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress::LocalHost) {
            // 找到第一个有效的局域网IP地址，设置为设备地址
            mThisDevice.setAddress(address);
            break;  // 找到后立即退出循环
        }
    }

    // 从配置文件加载用户保存的设置
    loadSettings();
}

/**
 * @brief 设置设备名称
 * @param name 新的设备名称
 */
void Settings::setDeviceName(const QString &name)
{
    mThisDevice.setName(name);
}

/**
 * @brief 设置设备IP地址
 * @param address 选择的IP地址
 */
void Settings::setDeviceAddress(const QHostAddress &address)
{
    mThisDevice.setAddress(address);
}


/**
 * @brief 设置 UDP 广播端口
 * @param port 新的广播端口号
 *
 * 只有在端口号大于 0 时才会更新设置
 */
void Settings::setBroadcastPort(quint16 port)
{
    if (port > 0)
        mBCPort = port;
}

/**
 * @brief 设置广播间隔时间
 * @param interval 新的广播间隔（毫秒）
 */
void Settings::setBroadcastInterval(quint16 interval)
{
    mBCInterval = interval;
}

/**
 * @brief 设置 TCP 传输端口
 * @param port 新的传输端口号
 *
 * 只有在端口号大于 0 时才会更新设置
 */
void Settings::setTransferPort(quint16 port)
{
    if (port > 0)
        mTransferPort = port;
}

/**
 * @brief 设置文件缓冲区大小
 * @param size 新的缓冲区大小（字节）
 *
 * 只有在大小大于 0 且小于最大限制（1MB）时才会更新设置
 */
void Settings::setFileBufferSize(qint32 size)
{
    if (size > 0 && size < MaxFileBufferSize)
        mFileBuffSize = size;
}

/**
 * @brief 设置下载目录
 * @param dir 新的下载目录路径
 *
 * 只有在目录不为空且确实存在时才会更新设置
 */
void Settings::setDownloadDir(const QString& dir)
{
    if (!dir.isEmpty() && QDir(dir).exists())
        mDownloadDir = dir;
}

/**
 * @brief 设置是否覆盖已存在文件
 * @param replace true 表示覆盖已存在文件，false 表示自动重命名
 */
void Settings::setReplaceExistingFile(bool replace)
{
    mReplaceExistingFile = replace;
}

/**
 * @brief 从配置文件加载设置
 *
 * 从 LANSConfig 配置文件读取用户保存的设置参数。
 * 如果配置文件不存在或某个参数缺失，则使用默认值。
 *
 * 加载的参数包括：
 * - 设备名称（默认为本地主机名）
 * - UDP 广播端口（默认 56780）
 * - TCP 传输端口（默认 17116）
 * - 文件缓冲区大小（默认 96KB）
 * - 下载目录（默认为系统下载文件夹/LANShareDownloads）
 * - 广播间隔（默认 5000ms）
 * - 是否覆盖已存在文件（默认 false）
 *
 * 如果下载目录不存在，会自动创建该目录
 */
void Settings::loadSettings()
{
    QSettings settings(SETTINGS_FILE);

    // 加载设备名称，默认使用本地主机名
    mThisDevice.setName(settings.value("DeviceName", QHostInfo::localHostName()).toString());

    // 加载保存的IP地址（如果有）
    QString savedIpStr = settings.value("DeviceIpAddress", "").toString();
    if (!savedIpStr.isEmpty()) {
        QHostAddress savedIp(savedIpStr);
        // 验证保存的IP地址是否仍然有效（在当前可用地址列表中）
        QVector<QHostAddress> availableAddrs = getAllAvailableAddresses();
        if (availableAddrs.contains(savedIp)) {
            mThisDevice.setAddress(savedIp);
        }
    }

    // 加载网络端口配置
    mBCPort = settings.value("BroadcastPort", DefaultBroadcastPort).value<quint16>();
    mTransferPort = settings.value("TransferPort", DefaultTransferPort).value<quint16>();

    // 加载文件传输配置
    mFileBuffSize = settings.value("FileBufferSize", DefaultFileBufferSize).value<quint32>();
    mDownloadDir = settings.value("DownloadDir", getDefaultDownloadPath()).toString();

    // 确保下载目录存在，如果不存在则创建
    if (!QDir(mDownloadDir).exists()) {
        QDir dir;
        dir.mkpath(mDownloadDir);
    }

    // 加载其他配置参数
    mBCInterval = settings.value("BroadcastInterval", DefaultBroadcastInterval).value<quint16>();
    mReplaceExistingFile = settings.value("ReplaceExistingFile", false).toBool();
}

/**
 * @brief 获取默认下载路径
 * @return 根据操作系统确定的默认下载目录路径
 *
 * Windows: 系统下载文件夹\LANShareDownloads
 * Linux/Mac: 用户主目录/LANShareDownloads
 */
QString Settings::getDefaultDownloadPath()
{
#if defined (Q_OS_WIN)
    // Windows 系统：使用系统标准的下载文件夹
    return
        QStandardPaths::locate(QStandardPaths::DownloadLocation, QString(), QStandardPaths::LocateDirectory) + "LANShareDownloads";
#else
    // Linux/Mac 系统：使用用户主目录
    return QDir::homePath() + QDir::separator() + "LANShareDownloads";
#endif
}

/**
 * @brief 保存当前设置到配置文件
 *
 * 将所有配置参数持久化存储到 LANSConfig 文件中，
 * 以便下次启动时恢复用户的设置
 */
void Settings::saveSettings()
{
    QSettings settings(SETTINGS_FILE);

    // 保存设备信息
    settings.setValue("DeviceName", mThisDevice.getName());
    settings.setValue("DeviceIpAddress", mThisDevice.getAddress().toString());

    // 保存网络端口配置
    settings.setValue("BroadcastPort", mBCPort);
    settings.setValue("TransferPort", mTransferPort);

    // 保存文件传输配置
    settings.setValue("FileBufferSize", mFileBuffSize);
    settings.setValue("DownloadDir", mDownloadDir);

    // 保存其他配置参数
    settings.setValue("BroadcastInterval", mBCInterval);
    settings.setValue("ReplaceExistingFile", mReplaceExistingFile);
}

/**
 * @brief 重置所有设置为默认值
 *
 * 将所有配置参数恢复为默认值：
 * - 设备名称：本地主机名
 * - UDP 广播端口：56780
 * - TCP 传输端口：17116
 * - 广播间隔：5000ms
 * - 文件缓冲区大小：96KB
 * - 下载目录：默认路径
 *
 * 注意：此方法仅重置内存中的值，不会立即保存到文件，
 *       需要调用 saveSettings() 来持久化重置后的设置
 */
void Settings::reset()
{
    mThisDevice.setName(QHostInfo::localHostName());
    mBCPort = DefaultBroadcastPort;
    mTransferPort = DefaultTransferPort;
    mBCInterval = DefaultBroadcastInterval;
    mFileBuffSize = DefaultFileBufferSize;
    mDownloadDir = getDefaultDownloadPath();
}

/**
 * @brief 获取 UDP 广播端口
 * @return 广播端口号
 */
quint16 Settings::getBroadcastPort() const
{
    return mBCPort;
}

/**
 * @brief 获取 TCP 传输端口
 * @return 传输端口号
 */
quint16 Settings::getTransferPort() const
{
    return mTransferPort;
}

/**
 * @brief 获取广播间隔时间
 * @return 广播间隔（毫秒）
 */
quint16 Settings::getBroadcastInterval() const
{
    return mBCInterval;
}

/**
 * @brief 获取文件缓冲区大小
 * @return 缓冲区大小（字节）
 */
qint32 Settings::getFileBufferSize() const
{
    return mFileBuffSize;
}

/**
 * @brief 获取下载目录路径
 * @return 下载目录的完整路径
 */
QString Settings::getDownloadDir() const
{
    return mDownloadDir;
}

/**
 * @brief 获取本地设备对象
 * @return Device 对象，包含完整的设备信息
 */
Device Settings::getMyDevice() const
{
    return mThisDevice;
}

/**
 * @brief 获取设备唯一ID
 * @return 设备ID字符串（UUID格式）
 */
QString Settings::getDeviceId() const
{
    return mThisDevice.getId();
}

/**
 * @brief 获取设备名称
 * @return 设备名称字符串
 */
QString Settings::getDeviceName() const
{
    return mThisDevice.getName();
}

/**
 * @brief 获取设备IP地址
 * @return QHostAddress 对象，表示设备的IPv4地址
 */
QHostAddress Settings::getDeviceAddress() const
{
    return mThisDevice.getAddress();
}

/**
 * @brief 获取所有可用的IPv4地址
 * @return 包含所有非回环IPv4地址的列表
 */
QVector<QHostAddress> Settings::getAllAvailableAddresses() const
{
    QVector<QHostAddress> addresses;
    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress::LocalHost) {
            addresses.append(address);
        }
    }
    return addresses;
}


/**
 * @brief 获取是否覆盖已存在文件的设置
 * @return true 表示覆盖，false 表示自动重命名
 */
bool Settings::getReplaceExistingFile() const
{
    return mReplaceExistingFile;
}

