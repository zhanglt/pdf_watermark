/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file shareditem.h
 * @brief 共享项数据结构定义
 *
 * 本文件定义了 SharedItem 结构体，用于表示一个完整的共享项（文件或文件夹）。
 * SharedItem 包含了共享资源的所有信息，包括路径、配置、状态和统计数据。
 *
 * 在共享管理系统中，SharedItem 是核心数据结构：
 * - 存储在数据库中，实现持久化
 * - 缓存在内存中，提供快速访问
 * - 用于网络传输，向远程设备提供共享列表
 *
 * 数据库对应关系：
 * SharedItem 对象对应 shares 表的一行记录，通过 SharedFileManager 进行增删改查。
 */

#ifndef SHAREDITEM_H
#define SHAREDITEM_H

#include <QString>
#include <QDateTime>
#include "shareconfig.h"

/**
 * @struct SharedItem
 * @brief 共享项结构
 *
 * 表示一个被共享的文件或文件夹，包含完整的元数据和配置信息。
 * 这个结构体是 SharedFileManager 管理的核心对象。
 *
 * 生命周期：
 * 1. 用户添加共享 -> 创建 SharedItem 并写入数据库
 * 2. 程序启动 -> 从数据库加载到内存 Map 中
 * 3. 远程访问 -> 读取 SharedItem 验证权限
 * 4. 用户删除 -> 从内存和数据库中移除
 *
 * 关键字段说明：
 * - id：UUID 唯一标识符，用于在网络和数据库中引用共享
 * - path：实际的文件系统路径（绝对路径）
 * - name：对外展示的共享名称，可以与实际文件名不同
 * - type：区分文件和文件夹，影响传输和浏览行为
 * - size：总大小，文件夹需要递归计算
 * - config：共享配置（密码、权限、描述等）
 * - isActive：激活状态，可临时禁用共享而不删除
 */
struct SharedItem {
    /**
     * @enum Type
     * @brief 共享类型枚举
     */
    enum Type {
        File,       ///< 文件类型，可直接下载
        Folder      ///< 文件夹类型，可浏览和下载其中的文件
    };

    QString id;              ///< 唯一标识符（UUID格式）
    QString path;            ///< 本地文件系统的绝对路径
    QString name;            ///< 共享的显示名称
    Type type;               ///< 共享类型（文件或文件夹）
    qint64 size;             ///< 大小（字节），文件夹为递归计算的总大小
    QString permissions;     ///< 权限字符串："r"=只读，"rw"=读写
    ShareConfig config;      ///< 共享配置信息（密码、描述等）
    QDateTime createdAt;     ///< 共享创建时间
    QDateTime modifiedAt;    ///< 共享最后修改时间
    int accessCount;         ///< 访问次数统计（下载次数）
    bool isActive;           ///< 是否激活（false时不对外提供）

    /**
     * @brief 默认构造函数
     *
     * 初始化为默认值：
     * - type = File（文件类型）
     * - size = 0（未知大小）
     * - permissions = "r"（只读权限）
     * - accessCount = 0（未被访问）
     * - isActive = true（默认激活）
     */
    SharedItem() : type(File), size(0), permissions("r"), accessCount(0), isActive(true) {}
};

#endif // SHAREDITEM_H