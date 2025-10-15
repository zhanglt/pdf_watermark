/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file sharedfilemanager.cpp
 * @brief 共享文件管理器实现
 *
 * 本文件实现了 SharedFileManager 类的所有功能，包括：
 * - 数据库初始化和操作
 * - 共享项的增删改查
 * - 文件索引的建立和更新
 * - 访问控制和权限验证
 * - JSON 格式的数据交换
 *
 * 数据库表结构详解：
 *
 * 1. shares 表（主表）：
 *    - id: 共享唯一标识符（UUID）
 *    - path: 本地文件系统路径
 *    - name: 共享显示名称
 *    - type: 类型（"file" 或 "folder"）
 *    - size: 大小（字节）
 *    - permissions: 权限（"r" 或 "rw"）
 *    - password: 访问密码（可为空）
 *    - description: 描述信息
 *    - created_at, modified_at: 时间戳
 *    - access_count: 访问计数
 *    - is_active: 激活状态
 *
 * 2. file_index 表（文件索引）：
 *    - id: 索引项ID
 *    - share_id: 所属共享ID（外键）
 *    - path: 相对路径
 *    - name: 文件名
 *    - type: 类型
 *    - size: 大小
 *    - hash: 文件哈希（预留）
 *    - modified_at: 修改时间
 *
 * 3. access_control 表（访问控制）：
 *    - id: 规则ID
 *    - share_id: 共享ID（外键）
 *    - rule_type: 规则类型（"whitelist" 或 "blacklist"）
 *    - device_id: 设备ID
 *    - ip_pattern: IP 模式
 *
 * 4. access_log 表（访问日志）：
 *    - id: 日志ID
 *    - share_id: 共享ID
 *    - device_id, device_name: 设备信息
 *    - action: 操作类型
 *    - file_path: 访问的文件路径
 *    - timestamp: 时间戳
 *    - bytes_transferred: 传输字节数
 *    - status: 状态
 */

#include "sharedfilemanager.h"
#include "../ui/settings.h"
#include "../ui/util.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QtDebug>

// 单例实例指针初始化
SharedFileManager* SharedFileManager::instance = nullptr;

/**
 * 构造函数（私有）
 * 执行以下初始化：
 * 1. 初始化 SQLite 数据库
 * 2. 从数据库加载现有共享列表
 * 3. 启动定时器，每5分钟更新一次文件索引
 */
SharedFileManager::SharedFileManager(QObject* parent)
    : QObject(parent) {
    initDatabase();
    loadSharedItems();

    // 设置索引更新定时器
    mIndexUpdateTimer = new QTimer(this);
    mIndexUpdateTimer->setInterval(300000); // 5分钟
    connect(mIndexUpdateTimer, &QTimer::timeout,
            this, &SharedFileManager::onIndexUpdateTimeout);
    mIndexUpdateTimer->start();
}

/**
 * 获取单例实例（线程安全）
 * 延迟初始化：首次调用时创建实例
 */
SharedFileManager* SharedFileManager::getInstance() {
    if (!instance) {
        instance = new SharedFileManager();
    }
    return instance;
}

/**
 * 初始化数据库
 * 创建 shares.db 文件和所需的表结构
 */
void SharedFileManager::initDatabase() {
    mDatabase = QSqlDatabase::addDatabase("QSQLITE", "shares");
    mDatabase.setDatabaseName("shares.db");

    if (!mDatabase.open()) {
        qCritical() << "Failed to open shares database:"
                   << mDatabase.lastError().text();
        return;
    }

    QSqlQuery query(mDatabase);

    // 创建共享表
    query.exec("CREATE TABLE IF NOT EXISTS shares ("
               "id TEXT PRIMARY KEY,"
               "path TEXT NOT NULL,"
               "name TEXT NOT NULL,"
               "type TEXT NOT NULL,"
               "size INTEGER,"
               "permissions TEXT DEFAULT 'r',"
               "password TEXT,"
               "description TEXT,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "modified_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "access_count INTEGER DEFAULT 0,"
               "is_active BOOLEAN DEFAULT 1"
               ")");

    // 创建文件索引表
    query.exec("CREATE TABLE IF NOT EXISTS file_index ("
               "id TEXT PRIMARY KEY,"
               "share_id TEXT,"
               "path TEXT NOT NULL,"
               "name TEXT NOT NULL,"
               "type TEXT NOT NULL,"
               "size INTEGER,"
               "hash TEXT,"
               "modified_at TIMESTAMP,"
               "FOREIGN KEY (share_id) REFERENCES shares(id)"
               ")");

    // 创建访问控制表
    query.exec("CREATE TABLE IF NOT EXISTS access_control ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "share_id TEXT,"
               "rule_type TEXT,"
               "device_id TEXT,"
               "ip_pattern TEXT,"
               "FOREIGN KEY (share_id) REFERENCES shares(id)"
               ")");

    // 创建访问日志表
    query.exec("CREATE TABLE IF NOT EXISTS access_log ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "share_id TEXT,"
               "device_id TEXT,"
               "device_name TEXT,"
               "action TEXT,"
               "file_path TEXT,"
               "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "bytes_transferred INTEGER,"
               "status TEXT"
               ")");
}

