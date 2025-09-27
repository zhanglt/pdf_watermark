/**
 * @file FileSystemUtils.h
 * @brief 文件系统操作工具模块头文件
 * 
 * 提供文件和目录操作的实用工具函数，包括：
 * - 目录递归遍历和文件收集
 * - 路径字符串处理和变换
 * - 目录创建和文件名提取
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma once
#ifndef FILE_SYSTEM_UTILS_H
#define FILE_SYSTEM_UTILS_H

#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <QVariantList>
#include <QDebug>
#include <string>

using namespace std;

/**
 * @namespace FileSystemUtils
 * @brief 文件系统操作工具命名空间
 */
namespace FileSystemUtils {

// ================================
// 目录遍历函数组
// ================================

/**
 * @brief 递归遍历目录，收集指定类型的文件
 * @param dir 要遍历的根目录
 * @param resultList 输出参数，存放符合条件的文件路径列表
 * @param fileType 要查找的文件类型（扩展名，如"pdf"）
 * @param exclude 排除包含此关键字的文件
 * @note 会递归遍历所有子目录，排除包含"_pdf_"标记的文件
 */
void traverseDirectory(const QDir& dir, QStringList& resultList,
                       QString fileType, QString exclude);

/**
 * @brief 遍历目录，收集多种文件类型
 * @param dir 要遍历的目录
 * @param resultList 输出参数，存放文件路径列表
 * @param fileTypes 要收集的文件类型列表（扩展名）
 * @note 支持同时收集多种文件类型，递归遍历子目录
 */
void tDirectory(const QDir& dir, QStringList& resultList,
                const QVariantList& fileTypes);

// ================================
// 路径处理函数组
// ================================

/**
 * @brief 根据输入路径生成新的输出路径
 * @param rootInput 原始根目录路径
 * @param rootOutput 新的输出根目录路径
 * @param file 完整的文件路径
 * @param insert 要插入的标识字符串（如"_watermarked"）
 * @return 生成的新文件路径
 * @note 用于批处理时生成对应的输出文件路径，会自动创建必要的目录
 */
QString pathChange(QString rootInput, QString rootOutput, QString file,
                   QString insert);

/**
 * @brief 从完整文件路径中提取文件名部分
 * @param fullPath 完整的文件路径
 * @return 文件名部分（不包含路径）
 * @note 支持Windows和Unix风格的路径分隔符
 */
string extractFileName(const std::string& fullPath);

/**
 * @brief 从完整文件路径中提取目录路径部分
 * @param fullPath 完整的文件路径
 * @return 目录路径部分（不包含文件名）
 */
string extractDirectory(const std::string& fullPath);

// ================================
// 目录管理函数组
// ================================

/**
 * @brief 创建目录（包括中间目录）
 * @param path 要创建的目录路径
 * @return true: 创建成功或目录已存在, false: 创建失败
 * @note 会递归创建所有必要的父目录
 */
bool createDirectory(const QString& path);

} // namespace FileSystemUtils

#endif // FILE_SYSTEM_UTILS_H