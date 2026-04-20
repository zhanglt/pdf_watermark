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
 * @file settings.h
 * @brief 应用程序设置管理类头文件
 *
 * 本文件定义了 Settings 类，用于管理 LAN Share 的所有配置参数，包括：
 * - 网络配置（广播端口、传输端口、广播间隔）
 * - 本地设备信息（设备名称、设备ID、IP地址）
 * - 文件传输配置（缓冲区大小、下载目录）
 * - 用户偏好设置（是否覆盖已存在文件）
 *
 * Settings 类采用单例模式，确保全局只有一个配置实例。
 * 配置数据持久化存储在 LANSConfig 文件中。
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QHostAddress>
#include <QString>

#include "../model/device.h"

// 根据编译平台定义操作系统名称宏
#if defined (Q_OS_WIN)
    #define OS_NAME "Windows"
#elif defined (Q_OS_OSX)
    #define OS_NAME "Mac OSX"
#elif defined (Q_OS_LINUX)
    #define OS_NAME "Linux"
#else
    #define OS_NAME "Unknown"
#endif

// 程序基本信息常量
const QString PROGRAM_NAME{"PDF工具"};  // 程序名称
const QString PROGRAM_DESC{"A simple program that let you transfer files over local area network (LAN) easily."};  // 程序描述
constexpr int PROGRAM_X_VER{1};  // 主版本号
constexpr int PROGRAM_Y_VER{2};  // 次版本号
constexpr int PROGRAM_Z_VER{1};  // 修订版本号
const QString SETTINGS_FILE{"LANSConfig"};  // 配置文件名

/**
 * @class Settings
 * @brief 应用程序设置管理类（单例模式）
 *
 * Settings 类负责管理 LAN Share 的所有配置参数：
 * - 从配置文件加载设置
 * - 保存设置到配置文件
 * - 提供配置参数的读写接口
 * - 管理本地设备信息
 *
 * 使用单例模式确保全局只有一个配置实例，通过 instance() 方法获取实例。
 *
 * 配置参数包括：
 * - 网络参数：UDP广播端口、TCP传输端口、广播间隔
 * - 设备信息：设备名称、唯一ID、IP地址、操作系统
 * - 传输参数：文件缓冲区大小、下载目录
 * - 用户偏好：是否覆盖已存在文件
 */
class Settings
{
public:
    /**
     * @brief 获取 Settings 类的单例实例
     * @return Settings 实例指针
     */
    static Settings* instance() { return obj; }

    // === Getter 方法：获取配置参数 ===

    /**
     * @brief 获取 UDP 广播端口
     * @return 广播端口号
     *
     * 用于设备发现的 UDP 广播端口，默认为 56780
     */
    quint16 getBroadcastPort() const;

    /**
     * @brief 获取 TCP 传输端口
     * @return 传输端口号
     *
     * 用于文件传输的 TCP 服务器端口，默认为 17116
     */
    quint16 getTransferPort() const;

    /**
     * @brief 获取广播间隔时间
     * @return 广播间隔（毫秒）
     *
     * 设备广播的时间间隔，默认为 5000 毫秒（5秒）
     */
    quint16 getBroadcastInterval() const;

    /**
     * @brief 获取文件缓冲区大小
     * @return 缓冲区大小（字节）
     *
     * 文件传输时使用的缓冲区大小，默认为 98304 字节（96KB）
     */
    qint32 getFileBufferSize() const;

    /**
     * @brief 获取下载目录路径
     * @return 下载目录的完整路径
     *
     * 接收文件的默认保存目录
     */
    QString getDownloadDir() const;

    /**
     * @brief 获取本地设备对象
     * @return Device 对象，包含完整的设备信息
     */
    Device getMyDevice() const;

    /**
     * @brief 获取设备唯一ID
     * @return 设备ID字符串（UUID格式）
     */
    QString getDeviceId() const;

    /**
     * @brief 获取设备名称
     * @return 设备名称字符串
     */
    QString getDeviceName() const;

    /**
     * @brief 获取设备IP地址
     * @return QHostAddress 对象，表示设备的IPv4地址
     */
    QHostAddress getDeviceAddress() const;

    /**
     * @brief 获取所有可用的IPv4地址
     * @return 包含所有非回环IPv4地址的列表
     */
    QVector<QHostAddress> getAllAvailableAddresses() const;

    /**
     * @brief 获取是否覆盖已存在文件的设置
     * @return true 表示覆盖，false 表示重命名
     */
    bool getReplaceExistingFile() const;

    // === Setter 方法：设置配置参数 ===

    /**
     * @brief 设置设备名称
     * @param name 新的设备名称
     */
    void setDeviceName(const QString& name);

    /**
     * @brief 设置设备IP地址
     * @param address 选择的IP地址
     */
    void setDeviceAddress(const QHostAddress& address);

    /**
     * @brief 设置 UDP 广播端口
     * @param port 新的广播端口号（必须大于0）
     */
    void setBroadcastPort(quint16 port);

    /**
     * @brief 设置 TCP 传输端口
     * @param port 新的传输端口号（必须大于0）
     */
    void setTransferPort(quint16 port);

    /**
     * @brief 设置广播间隔时间
     * @param interval 新的广播间隔（毫秒）
     */
    void setBroadcastInterval(quint16 interval);

    /**
     * @brief 设置文件缓冲区大小
     * @param size 新的缓冲区大小（必须大于0且小于1MB）
     */
    void setFileBufferSize(qint32 size);

    /**
     * @brief 设置下载目录
     * @param dir 新的下载目录路径（目录必须存在）
     */
    void setDownloadDir(const QString& dir);

    /**
     * @brief 设置是否覆盖已存在文件
     * @param replace true 表示覆盖，false 表示重命名
     */
    void setReplaceExistingFile(bool replace);

    // === 配置管理方法 ===

    /**
     * @brief 保存当前设置到配置文件
     *
     * 将所有配置参数持久化存储到 LANSConfig 文件中
     */
    void saveSettings();

    /**
     * @brief 重置所有设置为默认值
     *
     * 将所有配置参数恢复为默认值，但不立即保存到文件
     */
    void reset();

private:
    /**
     * @brief 私有构造函数（单例模式）
     *
     * 初始化设备信息和加载配置文件
     */
    Settings();

    /**
     * @brief 从配置文件加载设置
     *
     * 从 LANSConfig 文件读取保存的配置参数，
     * 如果文件不存在或参数缺失，则使用默认值
     */
    void loadSettings();

    /**
     * @brief 获取默认下载路径
     * @return 根据操作系统确定的默认下载目录路径
     *
     * Windows: 下载文件夹\LANShareDownloads
     * Linux/Mac: 用户主目录/LANShareDownloads
     */
    QString getDefaultDownloadPath();

    // === 私有成员变量 ===

    Device mThisDevice;           ///< 本地设备信息对象
    quint16 mBCPort{0};           ///< UDP 广播端口
    quint16 mTransferPort{0};     ///< TCP 传输端口
    quint16 mBCInterval{0};       ///< 广播间隔（毫秒）
    qint32 mFileBuffSize{0};      ///< 文件缓冲区大小（字节）
    QString mDownloadDir;         ///< 下载目录路径
    bool mReplaceExistingFile{false};  ///< 是否覆盖已存在文件

    static Settings* obj;  ///< 单例实例指针
};

#endif // SETTINGS_H
