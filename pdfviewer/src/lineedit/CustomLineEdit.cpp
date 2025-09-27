
/**
 * @file CustomLineEdit.cpp
 * @brief 自定义行编辑控件实现文件
 * @author PDF工具集项目组
 * @date 2024
 * 
 * 本文件实现了一个支持拖拽功能的自定义行编辑控件。
 * 继承自QLineEdit，增加了文件拖拽支持和数据变化通知功能。
 * 
 * 主要特性：
 * - 支持文件拖拽操作
 * - 自动提取拖拽文件的本地路径
 * - 失去焦点时发送数据变化信号
 * - 完整的拖拽事件处理流程
 */

#include "include/lineedit/CustomLineEdit.h"

#include <QMessageBox>

/**
 * @brief 自定义行编辑控件构造函数
 * @param parent 父控件指针
 * 
 * 初始化自定义行编辑控件，启用拖拽功能。
 * 设置接受拖拽操作，允许用户直接拖拽文件到输入框。
 */
CustomLineEdit::CustomLineEdit(QWidget* parent) : QLineEdit(parent) {
  setAcceptDrops(true);  // 启用拖拽功能，允许接收拖拽事件
}

/**
 * @brief 析构函数
 * 
 * 清理资源，执行必要的清理工作。
 */
CustomLineEdit::~CustomLineEdit() {}

/**
 * @brief 拖拽进入事件处理函数
 * @param event 拖拽进入事件对象
 * 
 * 当有对象被拖拽到控件上方时触发。
 * 检查拖拽数据是否包含URL（文件路径），如果包含则接受拖拽操作。
 */
void CustomLineEdit::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls())
    event->acceptProposedAction();  // 事件数据中存在路径，接受拖拽事件
  else
    event->ignore();  // 不包含URL数据，忽略拖拽事件
}

/**
 * @brief 拖拽移动事件处理函数
 * @param event 拖拽移动事件对象
 * 
 * 当拖拽对象在控件上方移动时触发。
 * 当前实现为空，可根据需要扩展功能（如视觉反馈）。
 */
void CustomLineEdit::dragMoveEvent(QDragMoveEvent* event) {
  // 预留接口，可用于添加拖拽移动时的视觉反馈
}

/**
 * @brief 拖拽释放事件处理函数
 * @param event 拖拽释放事件对象
 * 
 * 当拖拽对象在控件上释放时触发。
 * 提取拖拽文件的本地路径并设置到输入框中。
 */
void CustomLineEdit::dropEvent(QDropEvent* event) {
  const QMimeData* mimeData = event->mimeData();  // 获取拖拽数据
  if (mimeData->hasUrls()) {
    QList<QUrl> urls = mimeData->urls();              // 获取URL列表
    QString fileName = urls.at(0).toLocalFile();     // 提取第一个文件的本地路径
    setText(fileName);                                // 将文件路径设置到输入框
    event->accept();                                  // 接受拖拽事件
  } else {
    event->ignore();  // 不包含URL数据，忽略事件
  }
}

/**
 * @brief 失去焦点事件处理函数
 * @param event 焦点事件对象
 * 
 * 当控件失去焦点时触发。
 * 检查内容是否已修改，如果有修改则发送数据变化信号。
 */
void CustomLineEdit::focusOutEvent(QFocusEvent* event) {
  QLineEdit::focusOutEvent(event);  // 调用基类的失去焦点处理
  if (isModified()) {               // 检查内容是否已被修改
    setText(this->text());          // 重新设置文本（清除修改状态）
    // QMessageBox::information(this, "Focus Out", "LineEdit has lost focus.");  // 调试用信息框（已注释）
    emit dataChanged();             // 发射数据变化信号，通知外部组件
  }
}
