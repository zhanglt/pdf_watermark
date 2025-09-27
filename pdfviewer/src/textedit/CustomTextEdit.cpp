
#include "include/textedit/CustomTextEdit.h"

#include <QMessageBox>

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 * 
 * 初始化自定义文本编辑框，设置接受拖放操作
 */
CustomTextEdit::CustomTextEdit(QWidget* parent) : QTextEdit(parent) {
  // 启用拖放功能，允许接收拖放的文件
  setAcceptDrops(true);
}

/**
 * @brief 析构函数
 */
CustomTextEdit::~CustomTextEdit() {}

/**
 * @brief 拖动进入事件处理函数
 * @param event 拖动进入事件对象
 * 
 * 当拖动对象进入文本编辑框区域时触发。
 * 检查拖动的数据是否包含文件URL，如果包含则接受该拖动操作。
 */
void CustomTextEdit::dragEnterEvent(QDragEnterEvent* event) {
  // 检查MIME数据中是否包含URL（通常是文件路径）
  if (event->mimeData()->hasUrls())
    event->acceptProposedAction();  // 接受拖动操作，显示可以放置的视觉反馈
  else
    event->ignore();  // 忽略不包含URL的拖动操作
}

/**
 * @brief 拖动移动事件处理函数
 * @param event 拖动移动事件对象
 * 
 * 当拖动对象在文本编辑框内移动时触发。
 * 当前为空实现，可根据需要添加移动时的视觉反馈。
 */
void CustomTextEdit::dragMoveEvent(QDragMoveEvent* event) {}

/**
 * @brief 拖放事件处理函数
 * @param event 拖放事件对象
 * 
 * 当用户在文本编辑框内释放拖动的对象时触发。
 * 提取第一个文件的路径并设置为文本框的内容。
 * 
 * 注意：当前实现只处理第一个文件，如果需要支持多文件，
 * 可以修改为遍历所有URL并换行显示。
 */
void CustomTextEdit::dropEvent(QDropEvent* event) {
  // 获取拖放数据
  const QMimeData* mimeData = event->mimeData();
  
  // 检查是否包含文件URL
  if (mimeData->hasUrls()) {
    // 获取所有拖放的文件URL列表
    QList<QUrl> urls = mimeData->urls();
    
    // 将第一个文件的本地路径转换为字符串
    QString fileName = urls.at(0).toLocalFile();
    
    // 设置文本框内容为文件路径
    setText(fileName);
    
    // 接受拖放事件，表示处理成功
    event->accept();
  } else {
    // 不包含URL则忽略该事件
    event->ignore();
  }
}

/**
 * @brief 失去焦点事件处理函数
 * @param event 焦点事件对象
 * 
 * 当文本编辑框失去焦点时触发。
 * 检查文档是否被修改，如果有修改则发出数据变化信号。
 */
void CustomTextEdit::focusOutEvent(QFocusEvent* event) {
  // 调用基类的失去焦点事件处理
  QTextEdit::focusOutEvent(event);
  
  // 检查文档是否被修改过
  if (this->document()->isModified()) {
    // 重新设置纯文本内容（这会清除格式）
    setText(this->toPlainText());
    
    // 发出数据变化信号，通知其他组件文本内容已更新
    // 注释掉的代码：显示失去焦点的消息框（用于调试）
    // QMessageBox::information(this, "Focus Out", "LineEdit has lost focus.");
    emit dataChanged();
  }
}
