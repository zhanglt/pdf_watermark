/**
 * @file mark.h
 * @brief PDF水印处理功能头文件
 * @author PDF工具集项目组
 * @date 2024
 * 
 * 本头文件定义了PDF文档水印添加的相关函数接口。
 * 提供了完整的PDF水印处理解决方案，包括：
 * - 单页和多页水印添加
 * - 字符串格式转换工具
 * - 系统路径获取函数
 * - Qt框架集成接口
 * 
 * 支持的水印参数：
 * - 水印文本内容和字体设置
 * - 透明度、颜色和旋转角度控制
 * - 位置对齐和缩放比例调整
 * 
 * 依赖库：PDFlib、Qt5核心模块
 */

#pragma once
#ifndef WATERMARK_H
#define WATERMARL_H

#include <QObject>
#include <string>

using namespace std;

/**
 * @brief 将标准字符串转换为宽字符串
 * @param s 输入的标准字符串
 * @return 转换后的宽字符串
 */
std::wstring String2WString(const std::string& s);

/**
 * @brief 获取PDF文件的页数
 * @param filename PDF文件名（宽字符串格式）
 * @param flag 标记参数
 * @return PDF文件的页数，失败返回负值
 */
int getpages(std::wstring filename, string flag);

/**
 * @brief 为PDF文件添加单页水印
 * @param in 输入PDF文件路径
 * @param out 输出PDF文件路径
 * @param mark_txt 水印文本内容
 * @param mark_opacity 水印透明度（如"15%"）
 * @param mark_font 水印字体名称
 * @param mark_rotate 水印旋转角度
 * @param mark_color 水印颜色
 * @return 处理结果，0表示成功
 */
int setSingleMark(string in, string out, wstring mark_txt, wstring mark_opacity,
                  wstring mark_font, wstring mark_rotate, wstring mark_color);

/**
 * @brief 为PDF文件添加多页水印（高级版本）
 * @param infile 输入PDF文件路径
 * @param outfile 输出PDF文件路径
 * @param mark_txt 水印文本内容
 * @param mark_opacity 水印透明度
 * @param mark_font 水印字体
 * @param mark_rotate 水印旋转角度
 * @param mark_color 水印颜色
 * @param scale 水印缩放比例
 * @param vertalign 垂直对齐方式（top/center/bottom）
 * @param vertshift 垂直偏移量
 * @return 处理结果，0表示成功
 */
int setMark(std::string infile, std::string outfile, wstring mark_txt,
            wstring mark_opacity, wstring mark_font, wstring mark_rotate,
            wstring mark_color, wstring scale, wstring vertalign,
            wstring vertshift);

/**
 * @brief 获取系统字体文件夹路径
 * @return 字体文件夹的完整路径（宽字符串格式）
 */
std::wstring GetFontsFolder();

/**
 * @brief 获取临时文件夹路径
 * @return 临时文件夹路径
 */
std::string GetTmpFolder();

/**
 * @brief 从文件路径中提取文件名
 * @param path 完整文件路径
 * @return 提取出的文件名
 */
std::string GetFilename(string path);

/**
 * @brief 将C风格字符数组转换为标准字符串
 * @param c C风格字符数组指针
 * @return 转换后的标准字符串
 */
std::string char2string(char* c);

/**
 * @brief 将Qt字符串转换为宽字符串
 * @param str Qt字符串对象
 * @return 转换后的宽字符串
 */
std::wstring QString2WString(const QString& str);

/**
 * @brief 添加水印的主要接口函数（Qt版本）
 * @param i 输入PDF文件路径，默认"in.pdf"
 * @param o 输出PDF文件路径，默认"out.pdf"
 * @param t 水印文本，默认为"联通数字科技有限公司总部投标专用文档"
 * @param p 水印透明度百分比，默认"15%"
 * @param c 水印颜色，默认"gray"（灰色）
 * @param r 水印旋转角度，默认"45"度
 * @param f 水印字体，默认"simkai"（楷体）
 * @param s 水印缩放比例，默认"0.6"
 * @param va 垂直对齐方式，默认"center"（居中）
 * @param vs 垂直偏移量，默认"10"
 * @return 处理结果，0表示成功
 */
int addWatermark(const QString& i = "in.pdf", const QString& o = "out.pdf",
                 const QString& t = "联通数字科技有限公司总部投标专用文档",
                 const QString& p = "15%", const QString& c = "gray",
                 const QString& r = "45", const QString& f = "simkai",
                 const QString& s = "0.6", const QString& va = "center",
                 const QString& vs = "10");

#endif  // WATERMARL_H
