#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QtWidgets/QSlider>

/**
 * @brief 自定义滑块控件类
 * 
 * 继承自QSlider，扩展了鼠标点击事件的处理。
 * 主要用于实现点击滑块轨道时直接跳转到点击位置的功能。
 */
class CustomSlider : public QSlider {
 public:
  /**
   * @brief 构造函数
   * @param parent 父窗口指针，默认为nullptr
   */
  explicit CustomSlider(QWidget* parent = nullptr);
  
  /**
   * @brief 析构函数
   */
  ~CustomSlider();

 protected:
  /**
   * @brief 鼠标按下事件处理函数
   * @param event 鼠标事件对象
   * 
   * 重写此方法以实现点击滑块轨道时直接跳转到点击位置的功能。
   * 通常QSlider默认只能通过拖动滑块来改变值，通过重写此方法
   * 可以让用户点击轨道任意位置时，滑块直接跳转到该位置。
   */
  void mousePressEvent(QMouseEvent* event);
};

#endif // CUSTOMSLIDER_H
