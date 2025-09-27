/**
 * @file StringConverter.h
 * @brief 字符串转换模块头文件
 * 
 * 提供各种字符串格式之间的转换功能，主要用于：
 * - std::string 与 std::wstring 之间的转换
 * - QString 与 std::wstring 之间的转换
 * - 支持UTF-8编码的正确处理
 * 
 * 这些转换功能主要用于与PDFlib等需要宽字符的第三方库进行接口调用
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma once
#ifndef STRING_CONVERTER_H
#define STRING_CONVERTER_H

#include <string>
#include <QString>
#include <codecvt>
#include <locale>

using namespace std;

/**
 * @namespace StringConverter
 * @brief 字符串转换功能命名空间
 */
namespace StringConverter {

// ================================
// 字符串转换函数组
// ================================

/**
 * @brief 将std::string转换为std::wstring
 * @param str 要转换的标准字符串
 * @return 转换后的宽字符串
 * @note 使用UTF-8编码进行转换，用于与PDFlib等需要宽字符的库进行接口调用
 */
std::wstring String2WString(const string& str);

/**
 * @brief 将QString转换为std::wstring
 * @param str 要转换的Qt字符串
 * @return 转换后的宽字符串
 * @note 用于Qt字符串与PDFlib宽字符接口的转换，确保Unicode字符的正确处理
 */
std::wstring QString2WString(const QString& str);

} // namespace StringConverter

#endif // STRING_CONVERTER_H