void SharedFileManager::loadSharedItems() {
    QWriteLocker locker(&mLock);
    mSharedItems.clear();

    QSqlQuery query(mDatabase);
    query.exec("SELECT id, path, name, type, size, permissions, password, "
               "description, created_at, modified_at, access_count, is_active "
               "FROM shares");

    while (query.next()) {
        SharedItem item;
        item.id = query.value(0).toString();
        item.path = query.value(1).toString();
        item.name = query.value(2).toString();
        item.type = (query.value(3).toString() == "folder") ?
                    SharedItem::Folder : SharedItem::File;
        item.size = query.value(4).toLongLong();
        item.permissions = query.value(5).toString();
        item.config.password = query.value(6).toString();
        item.config.description = query.value(7).toString();
        item.createdAt = query.value(8).toDateTime();
        item.modifiedAt = query.value(9).toDateTime();
        item.accessCount = query.value(10).toInt();
        item.isActive = query.value(11).toBool();

        item.config.name = item.name;
        item.config.permissions = item.permissions;

        mSharedItems[item.id] = item;
    }
}

QString SharedFileManager::addShare(const QString& path,
                                    const ShareConfig& config) {
    QWriteLocker locker(&mLock);

    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        qWarning() << "Path does not exist:" << path;
        return QString();
    }

    QString shareId = QUuid::createUuid().toString();

    QSqlQuery query(mDatabase);
    query.prepare("INSERT INTO shares (id, path, name, type, size, "
                  "permissions, password, description) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(shareId);
    query.addBindValue(fileInfo.absoluteFilePath());
    query.addBindValue(config.name.isEmpty() ? fileInfo.fileName() : config.name);
    query.addBindValue(fileInfo.isDir() ? "folder" : "file");
    query.addBindValue(fileInfo.isDir() ?
                      calculateDirectorySize(path) : fileInfo.size());
    query.addBindValue(config.permissions);
    query.addBindValue(config.password);
    query.addBindValue(config.description);

    if (!query.exec()) {
        qWarning() << "Failed to add share:" << query.lastError().text();
        return QString();
    }

    // 创建SharedItem对象
    SharedItem item;
    item.id = shareId;
    item.path = fileInfo.absoluteFilePath();
    item.name = config.name.isEmpty() ? fileInfo.fileName() : config.name;
    item.type = fileInfo.isDir() ? SharedItem::Folder : SharedItem::File;
    item.size = fileInfo.isDir() ?
                calculateDirectorySize(path) : fileInfo.size();
    item.permissions = config.permissions;
    item.config = config;
    item.isActive = true;
    item.createdAt = QDateTime::currentDateTime();
    item.modifiedAt = QDateTime::currentDateTime();
    item.accessCount = 0;

    mSharedItems[shareId] = item;

    // 为文件夹创建索引
    if (fileInfo.isDir()) {
        locker.unlock(); // 避免死锁
        updateFileIndex(shareId);
    }

    emit shareAdded(shareId);
    return shareId;
}

bool SharedFileManager::removeShare(const QString& shareId) {
    QWriteLocker locker(&mLock);

    if (!mSharedItems.contains(shareId)) {
        return false;
    }

    QSqlQuery query(mDatabase);

    // 删除文件索引
    query.prepare("DELETE FROM file_index WHERE share_id = ?");
    query.addBindValue(shareId);
    query.exec();

    // 删除访问控制
    query.prepare("DELETE FROM access_control WHERE share_id = ?");
    query.addBindValue(shareId);
    query.exec();

    // 删除共享
    query.prepare("DELETE FROM shares WHERE id = ?");
    query.addBindValue(shareId);

    if (!query.exec()) {
        qWarning() << "Failed to remove share:" << query.lastError().text();
        return false;
    }

    mSharedItems.remove(shareId);
    emit shareRemoved(shareId);
    return true;
}

bool SharedFileManager::updateShare(const QString& shareId, const ShareConfig& config) {
    QWriteLocker locker(&mLock);

    if (!mSharedItems.contains(shareId)) {
        return false;
    }

    QSqlQuery query(mDatabase);
    query.prepare("UPDATE shares SET name = ?, permissions = ?, "
                  "password = ?, description = ?, modified_at = CURRENT_TIMESTAMP "
                  "WHERE id = ?");

    query.addBindValue(config.name);
    query.addBindValue(config.permissions);
    query.addBindValue(config.password);
    query.addBindValue(config.description);
    query.addBindValue(shareId);

    if (!query.exec()) {
        qWarning() << "Failed to update share:" << query.lastError().text();
        return false;
    }

    SharedItem& item = mSharedItems[shareId];
    item.name = config.name;
    item.permissions = config.permissions;
    item.config = config;
    item.modifiedAt = QDateTime::currentDateTime();

    emit shareUpdated(shareId);
    return true;
}

