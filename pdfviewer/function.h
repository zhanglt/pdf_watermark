/**
 * @file function.h
 * @brief PDF工具函数库聚合头文件
 * 
 * 本文件是所有PDF处理功能模块的聚合头文件，通过包含各个专门模块的头文件，
 * 提供统一的函数接口。各功能模块包括：
 * - 文件类型检测模块 (FileDetector)
 * - 字符串转换模块 (StringConverter) 
 * - 文件系统操作模块 (FileSystemUtils)
 * - 格式转换模块 (FormatConverter)
 * - PDF操作模块 (PdfOperations)
 * - 水印处理模块 (WatermarkProcessor)
 * - 几何计算模块 (GeometryUtils)
 * 
 * 使用此头文件可以访问所有PDF处理功能，各模块已按功能职责进行了合理拆分。
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma once
#ifndef FUNCTION_H
#define FUNCTION_H

// ================================
// 包含各功能模块头文件
// ================================

#include "include/function/FileDetector.h"       // 文件类型检测功能
#include "include/function/StringConverter.h"    // 字符串转换功能
#include "include/function/FileSystemUtils.h"    // 文件系统操作功能
#include "include/function/FormatConverter.h"    // 格式转换功能
#include "include/function/PdfOperations.h"      // PDF操作功能
#include "include/function/WatermarkProcessor.h" // 水印处理功能
#include "include/function/GeometryUtils.h"      // 几何计算功能

// ================================
// 为了保持向后兼容性，提供全局函数别名
// ================================

// 文件检测函数（映射到FileDetector命名空间）
inline bool isPDFByExtension(const QString& fileName) {
    return FileDetector::isPDFByExtension(fileName);
}

inline bool isPDFByMagicNumber(const QString& filePath) {
    return FileDetector::isPDFByMagicNumber(filePath);
}

inline bool isImageByExtension(const QString& fileName) {
    return FileDetector::isImageByExtension(fileName);
}

inline bool isImageByMagicNumber(const QString& filePath) {
    return FileDetector::isImageByMagicNumber(filePath);
}

inline bool isNumeric(const QString& str) {
    return FileDetector::isNumeric(str);
}

// 字符串转换函数（映射到StringConverter命名空间）
inline std::wstring String2WString(const string& str) {
    return StringConverter::String2WString(str);
}

inline std::wstring QString2WString(const QString& str) {
    return StringConverter::QString2WString(str);
}

// 文件系统操作函数（映射到FileSystemUtils命名空间）
inline void traverseDirectory(const QDir& dir, QStringList& resultList,
                               QString fileType, QString exclude) {
    FileSystemUtils::traverseDirectory(dir, resultList, fileType, exclude);
}

inline void tDirectory(const QDir& dir, QStringList& resultList,
                       const QVariantList& fileTypes) {
    FileSystemUtils::tDirectory(dir, resultList, fileTypes);
}

inline QString pathChange(QString rootInput, QString rootOutput, QString file,
                          QString insert) {
    return FileSystemUtils::pathChange(rootInput, rootOutput, file, insert);
}

inline bool createDirectory(const QString& path) {
    return FileSystemUtils::createDirectory(path);
}

inline string extractFileName(const std::string& fullPath) {
    return FileSystemUtils::extractFileName(fullPath);
}

inline string extractDirectory(const std::string& fullPath) {
    return FileSystemUtils::extractDirectory(fullPath);
}

// 格式转换函数（映射到FormatConverter命名空间）
inline int pdf2image(string pdfFile, string imagePath, int resolution = 72) {
    return FormatConverter::pdf2image(pdfFile, imagePath, resolution);
}

inline int image2pdf(std::string imageFile, std::string pdfFile) {
    return FormatConverter::image2pdf(imageFile, pdfFile);
}

inline int images2pdf(QStringList& images, std::string pdfFile) {
    return FormatConverter::images2pdf(images, pdfFile);
}

inline int images2pdf(std::string imagesDir, std::string pdfFile, int num) {
    return FormatConverter::images2pdf(imagesDir, pdfFile, num);
}

// PDF操作函数（映射到PdfOperations命名空间）
inline int getPages(string pdfFile) {
    return PdfOperations::getPages(pdfFile);
}

inline int getPageheight(wstring filename) {
    return PdfOperations::getPageheight(filename);
}

inline int splitPdf(string in, string out, int subpages) {
    return PdfOperations::splitPdf(in, out, subpages);
}

inline int splitPdf(string in, string out, int start, int end) {
    return PdfOperations::splitPdf(in, out, start, end);
}

inline int mergePdf(std::list<string> fileList, string outFile) {
    return PdfOperations::mergePdf(fileList, outFile);
}

// 水印处理函数（映射到WatermarkProcessor命名空间）
inline int addWatermark_multiline(string in, string out, QString mark_txt,
                                   QString fontName, int fontSize, QString color,
                                   qreal angle, qreal opacity) {
    return WatermarkProcessor::addWatermark_multiline(in, out, mark_txt, fontName, 
                                                      fontSize, color, angle, opacity);
}

inline void getSVGDimensions(const char* filename, int& width, int& height) {
    WatermarkProcessor::getSVGDimensions(filename, width, height);
}

// 几何计算函数（映射到GeometryUtils命名空间）
inline SIZE getTextWidth(QString text, QString fontName, int fontSize) {
    return GeometryUtils::getTextWidth(text, fontName, fontSize);
}

inline double toRadians(double degrees) {
    return GeometryUtils::toRadians(degrees);
}

inline void rotateRectangle(GeometryUtils::Rectangle& rect, double angle, 
                            double& offsetX, double& offsetY) {
    GeometryUtils::rotateRectangle(rect, angle, offsetX, offsetY);
}

inline void createSVG(QString svgName, QString text, int pageWidth, int pageHeight,
                      QString fontName, int fontSize, QString color, qreal angle,
                      qreal opacity) {
    GeometryUtils::createSVG(svgName, text, pageWidth, pageHeight, fontName, 
                            fontSize, color, angle, opacity);
}

// 为了保持兼容性，提供Rectangle结构体的全局别名
//using Rectangle = GeometryUtils::Rectangle;

#endif  // FUNCTION_H
