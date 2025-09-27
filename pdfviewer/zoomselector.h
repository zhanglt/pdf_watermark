/**
 * @file zoomselector.h
 * @brief PDF缩放选择器控件头文件
 * 
 * 定义了一个基于QComboBox的缩放选择器控件，
 * 提供PDF文档的缩放控制功能
 */

#ifndef ZOOMSELECTOR_H
#define ZOOMSELECTOR_H

#include <QComboBox>
#include <QPdfView>

/**
 * @brief PDF缩放选择器类
 * 
 * 继承自QComboBox，提供PDF文档的缩放控制功能。
 * 支持预设缩放比例、自适应模式和自定义缩放输入。
 */
class ZoomSelector : public QComboBox {
  Q_OBJECT

 public:
  /**
   * @brief 构造函数
   * @param parent 父窗口指针
   */
  explicit ZoomSelector(QWidget *parent = nullptr);

 public slots:
  /**
   * @brief 设置缩放倍数
   * @param zoomFactor 缩放倍数（1.0表示100%）
   * 
   * 将传入的缩放倍数转换为百分比格式并显示
   */
  void setZoomFactor(qreal zoomFactor);

  /**
   * @brief 重置缩放选择器到默认状态
   * 
   * 将缩放选择器重置为100%
   */
  void reset();

 signals:
  /**
   * @brief 缩放模式变化信号
   * @param zoomMode 新的缩放模式
   * 
   * 当用户选择适合宽度、适合页面或自定义缩放时发出
   */
  void zoomModeChanged(QPdfView::ZoomMode zoomMode);
  
  /**
   * @brief 缩放倍数变化信号
   * @param zoomFactor 新的缩放倍数
   * 
   * 当用户输入自定义缩放百分比时发出
   */
  void zoomFactorChanged(qreal zoomFactor);

 private slots:
  /**
   * @brief 当前文本变化事件处理
   * @param text 用户选择或输入的文本
   * 
   * 解析用户输入并发出相应的缩放信号
   */
  void onCurrentTextChanged(const QString &text);
};

#endif  // ZOOMSELECTOR_H
