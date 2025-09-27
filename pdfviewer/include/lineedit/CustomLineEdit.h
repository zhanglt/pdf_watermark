#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLineEdit>
#include <QMimeData>
#include <QtWidgets/Qwidget>

/**
 * @brief 自定义输入框类，继承自QLineEdit
 * 
 * 该类扩展了QLineEdit的功能，支持拖放操作和焦点事件处理。
 * 主要用于接收拖放的文件路径，并在失去焦点时发出数据变化信号。
 */
class CustomLineEdit : public QLineEdit {
  Q_OBJECT

 public:
  /**
   * @brief 构造函数
   * @param parent 父窗口指针，默认为nullptr
   */
  CustomLineEdit(QWidget* parent = nullptr);
  
  /**
   * @brief 析构函数
   */
  ~CustomLineEdit();

  /**
   * @brief 拖动进入事件处理
   * @param event 拖动进入事件对象
   * 
   * 当用户拖动文件到输入框时触发。
   * 该函数判断拖动的数据是否为可接受的格式。
   */
  void dragEnterEvent(QDragEnterEvent* event);
  
  /**
   * @brief 拖动移动事件处理
   * @param event 拖动移动事件对象
   * 
   * 当用户在输入框内移动拖动的文件时触发。
   * 用于更新拖动操作的视觉反馈。
   */
  void dragMoveEvent(QDragMoveEvent* event);
  
  /**
   * @brief 拖放事件处理
   * @param event 拖放事件对象
   * 
   * 当用户在输入框内释放拖动的文件时触发。
   * 该函数负责提取文件路径并设置到输入框中。
   */
  void dropEvent(QDropEvent* event);
  
  /**
   * @brief 失去焦点事件处理
   * @param event 焦点事件对象
   * 
   * 当输入框失去焦点时触发。
   * 通常用于触发数据验证或发送数据变化信号。
   */
  void focusOutEvent(QFocusEvent* event);
  
 signals:
  /**
   * @brief 数据变化信号
   * 
   * 当输入框的内容发生变化时发出此信号。
   * 可用于通知其他组件输入框的数据已更新。
   */
  void dataChanged();
};