bool SharedFileManager::setShareActive(const QString& shareId, bool active) {
    QWriteLocker locker(&mLock);

    if (!mSharedItems.contains(shareId)) {
        return false;
    }

    QSqlQuery query(mDatabase);
    query.prepare("UPDATE shares SET is_active = ? WHERE id = ?");
    query.addBindValue(active);
    query.addBindValue(shareId);

    if (!query.exec()) {
        qWarning() << "Failed to update share active state:" << query.lastError().text();
        return false;
    }

    mSharedItems[shareId].isActive = active;
    emit shareUpdated(shareId);
    return true;
}

QList<SharedItem> SharedFileManager::getSharedItems() const {
    QReadLocker locker(&mLock);
    return mSharedItems.values();
}

SharedItem SharedFileManager::getSharedItem(const QString& shareId) const {
    QReadLocker locker(&mLock);
    return mSharedItems.value(shareId);
}

QJsonObject SharedFileManager::getShareListJson() const {
    QReadLocker locker(&mLock);

    QJsonObject result;
    result["version"] = "2.0";

    QJsonObject device;
    device["id"] = Settings::instance()->getDeviceId();
    device["name"] = Settings::instance()->getDeviceName();
    result["device"] = device;

    QJsonArray shares;
    for (const SharedItem& item : mSharedItems) {
        if (!item.isActive) continue;

        QJsonObject share;
        share["id"] = item.id;
        share["name"] = item.name;
        share["type"] = (item.type == SharedItem::Folder) ? "folder" : "file";
        share["size"] = static_cast<qint64>(item.size);
        share["permissions"] = item.permissions;
        share["description"] = item.config.description;
        share["hasPassword"] = !item.config.password.isEmpty();

        shares.append(share);
    }

    result["shares"] = shares;
    return result;
}

