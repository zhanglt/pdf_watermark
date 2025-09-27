/**
 * @file GeometryUtils.cpp
 * @brief 几何计算工具模块实现
 * 
 * 该模块实现了几何计算和文本处理的各种工具函数，
 * 为水印处理和PDF图形操作提供基础的数学计算支持。
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma execution_character_set("utf-8") // 设置编译器使用UTF-8字符集
#include "include/function/GeometryUtils.h"

// 链接Windows GDI库，用于文本宽度计算
#pragma comment(lib, "Gdi32.lib")

namespace GeometryUtils {

/**
 * @brief 使用Windows GDI获取指定字体和大小的文本宽度
 * 用于水印文本的尺寸计算和布局
 * @param text 要测量的文本内容
 * @param fontName 字体名称
 * @param fontSize 字体大小
 * @return SIZE结构体，包含文本的宽度和高度信息
 */
SIZE getTextWidth(QString text, QString fontName, int fontSize) {
    // 创建兼容的设备上下文
    HDC hdc = CreateCompatibleDC(NULL);
    if (hdc == NULL) {
        qDebug() << "无法创建设备上下文！" << endl;
    }
    
    // 将QString转换为wchar_t数组
    std::vector<wchar_t> wcharArray(fontName.length() + 1);
    fontName.toWCharArray(wcharArray.data());

    // 获取wchar_t数组指针
    wchar_t* wcharPtr_fontname = wcharArray.data();
    
    // 选择字体
    HFONT hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                           DEFAULT_PITCH | FF_DONTCARE, wcharPtr_fontname);
    if (hFont == NULL) {
        qDebug() << "无法创建字体！" << endl;
        DeleteDC(hdc);
    }
    
    // 选择字体到设备上下文
    SelectObject(hdc, hFont);
    
    // 将QString转换为std::wstring
    std::wstring wtext = text.toStdWString();
    // 获取LPCWSTR指针
    LPCWSTR lpcwstr = wtext.c_str();

    SIZE size;
    if (!GetTextExtentPoint32(hdc, lpcwstr, wcslen(lpcwstr), &size)) {
        qDebug() << "无法获取文本尺寸！" << endl;
        DeleteObject(hFont);
        DeleteDC(hdc);
    }
    
    // 释放资源
    DeleteObject(hFont);
    DeleteDC(hdc);
    return size;
}

/**
 * @brief 将角度转换为弧度
 * @param degrees 角度值
 * @return 对应的弧度值
 */
double toRadians(double degrees) { 
    return degrees * M_PI / 180.0; 
}

/**
 * @brief 计算矩形旋转后的坐标补偿值
 * 用于确定旋转后的水印在页面上的准确位置
 * @param rect 要旋转的矩形（包含中心点和尺寸信息）
 * @param angle 旋转角度（度）
 * @param offsetX 输出参数：X方向的补偿值
 * @param offsetY 输出参数：Y方向的补偿值
 */
void rotateRectangle(struct Rectangle& rect, double angle, double& offsetX,
                     double& offsetY) {
    double centerX = rect.x;        // 矩形中心点X坐标
    double centerY = rect.y;        // 矩形中心点Y坐标

    double radians = toRadians(angle);  // 转换为弧度
    
    // 预计算三角函数值
    double cosAngle = cos(radians);
    double sinAngle = sin(radians);
    double halfWidth = rect.width / 2.0;   // 矩形半宽
    double halfHeight = rect.height / 2.0; // 矩形半高
    
    // 计算旋转后的顶点坐标
    // double rotatedX1 = centerX + halfWidth * cosAngle - halfHeight * sinAngle;
    // double rotatedY1 = centerY + halfWidth * sinAngle + halfHeight * cosAngle;
    double rotatedX2 = centerX - halfWidth * cosAngle - halfHeight * sinAngle;
    double rotatedY2 = centerY - halfWidth * sinAngle + halfHeight * cosAngle;
    double rotatedX3 = centerX - halfWidth * cosAngle + halfHeight * sinAngle;
    double rotatedY3 = centerY - halfWidth * sinAngle - halfHeight * cosAngle;
    // double rotatedX4 = centerX + halfWidth * cosAngle + halfHeight * sinAngle;
    // double rotatedY4 = centerY + halfWidth * sinAngle - halfHeight * cosAngle;

    // 计算坐标补偿值
    offsetX = rotatedX3 - rotatedX2;
    offsetY = rotatedY2 - rotatedY3;
}

