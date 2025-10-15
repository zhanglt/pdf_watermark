/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file shareconfig.h
 * @brief 共享配置结构定义
 *
 * 本文件定义了 ShareConfig 结构体，用于存储文件或文件夹的共享配置信息。
 * 这些配置信息控制共享资源的访问权限、密码保护、有效期等特性。
 *
 * ShareConfig 在以下场景使用：
 * - 创建新共享：用户设置共享名称、描述、密码和权限
 * - 更新共享：修改现有共享的配置
 * - 访问控制：验证远程设备的访问权限
 */

#ifndef SHARECONFIG_H
#define SHARECONFIG_H

#include <QString>

/**
 * @struct ShareConfig
 * @brief 共享配置结构
 *
 * 封装了创建和管理共享所需的所有配置参数。
 * 这个轻量级结构体用于在界面和后端之间传递共享设置。
 *
 * 配置项说明：
 * - name：共享的显示名称，可以不同于实际的文件/文件夹名
 * - description：共享的描述信息，帮助接收方了解共享内容
 * - password：访问密码，为空表示无密码保护
 * - permissions：访问权限，"r"表示只读，"rw"表示读写（目前仅支持只读）
 * - expirationHours：有效期（小时），0表示永久有效
 */
struct ShareConfig {
    QString name;            ///< 共享的自定义名称（用于显示）
    QString description;     ///< 共享的描述信息
    QString password;        ///< 访问密码（为空表示无密码）
    QString permissions;     ///< 权限设置："r"=只读，"rw"=读写
    int expirationHours;     ///< 有效期（小时数），0=永久有效

    /**
     * @brief 默认构造函数
     *
     * 初始化为默认配置：
     * - permissions = "r"（只读权限）
     * - expirationHours = 0（永久有效）
     */
    ShareConfig() : permissions("r"), expirationHours(0) {}
};

#endif // SHARECONFIG_H