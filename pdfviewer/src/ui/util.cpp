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
 * @file util.cpp
 * @brief 工具函数类实现
 *
 * 本文件实现了 Util 类的所有静态方法，
 * 提供文件大小格式化、目录遍历、版本解析等常用功能。
 */

#include <QFileInfo>

#include "util.h"
#include "settings.h"

/**
 * @brief 将字节大小转换为可读的字符串格式
 * @param size 文件大小（字节）
 * @return 格式化的大小字符串
 *
 * 转换逻辑：
 * 1. 从字节开始，每除以 1024 增加一个单位级别
 * 2. 根据除以 1024 的次数选择对应的单位
 * 3. 保留两位小数，拼接单位后缀
 */
QString Util::sizeToString(qint64 size)
{
    int count = 0;          // 记录除以 1024 的次数
    double f_size = size;   // 转换为浮点数以支持小数

    // 循环除以 1024，直到小于 1024
    // count 值决定了使用哪个单位
    while (f_size >= 1024) {
        f_size /= 1024;
        count++;
    }

    // 根据 count 值选择对应的单位后缀
    QString suffix;
    switch (count) {
    case 0 : suffix = " B"; break;   // 字节
    case 1 : suffix = " KB"; break;  // 千字节
    case 2 : suffix = " MB"; break;  // 兆字节
    case 3 : suffix = " GB"; break;  // 吉字节
    case 4 : suffix = " TB"; break;  // 太字节
    }

    // 格式化为两位小数，并拼接单位后缀
    return QString::number(f_size, 'f', 2).append(suffix);
}

/**
 * @brief 递归获取目录中所有文件的相对路径和绝对路径
 * @param startingDir 起始目录
 * @param innerDirName 当前的相对路径前缀
 * @return QPair 向量，包含所有文件的相对路径和绝对路径
 *
 * 递归遍历算法：
 * 1. 先获取当前目录中的所有文件，添加到结果中
 * 2. 再获取当前目录中的所有子目录
 * 3. 对每个子目录递归调用此函数
 * 4. 合并所有子目录返回的结果
 *
 * 相对路径的计算：
 * - 起始目录的文件，相对路径为空字符串
 * - 子目录的文件，相对路径为 "子目录名" 或 "父目录名/子目录名"
 */
QVector< QPair<QString, QString> >
    Util::getInnerDirNameAndFullFilePath(const QDir& startingDir, const QString& innerDirName)
{
    QVector< QPair<QString, QString> > pairs;

    // 第一步：获取当前目录中的所有文件（不包括 . 和 ..）
    QFileInfoList fiList = startingDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    for (const auto& fi : fiList) {
        // 添加 QPair：(相对路径, 绝对路径)
        pairs.push_back( QPair<QString, QString>(innerDirName, fi.filePath()) );
    }

    // 第二步：获取当前目录中的所有子目录
    fiList = startingDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const auto& fi : fiList) {
        // 计算新的相对路径前缀
        QString newInnerDirName;
        if (innerDirName.isEmpty())
            // 如果当前没有前缀，直接使用子目录名
            newInnerDirName = fi.fileName();
        else
            // 如果已有前缀，拼接路径：前缀/子目录名
            newInnerDirName = innerDirName + QDir::separator() + fi.fileName();

        // 递归调用，获取子目录中的所有文件
        QVector< QPair<QString, QString> > otherPairs =
                getInnerDirNameAndFullFilePath( QDir(fi.filePath()), newInnerDirName );

        // 合并子目录的结果到当前结果中
        pairs.append(otherPairs);
    }

    return pairs;
}

/**
 * @brief 解析应用程序版本号
 * @param onlyVerNum true 表示只返回版本号，false 表示包含操作系统信息
 * @return 版本字符串
 *
 * 版本号格式：X.Y.Z
 * - X: 主版本号（PROGRAM_X_VER）
 * - Y: 次版本号（PROGRAM_Y_VER）
 * - Z: 修订版本号（PROGRAM_Z_VER）
 *
 * 带操作系统信息的格式：v X.Y.Z (操作系统名称)
 */
QString Util::parseAppVersion(bool onlyVerNum)
{
    if (onlyVerNum) {
        // 只返回版本号，格式：X.Y.Z
        return QString::number(PROGRAM_X_VER) + "." +
               QString::number(PROGRAM_Y_VER) + "." +
               QString::number(PROGRAM_Z_VER);
    }

    // 返回完整版本信息，格式：v X.Y.Z (操作系统)
    return "v " + QString::number(PROGRAM_X_VER) + "." +
           QString::number(PROGRAM_Y_VER) + "." +
           QString::number(PROGRAM_Z_VER) +
           " (" + QString(OS_NAME) + ")";
}

/**
 * @brief 生成唯一的文件名，避免覆盖已存在的文件
 * @param fileName 原始文件名
 * @param folderPath 目标文件夹路径
 * @return 唯一的文件完整路径
 *
 * 算法流程：
 * 1. 首先尝试使用原始文件名
 * 2. 如果文件已存在，在文件名后添加 " (1)"
 * 3. 如果 " (1)" 也存在，尝试 " (2)"，依此类推
 * 4. 直到找到一个不存在的文件名
 *
 * 示例：
 * - document.txt -> document (1).txt -> document (2).txt -> ...
 *
 * 注意：编号添加在文件名和扩展名之间，而不是扩展名之后
 */
QString Util::getUniqueFileName(const QString& fileName, const QString& folderPath)
{
    int count = 1;  // 编号计数器，从 1 开始

    // 构造原始的完整文件路径
    QString originalFilePath = folderPath + QDir::separator() + fileName;
    QString fPath = originalFilePath;

    // 循环检查文件是否存在，直到找到不存在的文件名
    while (QFile::exists(fPath)) {
        QFileInfo fInfo(originalFilePath);

        // 构造新的文件名：基础名 + " (编号)" + 扩展名
        // baseName()：不含扩展名的文件名
        // completeSuffix()：完整的扩展名（包括多重扩展名，如 .tar.gz）
        QString baseName = fInfo.baseName() + " (" + QString::number(count) + ")";
        fPath = folderPath + QDir::separator() + baseName + "." + fInfo.completeSuffix();

        count++;  // 递增编号
    }

    // 返回找到的唯一文件名的完整路径
    return fPath;
}
