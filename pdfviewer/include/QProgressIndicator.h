#ifndef QPROGRESSINDICATOR_H
#define QPROGRESSINDICATOR_H

#include <QColor>
#include <QWidget>

/**
 * @class QProgressIndicator
 * @brief 进度指示器类，用于显示不确定的进度
 * 
 * QProgressIndicator类允许应用程序显示一个进度指示器，
 * 用来表明正在执行一个耗时的任务。
 * 
 * 该进度指示器是不确定的（indeterminate），仅通过旋转动画
 * 来表示应用程序正忙。与QProgressBar不同，它不显示具体的进度百分比。
 * 
 * @sa QProgressBar
 */
class QProgressIndicator : public QWidget {
  Q_OBJECT
  Q_PROPERTY(int delay READ animationDelay WRITE setAnimationDelay)
  Q_PROPERTY(bool displayedWhenStopped READ isDisplayedWhenStopped WRITE
                 setDisplayedWhenStopped)
  Q_PROPERTY(QColor color READ color WRITE setColor)
 public:
  /**
   * @brief 构造函数
   * @param parent 父窗口指针，默认为0
   */
  QProgressIndicator(QWidget* parent = 0);

  /**
   * @brief 返回动画步骤之间的延迟时间
   * @return 动画步骤之间的毫秒数。默认情况下，动画延迟设置为40毫秒
   * @sa setAnimationDelay
   */
  int animationDelay() const { return m_delay; }

  /**
   * @brief 返回组件当前是否正在动画
   * @return 动画状态。如果正在动画返回true，否则返回false
   * @sa startAnimation stopAnimation
   */
  bool isAnimated() const;

  /**
   * @brief 返回指示器在停止动画时是否仍然显示
   * @return 如果进度指示器在不动画时仍然显示自己，则返回true。
   *         默认情况下返回false
   * @sa setDisplayedWhenStopped
   */
  bool isDisplayedWhenStopped() const;

  /**
   * @brief 返回组件的颜色
   * @return 当前颜色
   * @sa setColor
   */
  const QColor& color() const { return m_color; }

  /**
   * @brief 返回推荐的大小
   * @return 推荐的组件大小
   */
  virtual QSize sizeHint() const;
  
  /**
   * @brief 返回给定宽度下的高度
   * @param w 宽度
   * @return 对应的高度
   */
  int heightForWidth(int w) const;
 public slots:
  /**
   * @brief 开始旋转动画
   * @sa stopAnimation isAnimated
   */
  void startAnimation();

  /**
   * @brief 停止旋转动画
   * @sa startAnimation isAnimated
   */
  void stopAnimation();

  /**
   * @brief 设置动画步骤之间的延迟
   * 
   * 将延迟设置为大于40的值会减慢动画速度，
   * 而将延迟设置为较小的值会加快动画速度。
   * 
   * @param delay 延迟时间，以毫秒为单位
   * @sa animationDelay
   */
  void setAnimationDelay(int delay);

  /**
   * @brief 设置组件在不动画时是否隐藏自己
   * @param state 显示状态。设置为false时，进度指示器在不动画时会隐藏；
   *              设置为true时，即使不动画也会显示
   * @sa isDisplayedWhenStopped
   */
  void setDisplayedWhenStopped(bool state);

  /**
   * @brief 设置组件的颜色
   * @param color 要设置的颜色
   * @sa color
   */
  void setColor(const QColor& color);

 protected:
  /**
   * @brief 定时器事件处理函数
   * @param event 定时器事件对象
   * 
   * 用于驱动动画的定时更新
   */
  virtual void timerEvent(QTimerEvent* event);
  
  /**
   * @brief 绘制事件处理函数
   * @param event 绘制事件对象
   * 
   * 负责绘制旋转的进度指示器
   */
  virtual void paintEvent(QPaintEvent* event);

 private:
  int m_angle;                 ///< 当前旋转角度
  int m_timerId;               ///< 定时器ID
  int m_delay;                 ///< 动画延迟（毫秒）
  bool m_displayedWhenStopped; ///< 停止时是否显示
  QColor m_color;              ///< 指示器颜色
  
 signals:
  /**
   * @brief 完成信号
   * 
   * 当动画停止时发出此信号
   */
  void Finished();
};

#endif  // QPROGRESSINDICATOR_H
