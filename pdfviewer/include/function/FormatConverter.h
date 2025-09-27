/**
 * @file FormatConverter.h
 * @brief 格式转换模块头文件
 * 
 * 提供PDF与图片格式之间的相互转换功能，包括：
 * - PDF转图片（使用MuPDF库）
 * - 图片转PDF（单个或批量）
 * - 支持多种图片格式和自定义分辨率
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma once
#ifndef FORMAT_CONVERTER_H
#define FORMAT_CONVERTER_H

#include <string>
#include <QStringList>
#include <QDebug>
#include <iostream>
#include "mupdf/fitz.h"
#include "lib/pdflib.hpp"
#include "include/function/StringConverter.h"

using namespace std;
using namespace pdflib;

/**
 * @namespace FormatConverter
 * @brief 格式转换功能命名空间
 */
namespace FormatConverter {

// ================================
// PDF转图片函数组
// ================================

/**
 * @brief 将PDF文件转换为图片文件
 * @param pdfFile PDF文件路径
 * @param imagePath 图片输出目录路径
 * @param resolution 输出图片的分辨率（DPI），默认72
 * @return 转换的页面数量，出错时返回1
 * @note 使用MuPDF库，将PDF的每一页转换为PNG格式图片，文件名格式为：0.png, 1.png, ...
 */
int pdf2image(string pdfFile, string imagePath, int resolution = 72);

// ================================
// 图片转PDF函数组
// ================================

/**
 * @brief 将单个图片文件转换为PDF文件
 * @param imageFile 输入图片文件路径
 * @param pdfFile 输出PDF文件路径
 * @return 0: 成功, 2: 失败
 * @note 使用PDFlib库，创建包含一页图片的PDF文档，页面大小自适应图片尺寸
 */
int image2pdf(std::string imageFile, std::string pdfFile);

/**
 * @brief 将图片列表合并到一个PDF文件
 * @param images 图片文件路径列表
 * @param pdfFile 输出PDF文件路径
 * @return 0: 成功, 2: 失败
 * @note 每个图片占一页，页面大小自适应图片尺寸，处理后会删除原图片文件
 */
int images2pdf(QStringList& images, std::string pdfFile);

/**
 * @brief 将指定目录中的图片文件合并为PDF
 * @param imagesDir 图片目录路径
 * @param pdfFile 输出PDF文件路径
 * @param num 图片文件数量
 * @return 0: 成功, 2: 失败
 * @note 特殊函数，图片文件名必须为0.png, 1.png, ..., (num-1).png格式，
 *       处理后会删除原图片文件
 */
int images2pdf(std::string imagesDir, std::string pdfFile, int num);

} // namespace FormatConverter

#endif // FORMAT_CONVERTER_H
