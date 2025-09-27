#include "include/slider/CustomSlider.h"

#include <QMessageBox>

#include "qcoreapplication.h"
#include "qcoreevent.h"

/**
 * @brief 鼠标按下事件处理函数的实现
 * @param event 鼠标事件对象
 * 
 * 该函数重写了QSlider的鼠标按下事件处理，实现了点击滑块轨道时
 * 直接跳转到点击位置的功能。
 * 
 * 工作流程：
 * 1. 首先保存当前滑块的值
 * 2. 调用父类的鼠标点击处理函数（这会计算新位置）
 * 3. 恢复原值（避免默认行为）
 * 4. 向父窗口发送自定义事件，通知滑块被点击
 * 
 * 注意：这里的实现方式比较特殊，先让父类计算位置，然后又恢复原值，
 * 可能是为了触发某些内部计算但又不想立即改变值。
 */
void CustomSlider::mousePressEvent(QMouseEvent* event) {
  // 获取点击触发前的值
  const int value = this->value();
  
  // 调用父类的鼠标点击处理事件
  // 父类会根据点击位置计算新的滑块值
  QSlider::mousePressEvent(event);
  
  // 恢复原来的值
  // 这样做可能是为了让应用程序自己决定如何处理点击事件
  setValue(value);
  
  // 向父窗口发送自定义事件
  // 使用 QEvent::User + 1 作为自定义事件类型
  // 父窗口可以通过事件过滤器捕获这个事件并进行相应处理
  QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
  QCoreApplication::sendEvent(parentWidget(), &evEvent);
}

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 * 
 * 调用QSlider的构造函数进行初始化
 */
CustomSlider::CustomSlider(QWidget* parent) : QSlider(parent) {}

/**
 * @brief 析构函数
 * 
 * 清理资源（当前为空实现）
 */
CustomSlider::~CustomSlider() {}
