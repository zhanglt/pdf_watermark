/**
 * @file WatermarkProcessor.h
 * @brief 水印处理模块头文件
 * 
 * 提供PDF文档水印处理功能，包括：
 * - 多行文本水印添加
 * - SVG水印文件生成和尺寸获取
 * - 支持自定义字体、颜色、旋转角度和透明度
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma once
#ifndef WATERMARK_PROCESSOR_H
#define WATERMARK_PROCESSOR_H

#include <string>
#include <QString>
#include <QDebug>
#include <iostream>
#include <sstream>
#include "lib/pdflib.hpp"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "include/function/StringConverter.h"

using namespace std;
using namespace pdflib;

/**
 * @namespace WatermarkProcessor
 * @brief 水印处理功能命名空间
 */
namespace WatermarkProcessor {

// ================================
// 水印处理主函数
// ================================

/**
 * @brief 为PDF文件添加多行文本水印
 * @param inFile 输入PDF文件路径
 * @param outFile 输出PDF文件路径
 * @param mark_txt 水印文本内容（支持\n换行符分隔的多行文本）
 * @param fontName 水印字体名称
 * @param fontSize 水印字体大小
 * @param color 水印颜色
 * @param angle 水印旋转角度（度）
 * @param opacity 水印透明度（0.0-1.0）
 * @return 0: 成功, 其他值: 失败
 * @note 支持自定义字体、颜色、旋转角度和透明度，使用SVG生成水印图形
 */
int addWatermark_multiline(string inFile, string outFile, QString mark_txt,
                           QString fontName, int fontSize, QString color,
                           qreal angle, qreal opacity);

// ================================
// SVG处理辅助函数
// ================================

/**
 * @brief 获取SVG文件的尺寸信息
 * @param filename SVG文件路径
 * @param width 输出参数，SVG宽度
 * @param height 输出参数，SVG高度
 * @note 解析SVG文件的width和height属性，使用libxml2库
 */
void getSVGDimensions(const char* filename, int& width, int& height);

} // namespace WatermarkProcessor

#endif // WATERMARK_PROCESSOR_H
