/**
 * @file pageselector.cpp
 * @brief PDF页面选择器控件实现
 * 
 * 提供PDF文档的页面导航功能，包括上一页、下一页、
 * 页码输入和总页数显示
 */

#include "pageselector.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPdfPageNavigation>
#include <QToolButton>

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 * 
 * 创建页面选择器的UI组件：
 * - 上一页按钮（<）
 * - 页码输入框
 * - 总页数标签
 * - 下一页按钮（>）
 */
PageSelector::PageSelector(QWidget *parent)
    : QWidget(parent), m_pageNavigation(nullptr) {
  // 创建水平布局
  QHBoxLayout *layout = new QHBoxLayout(this);

  // 创建上一页按钮
  m_previousPageButton = new QToolButton(this);
  m_previousPageButton->setText("<");
  m_previousPageButton->setEnabled(false);  // 初始状态禁用

  // 创建页码输入框
  m_pageNumberEdit = new QLineEdit(this);
  m_pageNumberEdit->setAlignment(Qt::AlignRight);  // 右对齐


  // 创建总页数显示标签
  m_pageCountLabel = new QLabel(this);
  m_pageCountLabel->setText("0");

  // 创建下一页按钮
  m_nextPageButton = new QToolButton(this);
  m_nextPageButton->setText(">");
  m_nextPageButton->setEnabled(false);  // 初始状态禁用

  // 将组件添加到布局中
  layout->addWidget(m_previousPageButton);
  layout->addWidget(m_pageNumberEdit);
  layout->addWidget(m_pageCountLabel);
  layout->addWidget(m_nextPageButton);
}

/**
 * @brief 设置页面导航对象
 * @param pageNavigation PDF页面导航对象指针
 * 
 * 将页面选择器与PDF文档的页面导航对象连接：
 * - 连接上一页按钮的点击事件到导航的goToPreviousPage()
 * - 根据是否可以向前翻页来启用/禁用上一页按钮
 * - 连接当前页面变化信号，更新页码显示
 * - 连接总页数变化信号，更新总页数显示
 * - 连接页码编辑框的编辑完成事件
 * - 连接下一页按钮的点击事件到导航的goToNextPage()
 * - 根据是否可以向后翻页来启用/禁用下一页按钮
 */
void PageSelector::setPageNavigation(QPdfPageNavigation *pageNavigation) {
  m_pageNavigation = pageNavigation;

  // 连接上一页按钮
  connect(m_previousPageButton, &QToolButton::clicked, m_pageNavigation,
          &QPdfPageNavigation::goToPreviousPage);
  connect(m_pageNavigation, &QPdfPageNavigation::canGoToPreviousPageChanged,
          m_previousPageButton, &QToolButton::setEnabled);

  // 连接当前页面变化信号
  connect(m_pageNavigation, &QPdfPageNavigation::currentPageChanged, this,
          &PageSelector::onCurrentPageChanged);
  
  // 连接总页数变化信号
  connect(
      m_pageNavigation, &QPdfPageNavigation::pageCountChanged, this,
      [this](int pageCount) {
        //m_pageCountLabel->setText(QString::fromLatin1("/ %1").arg(pageCount));
        m_pageCountLabel->setText(QString::fromLatin1("%1").arg(pageCount));
      });

  // 连接页码输入框
  connect(m_pageNumberEdit, &QLineEdit::editingFinished, this,
          &PageSelector::pageNumberEdited);

  // 连接下一页按钮
  connect(m_nextPageButton, &QToolButton::clicked, m_pageNavigation,
          &QPdfPageNavigation::goToNextPage);
  connect(m_pageNavigation, &QPdfPageNavigation::canGoToNextPageChanged,
          m_nextPageButton, &QToolButton::setEnabled);

  // 初始化当前页面显示
  onCurrentPageChanged(m_pageNavigation->currentPage());
}

/**
 * @brief 当前页面变化事件处理
 * @param page 新的当前页面索引（从0开始）
 * 
 * 更新页码输入框中显示的页面编号：
 * - 如果文档没有页面（总页数为0），显示0
 * - 否则显示实际页码（page + 1，因为内部索引从0开始）
 */
void PageSelector::onCurrentPageChanged(int page) {
  if (m_pageNavigation->pageCount() == 0)
    m_pageNumberEdit->setText(QString::number(0));
  else
    m_pageNumberEdit->setText(QString::number(page + 1));  // 显示用户友好的页码（从1开始）
}

/**
 * @brief 页码编辑完成事件处理
 * 
 * 当用户在页码输入框中输入页码并按下回车或失去焦点时触发：
 * 1. 检查输入的文本是否为有效数字
 * 2. 如果不是有效数字，恢复显示当前页码
 * 3. 如果是有效数字，跳转到指定页面：
 *    - 将用户输入的页码（从1开始）转换为内部索引（从0开始）
 *    - 使用qBound确保页码在有效范围内（0到总页数-1）
 */
void PageSelector::pageNumberEdited() {
  if (!m_pageNavigation) return;  // 确保导航对象存在

  const QString text = m_pageNumberEdit->text();

  bool ok = false;
  const int pageNumber = text.toInt(&ok);  // 尝试将输入转换为整数

  if (!ok)
    onCurrentPageChanged(m_pageNavigation->currentPage());  // 无效输入，恢复当前页码
  else
    m_pageNavigation->setCurrentPage(
        qBound(0, pageNumber - 1, m_pageNavigation->pageCount() - 1));  // 跳转到有效页面
}
