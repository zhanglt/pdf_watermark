/**
 * @file multiWatermarkThreadSingle.cpp
 * @brief 多行文本水印处理线程类的实现
 * 
 * 该文件实现了multiWatermarkThreadSingle类，用于在独立线程中为单个PDF文件添加多行文本水印。
 * 相比于单行水印处理，该类支持：
 * - 多行文本水印的添加和格式化
 * - 更灵活的文本布局和排版
 * - 支持换行和文本流控制
 * - 基于QRunnable实现并发处理
 * - 线程安全的结果管理
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#include "function.h"                                      // 核心功能函数声明
// 设置编译器使用UTF-8字符集，防止中文字符乱码
#pragma execution_character_set("utf-8")
#include <QDebug>                                          // Qt调试输出支持
#include <QElapsedTimer>                                   // Qt计时器，用于性能测试
#include <QThread>                                         // Qt线程支持

#include "include/mark/mark.h"                            // 水印处理相关头文件
#include "include/mark/multiWatermarkThreadSingle.h"      // 多行水印线程类头文件

/**
 * @brief 构造函数 - 用于批量多行水印处理
 * @param mutex 互斥锁指针，用于保护共享资源的线程安全访问
 * @param map 水印处理结果映射表指针，存储文件路径与处理结果的对应关系
 */
multiWatermarkThreadSingle::multiWatermarkThreadSingle(QMutex *mutex, wMap *map)
    : m_mutex(mutex), m_wmap(map) {
  setAutoDelete(true); // 设置任务完成后自动删除该对象，避免内存泄漏
}

/**
 * @brief 构造函数 - 基本构造函数
 * @param parent 父对象指针，用于Qt对象树管理
 */
multiWatermarkThreadSingle::multiWatermarkThreadSingle(QObject *parent)
    : QObject(parent), QRunnable() {
  setAutoDelete(true); // 设置任务完成后自动删除该对象，避免内存泄漏
}

/**
 * @brief 设置任务编号
 * @param num 任务编号，用于标识当前处理的任务
 */
void multiWatermarkThreadSingle::recvNum(int num) { m_num = num; }

/**
 * @brief 设置输入PDF文件路径
 * @param input 待处理的PDF文件完整路径
 */
void multiWatermarkThreadSingle::setInputFilename(QString input) {
  m_input = input;
}

/**
 * @brief 设置输出PDF文件路径
 * @param ouput 处理后输出的PDF文件完整路径
 */
void multiWatermarkThreadSingle::setOutputFilename(QString ouput) {
  m_output = ouput;
}

/**
 * @brief 设置多行水印文本内容
 * @param text 要添加的多行水印文字，支持换行符分隔的多行文本
 * 
 * 注意：只有当文本不为空时才会设置，支持包含换行符的多行文本
 */
void multiWatermarkThreadSingle::setText(QString text) {
  if (!text.isEmpty()) {
    m_text = text;
  }
}

/**
 * @brief 设置水印透明度
 * @param opacity 透明度值，数值格式（如"50"表示50%透明度）
 * 
 * 注意：与单行水印不同，这里不自动添加%符号，在实际使用时会转换为0-1之间的小数
 */
void multiWatermarkThreadSingle::setOpacity(QString opacity) {
  m_opacity = opacity;
}

/**
 * @brief 设置水印文本颜色
 * @param color 颜色值，支持十六进制格式（如#FF0000）或颜色名称
 */
void multiWatermarkThreadSingle::setColor(QString color) { m_color = color; }

/**
 * @brief 设置水印字体
 * @param font 字体名称，如"simkai"、"Arial"等
 */
void multiWatermarkThreadSingle::setFont(QString font) { m_font = font; }

/**
 * @brief 设置水印旋转角度
 * @param rotate 旋转角度值，字符串格式，实际使用时会转换为double类型
 */
void multiWatermarkThreadSingle::setRotate(QString rotate) {
  m_rotate = rotate;
}

/**
 * @brief 设置水印字体大小
 * @param fontsize 字体大小值，字符串格式，实际使用时会转换为int类型
 */
void multiWatermarkThreadSingle::setFontsize(QString fontsize) {
  m_fontsize = fontsize;
}

/**
 * @brief 线程执行函数 - 在独立线程中执行多行水印添加任务
 * 
 * 该函数是QRunnable接口的实现，会在Qt线程池中的工作线程中执行。
 * 主要功能：
 * 1. 调用addWatermark_multiline函数为PDF文件添加多行文本水印
 * 2. 进行必要的数据类型转换（字符串转数值类型）
 * 3. 使用互斥锁保护共享的结果映射表
 * 4. 发射完成信号通知主线程处理结果
 * 
 * 与单行水印处理的区别：
 * - 使用addWatermark_multiline函数而非addWatermark
 * - 支持多行文本的布局和排版
 * - 透明度参数需要从百分比转换为0-1之间的小数
 */
void multiWatermarkThreadSingle::run() {
  // 注释的调试代码 - 用于显示当前执行线程的地址
  // qDebug() << "生成随机数的线程的线程地址: " << QThread::currentThread();

  // 注释的性能测试相关代码
  // QElapsedTimer time;
  // time.start();
  // wMap map;
  // qDebug()<<"输入："<<m_filename;
  // qDebug()<<"输出："<<pathChange(m_input,m_output, m_filename, "_out_").replace("//", "/");

  // 注释的单行水印处理代码（已被多行处理替代）
  // int r = addWatermark(m_input, m_output, m_text, m_opacity, m_color, m_rotate,m_font);
  // if ((!m_input.contains("_out_")) && (!m_input.contains("_pdf_"))) {
  
  // 执行多行水印添加操作
  // 参数说明：
  // - m_input.toStdString(): 输入文件路径（转换为std::string）
  // - m_output.toStdString(): 输出文件路径（转换为std::string）
  // - m_text: 多行文本内容（QString格式）
  // - m_font: 字体名称
  // - m_fontsize.toInt(): 字体大小（转换为整数）
  // - m_color: 文本颜色
  // - m_rotate.toDouble(): 旋转角度（转换为double）
  // - m_opacity.toDouble() / 100: 透明度（转换为0-1之间的小数）
  int r = addWatermark_multiline(m_input.toStdString(), m_output.toStdString(),
                                 m_text, m_font, m_fontsize.toInt(), m_color,
                                 m_rotate.toDouble(), m_opacity.toDouble() / 100);

  // 使用互斥锁保护共享资源，确保多线程安全
  m_mutex->lock();
  m_wmap->insert(m_input, r); // 将处理结果存入映射表（0表示成功，其他值表示错误码）
  m_mutex->unlock();
  
  // 注释的测试用例代码
  // int r=addWatermark("h:/in.pdf", "h:/out.pdf", 
  //                    "联通数字科技有限公司总部投标专用文档", "15%", "#c8250c", "45", "simkai");
  // int milsec = time.elapsed(); 
  // qDebug() << "生成" << m_num << "个随机数总共用时:" << milsec << "毫秒";

  // 发射完成信号，通知主线程任务已完成
  emit addFinish(*m_wmap);
  // }
}
