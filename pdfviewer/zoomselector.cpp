/**
 * @file zoomselector.cpp
 * @brief PDF缩放选择器控件实现
 * 
 * 提供PDF文档的缩放控制功能，包括预设缩放模式和
 * 自定义缩放比例输入
 */

#pragma execution_character_set("utf-8")
#include "zoomselector.h"

#include <QLineEdit>

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 * 
 * 创建缩放选择器的下拉组合框：
 * - 设置为可编辑模式，允许用户输入自定义缩放比例
 * - 添加预设的缩放选项（适合宽度、适合页面、各种百分比）
 * - 连接信号槽处理用户选择和输入
 */
ZoomSelector::ZoomSelector(QWidget *parent) : QComboBox(parent) {

  // 设置为可编辑模式，允许用户输入自定义缩放值
  setEditable(true);

  // 添加预设缩放选项
  addItem("适合宽度");          // 自适应宽度模式
  addItem("适合页面");          // 自适应页面模式
  addItem(QLatin1String("12%"));   // 最小缩放
  addItem(QLatin1String("25%"));
  addItem(QLatin1String("33%"));
  addItem(QLatin1String("50%"));
  addItem(QLatin1String("66%"));
  addItem(QLatin1String("75%"));
  addItem(QLatin1String("100%"));  // 原始大小
  addItem(QLatin1String("125%"));
  addItem(QLatin1String("150%"));
  addItem(QLatin1String("200%"));
  addItem(QLatin1String("400%"));  // 最大缩放

  // 连接下拉选择变化信号
  connect(this,
          static_cast<void (QComboBox::*)(const QString &)>(
              &QComboBox::currentIndexChanged),
          this, &ZoomSelector::onCurrentTextChanged);

  // 连接输入框编辑完成信号，处理用户自定义输入
  connect(lineEdit(), &QLineEdit::editingFinished, this,
          [this]() { onCurrentTextChanged(lineEdit()->text()); });
}

/**
 * @brief 设置缩放倍数
 * @param zoomFactor 缩放倍数（1.0表示100%）
 * 
 * 将传入的缩放倍数转换为百分比格式并显示在组合框中。
 * 例如：1.5 -> "150%"
 */
void ZoomSelector::setZoomFactor(qreal zoomFactor) {
  setCurrentText(QString::number(qRound(zoomFactor * 100)) +
                 QLatin1String("%"));
}

/**
 * @brief 重置缩放选择器到默认状态
 * 
 * 将缩放选择器重置为100%（索引5对应"100%"选项）
 */
void ZoomSelector::reset() {
  setCurrentIndex(5);  // 设置为100%（在预设选项列表中的索引位置）
}

/**
 * @brief 当前文本变化事件处理
 * @param text 用户选择或输入的文本
 * 
 * 根据用户选择的缩放选项执行相应操作：
 * 1. "适合宽度" - 设置为自适应宽度模式
 * 2. "适合页面" - 设置为自适应页面模式  
 * 3. 百分比数值 - 解析并设置为自定义缩放倍数
 */
void ZoomSelector::onCurrentTextChanged(const QString &text) {
  if (text == "适合宽度") {
    // 发送适合宽度模式信号
    emit zoomModeChanged(QPdfView::FitToWidth);
  } else if (text == "适合页面") {
    // 发送适合页面模式信号
    emit zoomModeChanged(QPdfView::FitInView);
  } else {
    // 处理百分比数值输入
    qreal factor = 1.0;  // 默认缩放倍数

    // 去除百分号，提取数值部分
    QString withoutPercent(text);
    withoutPercent.remove(QLatin1Char('%'));

    // 尝试转换为整数
    bool ok = false;
    const int zoomLevel = withoutPercent.toInt(&ok);
    if (ok) factor = zoomLevel / 100.0;  // 转换为倍数（如150% -> 1.5）

    // 发送自定义缩放模式和倍数信号
    emit zoomModeChanged(QPdfView::CustomZoom);
    emit zoomFactorChanged(factor);
  }
}
