
/**
 * @file main.cpp
 * @brief PDF查看器应用程序主入口文件
 * @author PDF工具集项目组
 * @date 2024
 * 
 * 本文件为PDF查看器应用程序的主入口点，负责初始化Qt应用程序环境、
 * 加载本地化资源、设置UI样式、创建主窗口并启动应用程序。
 * 
 * 主要功能：
 * - Qt应用程序初始化
 * - 中文本地化支持
 * - CSS样式表加载
 * - 主窗口创建和配置
 * - 默认PDF文档加载
 */

#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8，支持中文
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QPdfView>
#include <QUrl>
#include <QStandardPaths>
//#include "mainwindow.h"
#include "qtranslator.h"
#include "zoomselector.h"

/**
 * @brief 应用程序主入口函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 应用程序退出代码
 * 
 * 初始化Qt应用程序，设置国际化支持、UI样式，
 * 创建并显示主窗口，启动事件循环
 */
int main(int argc, char *argv[]) {
  // 创建Qt应用程序实例
  QApplication app(argc, argv);

  // 加载中文本地化文件，实现界面汉化
  QTranslator base;
  if (base.load("languages/qt_zh_CN.qm")) {
    app.installTranslator(&base);  // 安装翻译器
  } else {
    // 翻译文件加载失败时显示警告对话框
    QMessageBox::warning(
        nullptr, QObject::tr("警告"),
        QObject::tr("加载翻译文件qt_zh_CN.qm失败，部分界面可能无法汉化显示！"));
  }

  // 加载CSS样式表文件，美化界面外观
  QFile file("qss/pdf.css");
  if (file.open(QFile::ReadOnly)) {
    QString qss = QLatin1String(file.readAll());  // 读取样式表内容
    QString paletteColor = qss.mid(20, 7);        // 提取调色板颜色
    qApp->setPalette(QPalette(QColor(paletteColor)));  // 设置应用程序调色板
    qApp->setStyleSheet(qss);                     // 应用样式表
    file.close();
  }

  // 创建主窗口实例
  MainWindow w;

  // 定义标题栏样式（备用样式，已注释）
  QString styleSheet =
      "QWidget::titleBar {"
      "background-color: green;"
      "}";
  
  // 设置主窗口标题栏背景色为浅蓝色
  w.setStyleSheet("QMainWindow::titleBar {background-color: #bbd4ec;}");
  // w.setStyleSheet(styleSheet);  // 备用样式设置

  // 获取命令行参数列表
  QStringList args = app.arguments();
  
  // 设置主窗口固定尺寸为 818x650 像素
  w.setFixedSize(818, 650);
  
  // 配置窗口标志：移除最大化按钮，保留最小化按钮
  w.setWindowFlags(w.windowFlags() & ~Qt::WindowMinMaxButtonsHint |
                   Qt::WindowMinimizeButtonHint);
  
  // 显示主窗口
  w.show();
  
  // 命令行参数处理（已注释）
  // if (args.length() > 1)
  //    w.open(QUrl::fromLocalFile(args[1]));
  
  // PDF文档错误处理变量
  QPdfDocument::DocumentError err;
  
  // 自动打开示例PDF文档
  w.open(QUrl::fromLocalFile("doc/2.pdf"), err);
  
  // 发送信号，设置PDF文档显示模式为适应宽度
  emit w.m_zoomSelector->zoomModeChanged(QPdfView::FitToWidth);

  // 启动Qt事件循环，程序进入主循环等待用户交互
  return app.exec();
}
