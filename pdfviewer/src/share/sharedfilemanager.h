/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file sharedfilemanager.h
 * @brief 共享文件管理器类定义
 *
 * 本文件定义了 SharedFileManager 类，它是 LAN Share 共享功能的核心管理器。
 * 该类采用单例模式，负责管理所有的共享资源、访问控制和文件索引。
 *
 * 核心功能：
 * 1. 共享管理：添加、删除、更新共享项
 * 2. 数据持久化：使用 SQLite 数据库存储共享配置
 * 3. 文件索引：为共享文件夹建立和维护文件索引
 * 4. 访问控制：基于密码、白名单/黑名单的访问验证
 * 5. 访问日志：记录所有访问行为和统计数据
 * 6. 线程安全：使用读写锁保护共享数据
 *
 * 数据库结构：
 * - shares 表：存储共享配置和元数据
 * - file_index 表：存储共享文件夹的文件列表索引
 * - access_control 表：存储访问控制规则（白名单/黑名单）
 * - access_log 表：记录访问日志
 *
 * 设计思路：
 * - 单例模式：全局唯一实例，方便从任何地方访问
 * - 内存缓存：mSharedItems 缓存所有共享，避免频繁查询数据库
 * - 定时更新：自动更新文件索引，反映文件系统变化
 * - 读写分离：使用 QReadWriteLock 支持并发读取
 */

#ifndef SHAREDFILEMANAGER_H
#define SHAREDFILEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QReadWriteLock>
#include <QMap>
#include <QFileInfo>
#include <QHostAddress>
#include "shareditem.h"
#include "shareconfig.h"
#include "../model/device.h"

/**
 * @class SharedFileManager
 * @brief 共享文件管理器（单例）
 *
 * SharedFileManager 是共享功能的核心类，负责：
 * 1. 管理共享列表：添加、删除、更新、查询共享项
 * 2. 数据持久化：将共享配置存储到 SQLite 数据库
 * 3. 文件索引：为共享文件夹建立索引，加速浏览
 * 4. 访问控制：验证远程设备的访问权限
 * 5. 访问日志：记录和统计访问行为
 *
 * 单例模式实现：
 * - 私有构造函数：防止外部直接创建实例
 * - getInstance()：获取全局唯一实例
 * - static instance：存储单例指针
 *
 * 线程安全：
 * - 使用 QReadWriteLock 保护 mSharedItems
 * - 读操作使用 QReadLocker（允许并发）
 * - 写操作使用 QWriteLocker（独占访问）
 *
 * 使用示例：
 * @code
 * SharedFileManager* mgr = SharedFileManager::getInstance();
 * ShareConfig config;
 * config.name = "我的文档";
 * config.password = "123456";
 * QString shareId = mgr->addShare("/path/to/folder", config);
 * @endcode
 */
class SharedFileManager : public QObject {
    Q_OBJECT

private:
    static SharedFileManager* instance;         ///< 单例实例指针
    QSqlDatabase mDatabase;                     ///< SQLite 数据库连接
    QTimer* mIndexUpdateTimer;                  ///< 文件索引更新定时器（5分钟）
    QMap<QString, SharedItem> mSharedItems;     ///< 共享项缓存（ID -> SharedItem）
    mutable QReadWriteLock mLock;               ///< 读写锁，保护 mSharedItems

    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     *
     * 初始化数据库、加载共享列表、启动定时器
     */
    SharedFileManager(QObject* parent = nullptr);

    /**
     * @brief 初始化数据库
     *
     * 创建 SQLite 数据库文件 shares.db，并建立以下表：
     * - shares：共享配置表
     * - file_index：文件索引表
     * - access_control：访问控制规则表
     * - access_log：访问日志表
     */
    void initDatabase();

    /**
     * @brief 从数据库加载共享列表到内存
     *
     * 程序启动时调用，将 shares 表的所有记录读取到 mSharedItems 缓存
     */
    void loadSharedItems();

    /**
     * @brief 更新共享文件夹的文件索引
     * @param shareId 共享ID
     *
     * 重新扫描文件夹，更新 file_index 表。
     * 只对 Folder 类型的共享有效。
     */
    void updateFileIndex(const QString& shareId);

    /**
     * @brief 递归索引目录
     * @param shareId 共享ID
     * @param basePath 共享根目录的绝对路径
     * @param relativePath 当前目录相对于根目录的路径
     *
     * 递归遍历目录，将所有文件和子文件夹的信息插入 file_index 表
     */
    void indexDirectory(const QString& shareId, const QString& basePath, const QString& relativePath);

    /**
     * @brief 递归计算目录大小
     * @param path 目录路径
     * @return 目录总大小（字节）
     *
     * 遍历目录及其子目录，累加所有文件的大小
     */
    qint64 calculateDirectorySize(const QString& path) const;

    /**
     * @brief 检查IP地址是否匹配模式
     * @param address 要检查的IP地址
     * @param pattern IP模式字符串
     * @return 是否匹配
     *
     * 当前实现为精确匹配，未来可扩展支持通配符（如 192.168.1.*）
     */
    bool matchesIpPattern(const QHostAddress& address, const QString& pattern) const;

public:
    /**
     * @brief 获取单例实例
     * @return SharedFileManager 唯一实例指针
     *
     * 线程安全的单例访问接口。首次调用时创建实例。
     */
    static SharedFileManager* getInstance();

    // ========== 共享管理接口 ==========

