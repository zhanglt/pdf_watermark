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
 * @file util.h
 * @brief 工具函数类头文件
 *
 * 本文件定义了 Util 工具类，提供一组静态辅助函数：
 * - 文件大小格式化显示
 * - 目录递归遍历
 * - 版本号解析
 * - 文件名唯一化处理
 *
 * 这些函数在整个应用程序中被广泛使用。
 */

#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QDir>
#include <QVector>
#include <QPair>

/**
 * @class Util
 * @brief 工具函数类
 *
 * Util 类提供一组静态辅助函数，用于处理常见的文件和字符串操作。
 * 所有方法都是静态的，不需要创建实例即可使用。
 *
 * 主要功能：
 * - 文件大小格式化（字节转换为KB/MB/GB等）
 * - 递归获取目录中的所有文件及其相对路径
 * - 解析应用程序版本号
 * - 生成唯一的文件名（避免覆盖已存在文件）
 */
class Util
{
public:
    /**
     * @brief 将字节大小转换为可读的字符串格式
     * @param size 文件大小（字节）
     * @return 格式化的大小字符串，如 "1.50 MB"、"256.00 KB"
     *
     * 自动选择合适的单位（B、KB、MB、GB、TB），
     * 保留两位小数，使文件大小更易读
     *
     * 示例：
     * - sizeToString(1024) -> "1.00 KB"
     * - sizeToString(1536000) -> "1.46 MB"
     */
    static QString sizeToString(qint64 size);

    /**
     * @brief 递归获取目录中所有文件的相对路径和绝对路径
     * @param startingDir 起始目录
     * @param innerDirName 内部目录的相对路径（初始调用时通常为空字符串）
     * @return QPair 向量，每个元素包含：
     *         - first: 文件的相对路径（相对于起始目录）
     *         - second: 文件的完整绝对路径
     *
     * 此函数递归遍历目录树，收集所有文件的路径信息。
     * 用于文件夹传输时，需要保持目录结构。
     *
     * 示例：
     * 假设目录结构为：
     *   /home/user/folder/
     *   ├── file1.txt
     *   └── subdir/
     *       └── file2.txt
     *
     * 调用 getInnerDirNameAndFullFilePath(QDir("/home/user/folder"), "")
     * 返回：
     *   [("", "/home/user/folder/file1.txt"),
     *    ("subdir", "/home/user/folder/subdir/file2.txt")]
     */
    static QVector< QPair<QString, QString> >
        getInnerDirNameAndFullFilePath(const QDir& startingDir, const QString& innerDirName);

    /**
     * @brief 解析应用程序版本号
     * @param onlyVerNum true 表示只返回版本号，false 表示包含操作系统信息
     * @return 版本字符串
     *
     * 根据 settings.h 中定义的版本号常量生成版本字符串。
     *
     * 示例：
     * - parseAppVersion(true) -> "1.2.1"
     * - parseAppVersion(false) -> "v 1.2.1 (Windows)"
     */
    static QString parseAppVersion(bool onlyVerNum = true);

    /**
     * @brief 生成唯一的文件名，避免覆盖已存在的文件
     * @param fileName 原始文件名
     * @param folderPath 目标文件夹路径
     * @return 唯一的文件完整路径
     *
     * 如果目标路径已存在同名文件，自动在文件名后添加编号。
     *
     * 示例：
     * - 如果 "document.txt" 已存在，返回 "document (1).txt"
     * - 如果 "document (1).txt" 也存在，返回 "document (2).txt"
     * - 依此类推，直到找到不存在的文件名
     */
    static QString getUniqueFileName(const QString& fileName, const QString& folderPath);
};

#endif // UTIL_H
