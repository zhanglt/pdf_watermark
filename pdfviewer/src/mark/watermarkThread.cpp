/**
 * @file watermarkThread.cpp
 * @brief PDF水印处理线程类的实现文件
 * @author PDF工具集项目组
 * @date 2024
 * 
 * 本文件实现了PDF批量水印处理功能的多线程支持。
 * 通过继承QRunnable和QObject，实现了在后台线程中
 * 对多个PDF文件进行水印添加操作，避免阻塞主线程UI。
 * 
 * 主要功能：
 * - 批量PDF文件水印添加
 * - 多线程并发处理
 * - 处理进度和结果反馈
 * - 自动内存管理
 */

#pragma execution_character_set("utf-8")  //防止出现中文乱码
#include "include/mark/watermarkThread.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QThread>

#include "../../include/mark/mark.h"
#include "function.h"

/**
 * @brief 水印处理线程构造函数
 * @param parent 父对象指针
 * 
 * 初始化水印处理线程，继承自QObject和QRunnable
 * 设置自动删除属性，线程完成后自动清理内存
 */
watermarkThread::watermarkThread(QObject* parent)
    : QObject(parent), QRunnable() {
  setAutoDelete(true);  // 线程执行完毕后自动删除
}

/**
 * @brief 接收处理数量参数
 * @param num 要处理的文件数量
 */
void watermarkThread::recvNum(int num) { m_num = num; }

/**
 * @brief 设置文件名
 * @param filename 当前处理的文件名
 */
void watermarkThread::setFilename(QString filename) { m_filename = filename; }

/**
 * @brief 设置要处理的文件列表
 * @param files 文件路径列表
 */
void watermarkThread::setFiles(QStringList files) { m_files = files; }

/**
 * @brief 设置输入目录路径
 * @param input 输入文件目录
 */
void watermarkThread::setInputDir(QString input) { m_input = input; }

/**
 * @brief 设置输出目录路径
 * @param ouput 输出文件目录
 */
void watermarkThread::setOutputDir(QString ouput) { m_output = ouput; }

/**
 * @brief 设置水印文本内容
 * @param text 水印显示的文本，如果为空则不设置
 */
void watermarkThread::setText(QString text) {
  if (!text.isEmpty()) {
    m_text = text;
  }
}

/**
 * @brief 设置水印透明度
 * @param opacity 透明度数值，会自动添加"%"符号
 */
void watermarkThread::setOpacity(QString opacity) { m_opacity = opacity + "%"; }

/**
 * @brief 设置水印颜色
 * @param color 颜色值，支持颜色名称或十六进制值
 */
void watermarkThread::setColor(QString color) { m_color = color; }

/**
 * @brief 设置水印字体
 * @param font 字体名称
 */
void watermarkThread::setFont(QString font) { m_font = font; }

/**
 * @brief 设置水印旋转角度
 * @param rotate 旋转角度（度数）
 */
void watermarkThread::setRotate(QString rotate) { m_rotate = rotate; }

/**
 * @brief 设置水印字体大小
 * @param fontsize 字体大小值
 */
void watermarkThread::setFontsize(QString fontsize) { m_fontsize = fontsize; }

/**
 * @brief 水印处理线程的主要执行方法
 * 
 * 在独立线程中批量处理PDF文件，为每个文件添加水印
 * 使用QElapsedTimer计时，处理完成后通过信号通知主线程
 */
void watermarkThread::run() {
  qDebug() << "水印处理线程的线程地址: " << QThread::currentThread();

  // 开始计时，用于统计处理耗时
  QElapsedTimer time;
  time.start();
  QMap<QString, int> map;  // 存储处理结果的映射表
  int milsec = time.elapsed();

  // 遍历处理每个PDF文件
  for (const QString& file : m_files) {
    // 输出调试信息：当前处理的文件和输出路径
    qDebug() << "文件名称：" << file << "输出文件："
             << pathChange(m_input, m_output, file, "_out_").replace("//", "/");
    
    // 输出水印参数调试信息
    qDebug() << "文本：" << m_text << "不透明：" << m_opacity;
    qDebug() << "颜色：" << m_color << "旋转：" << m_rotate;
    qDebug() << "字体：" << m_font;
    
    // 调用水印添加功能，处理单个PDF文件
    int r = addWatermark(
        file,  // 输入文件路径
        pathChange(m_input, m_output, file, "_out_").replace("//", "/"),  // 输出文件路径
        m_text,     // 水印文本
        m_opacity,  // 透明度
        m_color,    // 颜色
        m_rotate,   // 旋转角度
        m_font);    // 字体
    
    // 保存处理结果到映射表
    map.insert(m_filename, r);
  }

  // 示例代码（已注释）：单文件水印添加
  // int r=addWatermark("h:/in.pdf", "h:/out.pdf",
  // "联通数字科技有限公司总部投标专用文档", "15%", "#c8250c", "45", "simkai");

  // 输出处理时间统计
  qDebug() << "处理" << m_num << "个文件总共用时:" << milsec << "毫秒";

  // 发射完成信号，将处理结果传递给主线程
  emit addFinish(map);
}
