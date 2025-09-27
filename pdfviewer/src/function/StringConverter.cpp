/**
 * @file StringConverter.cpp
 * @brief 字符串转换模块实现
 * 
 * 该模块实现各种字符串格式之间的转换功能，确保在与第三方库
 * （如PDFlib）进行接口调用时能够正确处理字符编码。
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma execution_character_set("utf-8") // 设置编译器使用UTF-8字符集
#include "include/function/StringConverter.h"

namespace StringConverter {

/**
 * @brief 将std::string转换为std::wstring
 * 用于与PDFlib等需要宽字符的库进行接口调用
 * @param str 要转换的string
 * @return 转换后的wstring
 */
std::wstring String2WString(const string& str) {
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

/**
 * @brief 将QString转换为std::wstring
 * 用于Qt字符串与PDFlib宽字符接口的转换
 * @param str 要转换的QString
 * @return 转换后的wstring
 */
std::wstring QString2WString(const QString& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str.toStdString());
}

} // namespace StringConverter
