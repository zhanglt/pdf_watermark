/**
 * @file FileDetector.h
 * @brief 文件类型检测模块头文件
 * 
 * 提供各种文件类型的检测功能，包括：
 * - PDF文件检测（通过扩展名和文件头魔数）
 * - 图片文件检测（通过扩展名和文件头魔数）
 * - 数字字符串验证
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma once
#ifndef FILE_DETECTOR_H
#define FILE_DETECTOR_H

#include <QString>
#include <QFile>
#include <QRegularExpression>

/**
 * @namespace FileDetector
 * @brief 文件检测功能命名空间
 */
namespace FileDetector {

// ================================
// PDF文件检测函数
// ================================

/**
 * @brief 通过文件扩展名判断是否为PDF文件
 * @param fileName 文件名（包含扩展名）
 * @return true: 是PDF文件, false: 不是PDF文件
 * @note 不区分大小写，检查文件是否以.pdf结尾
 */
bool isPDFByExtension(const QString& fileName);

/**
 * @brief 通过文件头魔数判断是否为PDF文件
 * @param filePath 文件完整路径
 * @return true: 是PDF文件, false: 不是PDF文件
 * @note 通过读取文件前4个字节检查是否为"%PDF"标识
 */
bool isPDFByMagicNumber(const QString& filePath);

// ================================
// 图片文件检测函数
// ================================

/**
 * @brief 通过文件扩展名判断是否为图片文件
 * @param fileName 文件名（包含扩展名）
 * @return true: 是支持的图片格式, false: 不是图片文件
 * @note 支持的格式：jpg, jpeg, png, bmp（不区分大小写）
 */
bool isImageByExtension(const QString& fileName);

/**
 * @brief 通过文件头魔数判断是否为图片文件
 * @param filePath 文件完整路径
 * @return true: 是支持的图片格式, false: 不是图片文件
 * @note 检查JPEG、PNG、BMP格式的文件头标识
 */
bool isImageByMagicNumber(const QString& filePath);

// ================================
// 字符串验证函数
// ================================

/**
 * @brief 判断字符串是否为纯数字
 * @param str 要检查的字符串
 * @return true: 是纯数字, false: 包含非数字字符
 * @note 使用正则表达式检查整个字符串是否都是数字字符
 */
bool isNumeric(const QString& str);

} // namespace FileDetector

#endif // FILE_DETECTOR_H