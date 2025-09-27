#pragma execution_character_set("utf-8")
#include "include/QProgressIndicator.h"

#include <QDebug>
#include <QPainter>

/**
 * 自定义圆形进度指示器控件
 * 该控件显示一个旋转的圆形加载动画，类似于网页或移动应用中的加载指示器
 * @brief QProgressIndicator::QProgressIndicator 构造函数
 * @param parent 父窗口指针
 */
QProgressIndicator::QProgressIndicator(QWidget* parent)
    : QWidget(parent),
      m_angle(0),                      // 当前旋转角度，初始为0
      m_timerId(-1),                   // 定时器ID，-1表示定时器未启动
      m_delay(40),                     // 动画刷新延迟时间（毫秒），默认40ms，相当于25fps
      m_displayedWhenStopped(false),   // 停止时是否显示，默认为false（停止时隐藏）
      m_color(Qt::black) {             // 指示器颜色，默认为黑色
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);  // 设置为固定大小策略
  setFocusPolicy(Qt::NoFocus);                            // 不接受键盘焦点
}

/**
 * 检查动画是否正在运行
 * @return true 表示动画正在运行，false 表示动画已停止
 */
bool QProgressIndicator::isAnimated() const { return (m_timerId != -1); }

/**
 * 设置动画停止时是否仍然显示控件
 * @param state true表示停止时仍显示，false表示停止时隐藏
 */
void QProgressIndicator::setDisplayedWhenStopped(bool state) {
  m_displayedWhenStopped = state;

  update();  // 触发重绘以更新显示状态
}
/**
 * 获取动画停止时的显示状态
 * @return true表示停止时仍显示，false表示停止时隐藏
 */
bool QProgressIndicator::isDisplayedWhenStopped() const {
  return m_displayedWhenStopped;
}

/**
 * 开始播放加载动画
 * 启动定时器，定期更新旋转角度以产生动画效果
 */
void QProgressIndicator::startAnimation() {
  // qDebug() << "开启动画";
  m_angle = 0;  // 重置旋转角度为0

  // 如果定时器未启动，则启动定时器
  if (m_timerId == -1) m_timerId = startTimer(m_delay);
}

/**
 * 停止播放加载动画
 * 停止定时器并更新显示
 */
void QProgressIndicator::stopAnimation() {
  // qDebug() << "停止动画";
  if (m_timerId != -1) killTimer(m_timerId);  // 如果定时器在运行，则停止它

  m_timerId = -1;  // 重置定时器ID

  update();  // 触发重绘
}

/**
 * 设置动画刷新延迟时间
 * @param delay 延迟时间（毫秒），值越小动画越流畅，但CPU占用越高
 */
void QProgressIndicator::setAnimationDelay(int delay) {
  if (m_timerId != -1) killTimer(m_timerId);  // 先停止当前定时器

  m_delay = delay;  // 更新延迟时间

  // 如果之前动画在运行，使用新的延迟时间重新启动定时器
  if (m_timerId != -1) m_timerId = startTimer(m_delay);
}

/**
 * 设置进度指示器的颜色
 * @param color 要设置的颜色
 */
void QProgressIndicator::setColor(const QColor& color) {
  m_color = color;

  update();  // 触发重绘以应用新颜色
}

/**
 * 获取控件的推荐大小
 * @return 返回推荐的控件大小，默认为30x30像素
 */
QSize QProgressIndicator::sizeHint() const { return QSize(30, 30); }

/**
 * 根据给定宽度计算高度，保持控件为正方形
 * @param w 给定的宽度
 * @return 返回相同的高度值，使控件保持正方形
 */
int QProgressIndicator::heightForWidth(int w) const { return w; }

/**
 * 定时器事件处理函数
 * 每次定时器触发时，更新旋转角度并重绘控件
 * @param event 定时器事件（未使用）
 */
void QProgressIndicator::timerEvent(QTimerEvent* /*event*/) {
  m_angle = (m_angle + 30) % 360;  // 每次增加30度，360度后重置

  update();  // 触发重绘
}

/**
 * 绘制事件处理函数
 * 绘制圆形进度指示器，由12个圆角矩形（胶囊形状）组成一个圆圈
 * 每个胶囊的透明度逐渐降低，形成渐变效果
 * @param event 绘制事件（未使用）
 */
void QProgressIndicator::paintEvent(QPaintEvent* /*event*/) {
  // 如果动画已停止且设置为停止时不显示，则不绘制
  if (!m_displayedWhenStopped && !isAnimated()) return;

  // 获取控件的最小边长，确保绘制的是圆形
  int width = qMin(this->width(), this->height());

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿，使绘制更平滑

  // 计算外圆和内圆的半径
  int outerRadius = (width - 1) * 0.5;      // 外圆半径
  int innerRadius = (width - 1) * 0.5 * 0.38;  // 内圆半径，为外圆的38%

  // 计算胶囊形状的尺寸
  int capsuleHeight = outerRadius - innerRadius;  // 胶囊高度
  // 根据控件大小调整胶囊宽度比例，大控件用更细的胶囊
  int capsuleWidth = (width > 32) ? capsuleHeight * .23 : capsuleHeight * .35;
  int capsuleRadius = capsuleWidth / 2;  // 胶囊圆角半径

  // 绘制12个胶囊形状，组成圆形进度指示器
  for (int i = 0; i < 12; i++) {
    QColor color = m_color;
    // 设置透明度，从最亮（当前位置）到最暗（最远位置）渐变
    color.setAlphaF(1.0f - (i / 12.0f));
    p.setPen(Qt::NoPen);  // 无边框
    p.setBrush(color);    // 设置填充颜色
    p.save();             // 保存当前绘制状态
    p.translate(rect().center());  // 将坐标原点移到控件中心
    p.rotate(m_angle - i * 30.0f);  // 旋转到相应角度（每个胶囊相差30度）
    // 绘制胶囊形状（圆角矩形）
    p.drawRoundedRect(-capsuleWidth * 0.5, -(innerRadius + capsuleHeight),
                      capsuleWidth, capsuleHeight, capsuleRadius,
                      capsuleRadius);
    p.restore();  // 恢复绘制状态
  }
}