/**
 * @brief 生成包含水印文本的SVG文件
 * SVG文件将被用作水印图章添加到PDF页面上
 * @param svgName 输出SVG文件名
 * @param text 水印文本内容（支持多行）
 * @param pageWidth PDF页面宽度
 * @param pageHeight PDF页面高度
 * @param fontName 字体名称
 * @param fontSize 字体大小
 * @param color 文本颜色
 * @param angle 旋转角度
 * @param opacity 透明度（0.0-1.0）
 */
void createSVG(QString svgName, QString text, int pageWidth, int pageHeight,
               QString fontName, int fontSize, QString color, qreal angle,
               qreal opacity) {
    // 初始化字体和文本处理
    QFont font(fontName, fontSize);
    QStringList textLines = text.split("\n");  // 按换行符分割文本
    
    // 使用QFontMetrics计算文本尺寸
    QFontMetrics fontMetrics(font);
    int lineHeight = fontMetrics.height();  // 单行高度
    int maxWidth = 0;
    
    // 遍历所有文本行，找出最大宽度
    foreach (const QString& line, textLines) {
        int lineWidth = getTextWidth(line, fontName, fontSize).cx;
        if (lineWidth > maxWidth) {
            maxWidth = lineWidth;
        }
    }
    
    // 计算总体文本尺寸和画布参数
    int h = lineHeight * textLines.size();        // 总文本高度 = 单行高度 × 行数
    int xMax = qMax(maxWidth, pageWidth);         // SVG宽度：文字宽度和页面宽度的最大值
    int yMax = pageHeight;                        // SVG高度：与页面高度相同
    
    QSize rotatedTextBlockSize(xMax, yMax);
    
    // 初始化SVG生成器
    QSvgGenerator generator;
    generator.setFileName(svgName);                       // 设置输出文件名
    generator.setSize(rotatedTextBlockSize);              // 设置SVG尺寸
    generator.setViewBox(QRect(0, 0, xMax, yMax));        // 设置视口大小
    
    // 创建绘图器对象
    QPainter painter;

    painter.begin(&generator);
    
    // 设置绘图参数
    painter.setPen(QColor(color));    // 设置文字颜色
    painter.setFont(font);            // 设置字体
    painter.setOpacity(opacity);      // 设置透明度
    
    // 计算旋转中心点（页面中心偏移一定距离）
    QPoint c = QPoint(xMax / 2 + 50,    // X: 页面宽度一半 + 50像素补偿
                      yMax / 2 + h / 2); // Y: 页面高度一半 + 文字高度一半
    
    // 应用旋转变换
    painter.translate(c);     // 将坐标原点移动到旋转中心
    painter.rotate(angle);    // 旋转指定角度
    painter.translate(-c);    // 恢复坐标原点
    
    // 绘制多行文本
    for (int i = 0; i < textLines.size(); ++i) {
        // 计算每行文本的位置
        QPoint textPosition(0, yMax / 2 + i * lineHeight);
        
        // 绘制文本行，左对齐并垂直居中
        painter.drawText(
            QRect(textPosition, QSize(rotatedTextBlockSize.width(), lineHeight)),
            Qt::AlignLeft | Qt::AlignVCenter, textLines.at(i));
    }
    
    painter.end();  // 结束绘图操作
}

} // namespace GeometryUtils