QJsonObject SharedFileManager::browseShare(const QString& shareId,
                                          const QString& relativePath) const {
    QReadLocker locker(&mLock);

    QJsonObject result;

    if (!mSharedItems.contains(shareId)) {
        result["error"] = "Share not found";
        return result;
    }

    const SharedItem& item = mSharedItems[shareId];
    QString fullPath = QDir(item.path).absoluteFilePath(relativePath);

    // 安全检查：防止目录遍历
    if (!fullPath.startsWith(item.path)) {
        result["error"] = "Invalid path";
        return result;
    }

    QFileInfo fileInfo(fullPath);
    if (!fileInfo.exists()) {
        result["error"] = "Path not found";
        return result;
    }

    result["share_id"] = shareId;
    result["path"] = relativePath;

    if (fileInfo.isDir()) {
        QDir dir(fullPath);
        QJsonArray items;

        for (const QFileInfo& entry : dir.entryInfoList(
                QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            QJsonObject itemObj;
            itemObj["name"] = entry.fileName();
            itemObj["type"] = entry.isDir() ? "folder" : "file";
            itemObj["size"] = static_cast<qint64>(entry.size());
            itemObj["modified"] = entry.lastModified().toString(Qt::ISODate);

            if (entry.isDir()) {
                itemObj["item_count"] = QDir(entry.absoluteFilePath())
                                       .entryList(QDir::Files | QDir::Dirs |
                                                 QDir::NoDotAndDotDot).count();
            }

            items.append(itemObj);
        }

        result["items"] = items;
    } else {
        result["error"] = "Not a directory";
    }

    return result;
}

bool SharedFileManager::checkAccess(const QString& shareId,
                                   const Device& device,
                                   const QString& password) const {
    QReadLocker locker(&mLock);

    if (!mSharedItems.contains(shareId)) {
        return false;
    }

    const SharedItem& item = mSharedItems[shareId];

    // 检查共享是否激活
    if (!item.isActive) {
        return false;
    }

    // 检查密码
    if (!item.config.password.isEmpty()) {
        if (item.config.password != password) {
            return false;
        }
    }

    // 检查访问控制列表
    QSqlQuery query(mDatabase);
    query.prepare("SELECT rule_type, device_id, ip_pattern "
                  "FROM access_control WHERE share_id = ?");
    query.addBindValue(shareId);

    if (query.exec()) {
        bool hasWhitelist = false;
        bool inWhitelist = false;
        bool inBlacklist = false;

        while (query.next()) {
            QString ruleType = query.value(0).toString();
            QString deviceId = query.value(1).toString();
            QString ipPattern = query.value(2).toString();

            if (ruleType == "whitelist") {
                hasWhitelist = true;
                if (deviceId == device.getId() ||
                    matchesIpPattern(device.getAddress(), ipPattern)) {
                    inWhitelist = true;
                }
            } else if (ruleType == "blacklist") {
                if (deviceId == device.getId() ||
                    matchesIpPattern(device.getAddress(), ipPattern)) {
                    inBlacklist = true;
                }
            }
        }

        if (inBlacklist) return false;
        if (hasWhitelist && !inWhitelist) return false;
    }

    return true;
}

void SharedFileManager::recordAccess(const QString& shareId,
                                    const Device& device,
                                    const QString& action) {
    QSqlQuery query(mDatabase);
    query.prepare("INSERT INTO access_log (share_id, device_id, device_name, action) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(shareId);
    query.addBindValue(device.getId());
    query.addBindValue(device.getName());
    query.addBindValue(action);
    query.exec();

    // 更新访问计数
    if (action == "download") {
        QWriteLocker locker(&mLock);
        if (mSharedItems.contains(shareId)) {
            mSharedItems[shareId].accessCount++;

            QSqlQuery updateQuery(mDatabase);
            updateQuery.prepare("UPDATE shares SET access_count = access_count + 1 WHERE id = ?");
            updateQuery.addBindValue(shareId);
            updateQuery.exec();
        }
    }
}

QString SharedFileManager::getAbsolutePath(const QString& shareId,
                                          const QString& relativePath) const {
    QReadLocker locker(&mLock);

    if (!mSharedItems.contains(shareId)) {
        return QString();
    }

    const SharedItem& item = mSharedItems[shareId];
    QString fullPath = QDir(item.path).absoluteFilePath(relativePath);

    // 安全检查
    if (!fullPath.startsWith(item.path)) {
        return QString();
    }

    return fullPath;
}

QFileInfo SharedFileManager::getFileInfo(const QString& shareId,
                                        const QString& relativePath) const {
    QString fullPath = getAbsolutePath(shareId, relativePath);
    return QFileInfo(fullPath);
}

void SharedFileManager::onIndexUpdateTimeout() {
    // 更新所有文件夹共享的索引
    QReadLocker locker(&mLock);
    for (const SharedItem& item : mSharedItems) {
        if (item.type == SharedItem::Folder && item.isActive) {
            updateFileIndex(item.id);
        }
    }
}

void SharedFileManager::updateFileIndex(const QString& shareId) {
    SharedItem share = getSharedItem(shareId);
    if (share.type != SharedItem::Folder) {
        return;
    }

    // 清除旧索引
    QSqlQuery query(mDatabase);
    query.prepare("DELETE FROM file_index WHERE share_id = ?");
    query.addBindValue(shareId);
    query.exec();

    // 递归建立新索引
    indexDirectory(shareId, share.path, "");
    emit indexUpdated(shareId);
}

void SharedFileManager::indexDirectory(const QString& shareId,
                                      const QString& basePath,
                                      const QString& relativePath) {
    QDir dir(QDir(basePath).absoluteFilePath(relativePath));

    for (const QFileInfo& fileInfo : dir.entryInfoList(
            QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {

        QString itemPath = relativePath.isEmpty() ?
            fileInfo.fileName() :
            relativePath + "/" + fileInfo.fileName();

        QSqlQuery query(mDatabase);
        query.prepare("INSERT INTO file_index (id, share_id, path, name, "
                     "type, size, modified_at) VALUES (?, ?, ?, ?, ?, ?, ?)");

        query.addBindValue(QUuid::createUuid().toString());
        query.addBindValue(shareId);
        query.addBindValue(itemPath);
        query.addBindValue(fileInfo.fileName());
        query.addBindValue(fileInfo.isDir() ? "folder" : "file");
        query.addBindValue(fileInfo.size());
        query.addBindValue(fileInfo.lastModified());
        query.exec();

        // 递归索引子文件夹
        if (fileInfo.isDir()) {
            indexDirectory(shareId, basePath, itemPath);
        }
    }
}

qint64 SharedFileManager::calculateDirectorySize(const QString& path) const {
    qint64 size = 0;
    QDir dir(path);

    for (const QFileInfo& fileInfo : dir.entryInfoList(
            QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (fileInfo.isDir()) {
            size += calculateDirectorySize(fileInfo.absoluteFilePath());
        } else {
            size += fileInfo.size();
        }
    }

    return size;
}

bool SharedFileManager::matchesIpPattern(const QHostAddress& address,
                                        const QString& pattern) const {
    if (pattern.isEmpty()) return false;

    // 简单的IP匹配实现，可以扩展为支持通配符
    return address.toString() == pattern;
}
