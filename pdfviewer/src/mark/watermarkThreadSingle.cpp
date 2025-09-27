/**
 * @file watermarkThreadSingle.cpp
 * @brief 单文件水印处理线程类的实现
 * 
 * 该文件实现了watermarkThreadSingle类，用于在独立线程中为单个PDF文件添加水印。
 * 主要功能包括：
 * - 支持设置水印文本、颜色、透明度、字体、旋转角度等属性
 * - 基于QRunnable实现，可在Qt线程池中并发执行
 * - 使用互斥锁保证多线程环境下的数据安全
 * - 通过信号槽机制与主线程通信
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

// 设置编译器使用UTF-8字符集，防止中文字符乱码
#pragma execution_character_set("utf-8")
#include "include/mark/watermarkThreadSingle.h"

#include <QDebug>        // Qt调试输出支持
#include <QElapsedTimer> // Qt计时器，用于测量执行时间
#include <QThread>       // Qt线程支持

#include "include/mark/mark.h" // 水印功能相关头文件

/**
 * @brief 构造函数 - 用于批量水印处理
 * @param mutex 互斥锁，用于保护共享资源的线程安全
 * @param map 水印处理结果映射表指针
 */
watermarkThreadSingle::watermarkThreadSingle(QMutex *mutex, wMap *map)
    : m_mutex(mutex), m_wmap(map) {
  setAutoDelete(true); // 设置任务完成后自动删除该对象
}

/**
 * @brief 构造函数 - 基本构造函数
 * @param parent 父对象指针
 */
watermarkThreadSingle::watermarkThreadSingle(QObject *parent)
    : QObject(parent), QRunnable() {
  setAutoDelete(true); // 设置任务完成后自动删除该对象
}

/**
 * @brief 设置任务编号
 * @param num 任务编号
 */
void watermarkThreadSingle::recvNum(int num) { m_num = num; }

/**
 * @brief 设置输入PDF文件路径
 * @param input 输入文件的完整路径
 */
void watermarkThreadSingle::setInputFilename(QString input) { m_input = input; }

/**
 * @brief 设置输出PDF文件路径
 * @param ouput 输出文件的完整路径
 */
void watermarkThreadSingle::setOutputFilename(QString ouput) {
  m_output = ouput;
}

/**
 * @brief 设置水印文本内容
 * @param text 要添加的水印文字，如果为空则不设置
 */
void watermarkThreadSingle::setText(QString text) {
  if (!text.isEmpty()) {
    m_text = text;
  }
}

/**
 * @brief 设置水印透明度
 * @param opacity 透明度值，会自动添加%符号
 */
void watermarkThreadSingle::setOpacity(QString opacity) {
  m_opacity = opacity + "%";
}

/**
 * @brief 设置水印颜色
 * @param color 颜色值，通常为十六进制格式如#FF0000
 */
void watermarkThreadSingle::setColor(QString color) { m_color = color; }

/**
 * @brief 设置水印字体
 * @param font 字体名称
 */
void watermarkThreadSingle::setFont(QString font) { m_font = font; }

/**
 * @brief 设置水印旋转角度
 * @param rotate 旋转角度，单位为度
 */
void watermarkThreadSingle::setRotate(QString rotate) { m_rotate = rotate; }

/**
 * @brief 设置水印字体大小
 * @param fontsize 字体大小
 */
void watermarkThreadSingle::setFontsize(QString fontsize) {
  m_fontsize = fontsize;
}

/**
 * @brief 线程执行函数 - 在独立线程中执行水印添加任务
 * 
 * 该函数是QRunnable接口的实现，会在Qt线程池中的工作线程中执行
 * 主要功能：
 * 1. 调用addWatermark函数为PDF文件添加水印
 * 2. 使用互斥锁保护共享的结果映射表
 * 3. 发射完成信号通知主线程
 */
void watermarkThreadSingle::run() {
  // 注释的调试代码 - 用于显示当前线程地址
  // qDebug() << "生成随机数的线程的线程地址: " << QThread::currentThread();

  // 注释的性能测试代码
  // QElapsedTimer time;
  // time.start();
  // wMap map;
  // qDebug()<<"输入："<<m_filename;
  // qDebug()<<"输出："<<pathChange(m_input,m_output, m_filename, "_out_").replace("//", "/");
  // if ((!m_input.contains("_out_")) && (!m_input.contains("_pdf_"))) {
  
  // 执行水印添加操作，返回处理结果（0表示成功，其他值表示错误码）
  int r = addWatermark(m_input, m_output, m_text, m_opacity, m_color, m_rotate, m_font);
  
  // 使用互斥锁保护共享资源，确保多线程安全
  m_mutex->lock();
  m_wmap->insert(m_input, r); // 将处理结果存入映射表
  m_mutex->unlock();
  
  // 注释的测试代码示例
  // int r=addWatermark("h:/in.pdf", "h:/out.pdf", 
  //                    "联通数字科技有限公司总部投标专用文档", "15%", "#c8250c", "45", "simkai");
  // int milsec = time.elapsed(); 
  // qDebug() << "生成" << m_num << "个随机数总共用时:" << milsec << "毫秒";

  // 发射完成信号，通知主线程任务已完成
  emit addFinish(*m_wmap);
  //  }
}
