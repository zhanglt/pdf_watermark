#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTextEdit>
#include <QMimeData>
#include <QtWidgets/Qwidget>

/**
 * @brief 自定义文本编辑框类，继承自QTextEdit
 * 
 * 该类扩展了QTextEdit的功能，支持拖放操作和焦点事件处理。
 * 主要用于接收拖放的文件，支持多个文件路径的显示，
 * 并在失去焦点时发出数据变化信号。
 */
class CustomTextEdit : public QTextEdit {
  Q_OBJECT

 public:
  /**
   * @brief 构造函数
   * @param parent 父窗口指针，默认为nullptr
   */
  CustomTextEdit(QWidget* parent = nullptr);
  
  /**
   * @brief 析构函数
   */
  ~CustomTextEdit();

 protected:
  /**
   * @brief 拖动进入事件处理
   * @param event 拖动进入事件对象
   * 
   * 当用户拖动文件到文本编辑框时触发。
   * 该函数判断拖动的数据是否为可接受的格式（如文件URL）。
   */
  void dragEnterEvent(QDragEnterEvent* event);
  
  /**
   * @brief 拖动移动事件处理
   * @param event 拖动移动事件对象
   * 
   * 当用户在文本编辑框内移动拖动的文件时触发。
   * 用于更新拖动操作的视觉反馈，如改变光标样式。
   */
  void dragMoveEvent(QDragMoveEvent* event);
  
  /**
   * @brief 拖放事件处理
   * @param event 拖放事件对象
   * 
   * 当用户在文本编辑框内释放拖动的文件时触发。
   * 该函数负责提取文件路径并将其添加到文本编辑框中。
   * 支持同时拖放多个文件，每个文件路径占一行。
   */
  void dropEvent(QDropEvent* event);
  
  /**
   * @brief 失去焦点事件处理
   * @param event 焦点事件对象
   * 
   * 当文本编辑框失去焦点时触发。
   * 通常用于触发数据验证或发送数据变化信号，
   * 通知其他组件文本内容已经更新。
   */
  void focusOutEvent(QFocusEvent* event);
  
 signals:
  /**
   * @brief 数据变化信号
   * 
   * 当文本编辑框的内容发生变化时发出此信号。
   * 可用于通知其他组件文本编辑框的数据已更新，
   * 触发相关的业务逻辑处理。
   */
  void dataChanged();
};