    /**
     * @brief 添加共享
     * @param path 文件或文件夹的本地路径
     * @param config 共享配置（名称、密码、权限等）
     * @return 共享ID（UUID），失败返回空字符串
     *
     * 功能：
     * 1. 验证路径是否存在
     * 2. 生成唯一的共享ID
     * 3. 插入数据库 shares 表
     * 4. 添加到内存缓存
     * 5. 如果是文件夹，建立文件索引
     * 6. 发射 shareAdded 信号
     */
    QString addShare(const QString& path, const ShareConfig& config);

    /**
     * @brief 删除共享
     * @param shareId 共享ID
     * @return 是否成功
     *
     * 功能：
     * 1. 删除 file_index 表中的索引
     * 2. 删除 access_control 表中的规则
     * 3. 删除 shares 表中的记录
     * 4. 从内存缓存中移除
     * 5. 发射 shareRemoved 信号
     */
    bool removeShare(const QString& shareId);

    /**
     * @brief 更新共享配置
     * @param shareId 共享ID
     * @param config 新的配置
     * @return 是否成功
     *
     * 更新共享的名称、密码、权限、描述等配置信息
     */
    bool updateShare(const QString& shareId, const ShareConfig& config);

    /**
     * @brief 设置共享的激活状态
     * @param shareId 共享ID
     * @param active 是否激活
     * @return 是否成功
     *
     * 激活状态控制共享是否对外可见。
     * 设为 false 可临时禁用共享而不删除配置。
     */
    bool setShareActive(const QString& shareId, bool active);

    // ========== 查询接口 ==========

    /**
     * @brief 获取所有共享项列表
     * @return SharedItem 列表
     *
     * 从内存缓存返回，包括激活和未激活的共享
     */
    QList<SharedItem> getSharedItems() const;

    /**
     * @brief 获取指定共享项
     * @param shareId 共享ID
     * @return SharedItem 对象，不存在则返回默认构造的对象
     */
    SharedItem getSharedItem(const QString& shareId) const;

    /**
     * @brief 获取共享列表的 JSON 表示
     * @return JSON 对象，包含设备信息和共享列表
     *
     * 用于网络传输，只包含激活的共享。
     * JSON 格式：
     * {
     *   "version": "2.0",
     *   "device": {"id": "...", "name": "..."},
     *   "shares": [
     *     {"id": "...", "name": "...", "type": "...", "size": ..., ...}
     *   ]
     * }
     */
    QJsonObject getShareListJson() const;

    /**
     * @brief 浏览共享文件夹的内容
     * @param shareId 共享ID
     * @param relativePath 相对路径（默认为根目录）
     * @return JSON 对象，包含文件和文件夹列表
     *
     * 功能：
     * 1. 验证共享存在
     * 2. 防止目录遍历攻击（路径安全检查）
     * 3. 返回指定路径下的文件和文件夹列表
     *
     * JSON 格式：
     * {
     *   "share_id": "...",
     *   "path": "...",
     *   "items": [
     *     {"name": "...", "type": "file|folder", "size": ..., "modified": "..."}
     *   ]
     * }
     */
    QJsonObject browseShare(const QString& shareId, const QString& relativePath = "") const;

    // ========== 访问控制 ==========

    /**
     * @brief 检查设备是否有权访问共享
     * @param shareId 共享ID
     * @param device 请求访问的设备
     * @param password 访问密码（可选）
     * @return 是否允许访问
     *
     * 验证逻辑：
     * 1. 检查共享是否激活
     * 2. 验证密码（如果设置了密码）
     * 3. 检查黑名单（在黑名单中则拒绝）
     * 4. 检查白名单（有白名单且不在其中则拒绝）
     * 5. 全部通过则允许访问
     */
    bool checkAccess(const QString& shareId, const Device& device, const QString& password = "") const;

    /**
     * @brief 记录访问日志
     * @param shareId 共享ID
     * @param device 访问的设备
     * @param action 操作类型（"browse"、"download" 等）
     *
     * 功能：
     * 1. 插入 access_log 表
     * 2. 如果是下载操作，增加 accessCount 计数
     */
    void recordAccess(const QString& shareId, const Device& device, const QString& action);

    // ========== 文件操作 ==========

    /**
     * @brief 获取共享文件的绝对路径
     * @param shareId 共享ID
     * @param relativePath 相对路径
     * @return 绝对路径，失败返回空字符串
     *
     * 包含路径安全检查，防止访问共享目录之外的文件
     */
    QString getAbsolutePath(const QString& shareId, const QString& relativePath) const;

    /**
     * @brief 获取共享文件的 QFileInfo
     * @param shareId 共享ID
     * @param relativePath 相对路径
     * @return QFileInfo 对象
     */
    QFileInfo getFileInfo(const QString& shareId, const QString& relativePath) const;

Q_SIGNALS:
    /**
     * @brief 共享添加信号
     * @param shareId 新添加的共享ID
     */
    void shareAdded(const QString& shareId);

    /**
     * @brief 共享删除信号
     * @param shareId 被删除的共享ID
     */
    void shareRemoved(const QString& shareId);

    /**
     * @brief 共享更新信号
     * @param shareId 被更新的共享ID
     */
    void shareUpdated(const QString& shareId);

    /**
     * @brief 索引更新信号
     * @param shareId 索引被更新的共享ID
     */
    void indexUpdated(const QString& shareId);

private Q_SLOTS:
    /**
     * @brief 定时器超时槽函数
     *
     * 每5分钟触发一次，更新所有激活的文件夹共享的索引
     */
    void onIndexUpdateTimeout();
};

#endif // SHAREDFILEMANAGER_H