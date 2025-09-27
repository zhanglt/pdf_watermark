/**
 * @file GeometryUtils.h
 * @brief 几何计算工具模块头文件
 * 
 * 提供几何计算和文本处理的实用工具函数，包括：
 * - 文本宽度计算（使用Windows GDI）
 * - 几何图形的旋转变换计算
 * - SVG文件生成和处理
 * - 角度与弧度转换
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma once
#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include <QString>
#include <QFont>
#include <QFontMetrics>
#include <QSize>
#include <QSvgGenerator>
#include <QPainter>
#include <QColor>
#include <QFile>
#include "qmath.h"
#include <QDebug>
#include <cmath>
#include <iostream>
#include <vector>
#include "windows.h"



/**
 * @namespace GeometryUtils
 * @brief 几何计算工具命名空间
 */
namespace GeometryUtils {

/**
 * @struct Rectangle
 * @brief 矩形结构体，用于水印定位和旋转计算
 * 
 * 定义一个矩形区域，包含中心点坐标和尺寸信息
 * 主要用于水印文本的位置计算和旋转变换
 */
struct Rectangle {
    int x;       // 矩形中心点的X坐标（像素）
    int y;       // 矩形中心点的Y坐标（像素）
    int width;   // 矩形宽度（像素）
    int height;  // 矩形高度（像素）
};

// ================================
// 文本计算函数组
// ================================

/**
 * @brief 使用Windows GDI获取指定字体和大小的文本宽度
 * @param text 要测量的文本内容
 * @param fontName 字体名称
 * @param fontSize 字体大小
 * @return SIZE结构体，包含文本的宽度和高度信息
 * @note 用于水印文本的尺寸计算和布局，依赖Windows API
 */
SIZE getTextWidth(QString text, QString fontName, int fontSize);

// ================================
// 几何计算函数组
// ================================

/**
 * @brief 将角度转换为弧度
 * @param degrees 角度值
 * @return 对应的弧度值
 * @note 用于三角函数计算，转换公式：弧度 = 角度 × π / 180
 */
double toRadians(double degrees);

/**
 * @brief 计算矩形旋转后的坐标补偿值
 * @param rect 要旋转的矩形（包含中心点和尺寸信息）
 * @param angle 旋转角度（度）
 * @param offsetX 输出参数，X方向的补偿值
 * @param offsetY 输出参数，Y方向的补偿值
 * @note 用于确定旋转后的水印在页面上的准确位置
 */
void rotateRectangle(struct Rectangle& rect, double angle, double& offsetX,
                     double& offsetY);

// ================================
// SVG生成函数组
// ================================

/**
 * @brief 生成包含水印文本的SVG文件
 * @param svgName 输出SVG文件名
 * @param text 水印文本内容（支持多行）
 * @param pageWidth PDF页面宽度
 * @param pageHeight PDF页面高度
 * @param fontName 字体名称
 * @param fontSize 字体大小
 * @param color 文本颜色
 * @param angle 旋转角度
 * @param opacity 透明度（0.0-1.0）
 * @note SVG文件将被用作水印图章添加到PDF页面上，支持多行文本和各种样式设置
 */
void createSVG(QString svgName, QString text, int pageWidth, int pageHeight,
               QString fontName, int fontSize, QString color, qreal angle,
               qreal opacity);

} // namespace GeometryUtils

#endif // GEOMETRY_UTILS_H
