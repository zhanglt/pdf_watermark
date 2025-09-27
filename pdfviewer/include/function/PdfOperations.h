/**
 * @file PdfOperations.h
 * @brief PDF文档操作模块头文件
 * 
 * 提供PDF文档的基础操作功能，包括：
 * - PDF文档信息获取（页面数、页面尺寸）
 * - PDF文档拆分（按页数或页面范围）
 * - PDF文档合并
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma once
#ifndef PDF_OPERATIONS_H
#define PDF_OPERATIONS_H

#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include <QDebug>
#include "mupdf/fitz.h"
#include "lib/pdflib.hpp"
#include "include/function/StringConverter.h"

using namespace std;
using namespace pdflib;

/**
 * @namespace PdfOperations
 * @brief PDF操作功能命名空间
 */
namespace PdfOperations {

// ================================
// PDF信息获取函数组
// ================================

/**
 * @brief 获取PDF文件的页面数量
 * @param pdfFile PDF文件路径
 * @return PDF文件的页面数，出错时返回1
 * @note 使用MuPDF库解析PDF文件并返回页面数
 */
int getPages(string pdfFile);

/**
 * @brief 获取PDF文件页面高度
 * @param filename PDF文件路径（宽字符串）
 * @return 页面高度（点为单位），出错时返回0
 * @note 获取PDF第一页的高度信息，使用PDFlib库
 */
int getPageheight(wstring filename);

// ================================
// PDF拆分函数组
// ================================

/**
 * @brief 将PDF拆分为多个文件（按页数分割）
 * @param in 输入PDF文件路径
 * @param out 输出文件名前缀
 * @param subpages 每个拆分文件的页数
 * @return 0: 成功, 2: 失败
 * @note 生成的文件名格式为：out_1.pdf, out_2.pdf, ...
 */
int splitPdf(string in, string out, int subpages);

/**
 * @brief 从PDF中提取指定页面范围为新文件
 * @param in 输入PDF文件路径
 * @param out 输出PDF文件路径前缀
 * @param start 截取的起始页（从0开始）
 * @param end 截取的结束页（不包含此页）
 * @return 0: 成功, 2: 失败
 * @note 生成的文件名格式为：out_split.pdf
 */
int splitPdf(string in, string out, int start, int end);

// ================================
// PDF合并函数组
// ================================

/**
 * @brief 将多个PDF文件合并为一个文件
 * @param fileList 要合并的PDF文件路径列表
 * @param outFile 输出PDF文件路径
 * @return 0: 成功, 2: 失败
 * @note 会为每个文件的第一页创建书签，使用PDFlib库进行合并
 */
int mergePdf(std::list<string> fileList, string outFile);

} // namespace PdfOperations

#endif // PDF_OPERATIONS_H
