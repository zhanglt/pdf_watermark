/**
 * @file mytable.h
 * @brief 自定义表格控件头文件
 * @author PDF工具集项目组
 * @date 2024
 * 
 * 本文件定义了一个扩展的表格控件类，继承自QTableWidget。
 * 主要用于PDF文件列表管理，支持拖拽添加文件、行移动操作、
 * 页码范围设置等功能。
 * 
 * 主要特性：
 * - 支持PDF文件拖拽添加
 * - 键盘快捷键行移动（上下箭头键）
 * - 鼠标双击行选择
 * - 内嵌页码范围编辑控件
 * - 行操作按钮（上移、下移、删除）
 * - PDF文件格式验证
 */

#include "qdebug.h"
#include "qmimedata.h"
#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8
#ifndef MYTABLE_H
#define MYTABLE_H
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>

#include "function.h"

/**
 * @class mytable
 * @brief 扩展的表格控件类
 * 
 * 继承自QTableWidget，专门用于PDF文件列表管理。
 * 提供拖拽添加文件、行移动、页码设置等功能。
 */
class mytable : public QTableWidget {
  Q_OBJECT
 public:
  /**
   * @brief 构造函数
   * @param parent 父控件指针
   * 
   * 初始化自定义表格控件，启用拖拽功能。
   */
  explicit mytable(QWidget *parent = nullptr) : QTableWidget(parent) {
    setAcceptDrops(true);  // 启用拖拽功能，允许拖拽文件到表格
  }

  /**
   * @brief 设置当前操作行索引
   * @param row 行索引
   */
  void setCurrrow(int row) { currRow = row; }
  
  /**
   * @brief 获取当前操作行索引
   * @return 当前行索引
   */
  int getCurrrow() { return currRow; }
  
  /**
   * @brief 设置当前行对应的文件名
   * @param file 文件名
   */
  void setCurrFile(QString file) { currFile = file; }
  
  /**
   * @brief 获取当前行对应的文件名
   * @return 文件名
   */
  QString getCurrFile() { return currFile; }

 protected:
  /**
   * @brief 键盘按键事件处理函数
   * @param event 键盘事件对象
   * 
   * 处理键盘快捷键：
   * - 上箭头键：将当前行上移一行
   * - 下箭头键：将当前行下移一行
   * - 其他按键：调用基类处理
   */
  void keyPressEvent(QKeyEvent *event) override {
    if (event->key() == Qt::Key_Up) {
      moveRowUp(currentRow());  // 上箭头键：行上移
    } else if (event->key() == Qt::Key_Down) {
      moveRowDown(currentRow()); // 下箭头键：行下移
    } else {
      QTableWidget::keyPressEvent(event);  // 其他按键交给基类处理
    }
  }

  /**
   * @brief 拖拽进入事件处理函数
   * @param event 拖拽进入事件对象
   * 
   * 当有文件被拖拽到表格上方时触发。
   * 检查拖拽数据是否包含文件URL，如果包含则接受拖拽操作。
   */
  void dragEnterEvent(QDragEnterEvent *event) override {
    if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();  // 包含文件URL，接受拖拽事件
    }
  }

  /**
   * @brief 拖拽移动事件处理函数
   * @param event 拖拽移动事件对象
   * 
   * 当拖拽对象在表格上方移动时触发。
   * 持续检查并接受包含文件URL的拖拽操作。
   */
  void dragMoveEvent(QDragMoveEvent *event) override {
    if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();  // 包含文件URL，接受拖拽移动
    }
  }

  /**
   * @brief 拖拽释放事件处理函数
   * @param event 拖拽释放事件对象
   * 
   * 当拖拽的文件在表格上释放时触发。
   * 遍历所有拖拽的文件，验证是否为PDF格式，
   * 如果是有效的PDF文件则添加到表格中。
   */
  void dropEvent(QDropEvent *event) override {
    if (event->mimeData()->hasUrls()) {
      QList<QUrl> urls = event->mimeData()->urls();  // 获取拖拽的文件URL列表
      for (const QUrl &url : urls) {                 // 遍历每个文件
        if (url.isLocalFile()) {                     // 确保是本地文件
          QString filePath = url.toLocalFile();      // 获取本地文件路径
          // 验证文件是否为PDF格式（扩展名检查 + 文件头魔数检查）
          if (isPDFByExtension(filePath) && isPDFByMagicNumber(filePath)) {
            AddFile(filePath);  // 添加有效的PDF文件到表格
          } else {
            // 显示错误提示，文件不是PDF格式
            QMessageBox::information(
                nullptr, "提示信息！",
                filePath + " 不是PDF格式文件，不能加入合并列表");
            // return;
          }
        }
      }
      event->acceptProposedAction();  // 接受拖拽操作
    }
  }
  /**
   * @brief 鼠标双击事件处理函数
   * @param event 鼠标事件对象
   * 
   * 当用户双击表格行时触发。
   * 获取当前选中项的文本并设置为当前文件，
   * 然后发射行选择信号通知外部组件。
   */
  void mouseDoubleClickEvent(QMouseEvent *event) override {
    QTableWidgetItem *item = currentItem();  // 获取当前选中的表格项
    if (item != nullptr) {
      setCurrFile(item->text());  // 设置当前文件为选中项的文本内容
    }
    QTableWidget::mouseDoubleClickEvent(event);  // 调用基类的双击处理
    emit selectRow();  // 发射行选择信号
  }

 public slots:

  /**
   * @brief 将指定行上移一位
   * @param rowIndex 要移动的行索引
   * 
   * 将指定行与其上一行交换位置，包括表格项数据和内嵌的
   * 页码范围编辑控件的内容。如果已经是第一行则不执行操作。
   */
  void moveRowUp(int rowIndex) {
    if (rowIndex > 0 && rowIndex < rowCount()) {
      // 交换当前行和上一行的数据
      for (int col = 0; col < columnCount() - 1; ++col) {
        QTableWidgetItem *currentItem = takeItem(rowIndex, col);
        QTableWidgetItem *upperItem = takeItem(rowIndex - 1, col);
        setItem(rowIndex - 1, col, currentItem);
        setItem(rowIndex, col, upperItem);
      }
      // 获取第三列中的lineedit对象、获取其值，并进行行间交换
      QWidget *currenCell = cellWidget(rowIndex, 2);
      QWidget *upperCell = cellWidget(rowIndex - 1, 2);
      QList<QLineEdit *> currLine = currenCell->findChildren<QLineEdit *>();
      QList<QLineEdit *> upperLine = upperCell->findChildren<QLineEdit *>();
      // 保存当前行的开始、结束页码
      QString cStart = currLine[0]->text();
      QString cEnd = currLine[1]->text();
      // 设置当前行的值为 上一行的值
      currLine[0]->setText(upperLine[0]->text());
      currLine[1]->setText(upperLine[1]->text());
      // 设置上一行的值 为保存的当行含的值
      upperLine[0]->setText(cStart);
      upperLine[1]->setText(cEnd);
      // 更新当前行索引
      setCurrentCell(rowIndex - 1, currentColumn());
    }
  }
  /**
   * @brief 将指定行下移一位
   * @param rowIndex 要移动的行索引
   * 
   * 将指定行与其下一行交换位置，包括表格项数据和内嵌的
   * 页码范围编辑控件的内容。如果已经是最后一行则不执行操作。
   */
  void moveRowDown(int rowIndex) {
    // qDebug() << "当前行" << currentRow();
    // qDebug() << "rowIndex" << rowIndex;
    if (rowIndex > -1 && rowIndex < rowCount() - 1) {
      // 交换当前行和上一行的数据
      for (int col = 0; col < columnCount(); ++col) {
        QTableWidgetItem *currentItem = takeItem(rowIndex, col);
        QTableWidgetItem *upperItem = takeItem(rowIndex + 1, col);
        setItem(rowIndex + 1, col, currentItem);
        setItem(rowIndex, col, upperItem);
      }
      QWidget *currenCell = cellWidget(rowIndex, 2);
      QWidget *upperCell = cellWidget(rowIndex + 1, 2);
      QList<QLineEdit *> currLine = currenCell->findChildren<QLineEdit *>();
      QList<QLineEdit *> upperLine = upperCell->findChildren<QLineEdit *>();
      QString cStart = currLine[0]->text();
      QString cEnd = currLine[1]->text();
      currLine[0]->setText(upperLine[0]->text());
      currLine[1]->setText(upperLine[1]->text());
      upperLine[0]->setText(cStart);
      upperLine[1]->setText(cEnd);
      setCurrentCell(rowIndex + 1, currentColumn());
    }
  }
  /**
   * @brief 添加PDF文件到表格列表
   * @param fileName PDF文件的完整路径
   * 
   * 在表格末尾添加一个新行，显示PDF文件信息：
   * - 第1列：文件路径
   * - 第2列：总页数
   * - 第3列：页码范围输入控件（开始页-结束页）
   * - 第4列：操作按钮（上移、下移、删除）
   * 
   * 同时为页码范围输入框添加数据验证功能。
   */
  void AddFile(QString fileName) {
    if (!fileName.isEmpty()) {
      int rowCount = this->rowCount() - 1;
      this->insertRow(this->rowCount());  // 在最后插入行

      // 创建容器控件
      QWidget *containerBtn = new QWidget();
      QHBoxLayout *layoutBtn = new QHBoxLayout(containerBtn);
      layoutBtn->setContentsMargins(0, 0, 0, 0);  // 设置布局边距为0

      // 创建三个 QPushButton 控件，用于操作 上、下、删除操作
      QPushButton *btnDown = new QPushButton("↓");
      QPushButton *btnUp = new QPushButton("↑");
      QPushButton *btnRemove = new QPushButton("×");
      btnDown->setStyleSheet("min-width: 10px;border-width: 0px;");
      btnUp->setStyleSheet("min-width: 10px;border-width: 0px;");
      btnRemove->setStyleSheet("min-width: 10px;border-width: 0px;");
      btnDown->setFixedSize(15, 20);
      btnUp->setFixedSize(15, 20);
      btnRemove->setFixedSize(15, 20);
      // 将两个按钮添加到容器的布局中
      layoutBtn->addWidget(btnDown);
      layoutBtn->addWidget(btnUp);
      layoutBtn->addWidget(btnRemove);

      // 设置容器控件为单元格的控件
      rowCount = this->rowCount() - 1;
      //当点击按钮时设置当前行
      connect(btnDown, SIGNAL(clicked()), this, SLOT(SlotSetCurrRow()));
      // 下移一行操作
      connect(btnDown, &QPushButton::clicked, this,
              [=]() { this->moveRowDown(this->getCurrrow()); });
      connect(btnUp, SIGNAL(clicked()), this, SLOT(SlotSetCurrRow()));
      // 上移一行操作
      connect(btnUp, &QPushButton::clicked, this,
              [=]() { this->moveRowUp(this->getCurrrow()); });
      connect(btnRemove, SIGNAL(clicked()), this, SLOT(SlotSetCurrRow()));
      // 删除操作
      connect(btnRemove, &QPushButton::clicked, this,
              [=]() { this->removeRow(this->getCurrrow()); });

      QWidget *containerEdit = new QWidget();
      QHBoxLayout *layoutEdit = new QHBoxLayout(containerEdit);
      layoutEdit->setContentsMargins(0, 0, 0, 0);  // 设置布局边距为0
      // 创建两个lineedit用于保存 开始、结束页码
      QLineEdit *leStart = new QLineEdit("1");
      QLabel *label = new QLabel("-");
      QLineEdit *leEnd =
          new QLineEdit(QString::number(getPages(fileName.toStdString())));
      leStart->setFixedWidth(30);
      label->setFixedWidth(10);
      leEnd->setFixedWidth(30);
      layoutEdit->addWidget(leStart);
      layoutEdit->addWidget(label);
      layoutEdit->addWidget(leEnd);
      // 校验开始页码
      connect(leStart, &QLineEdit::editingFinished, this, [=]() {
        QString start = leStart->text();
        QString end = leEnd->text();
        QRegularExpression re("\\d+");  // 匹配一个或多个数字
        QRegularExpressionMatch match = re.match(start);

        if (!(isNumeric(start)) || (start.toInt() <= 0)) {
          leStart->setText("1");
          QMessageBox::information(nullptr, "提示信息！", "必须是大于0的数字");
        }
        if (start.toInt() > end.toInt()) {
          QString page = QString::number(getPages(fileName.toStdString()));
          leStart->setText("1");
          leEnd->setText(page);
          QMessageBox::information(nullptr, "提示信息！",
                                   "结束页数需要大于等于开始页数");
        }
      });
      // 校验结束页码
      connect(leEnd, &QLineEdit::editingFinished, this, [=]() {
        QString start = leStart->text();
        QString end = leEnd->text();
        QRegularExpression re("\\d+");  // 匹配一个或多个数字
        QRegularExpressionMatch match = re.match(start);

        if (!(isNumeric(start)) || (start.toInt() <= 0)) {
          leStart->setText("1");
          QMessageBox::information(nullptr, "提示信息！", "必须是大于0的数字");
        }
        if (start.toInt() > end.toInt()) {
          QString page = QString::number(getPages(fileName.toStdString()));
          leEnd->setText(page);
          leStart->setText("1");
          QMessageBox::information(nullptr, "提示信息！",
                                   "结束页数需要大于等于开始页数");
        }
      });

      QTableWidgetItem *itemFile = new QTableWidgetItem(fileName);
      // 设置不可编辑
      itemFile->setFlags(itemFile->flags() & ~Qt::ItemIsEditable);
      QTableWidgetItem *itemPages = new QTableWidgetItem(
          QString::number(getPages(fileName.toStdString())));
      itemPages->setFlags(itemPages->flags() & ~Qt::ItemIsEditable);

      this->setItem(rowCount, 0, itemFile);
      this->setItem(rowCount, 1,
                    itemPages);  // 将容器放置在第一行第二列
      this->setCellWidget(rowCount, 2,
                          containerEdit);  // 将容器放置在第一行第三列
      this->setCellWidget(rowCount, 3,
                          containerBtn);  // 将容器放置当前行 第四列
    }
  }

  /**
   * @brief 设置当前行的槽函数
   * 
   * 根据发送信号的按钮所在的位置，自动计算并设置当前操作行。
   * 用于处理行操作按钮（上移、下移、删除）的点击事件。
   */
  void SlotSetCurrRow() {
    QPushButton *senderButton = qobject_cast<QPushButton *>(QObject::sender());
    if (!senderButton) {
      qDebug() << "获取按钮及所在行失败:";
      return;
    }
    QModelIndex idx = this->indexAt(
        QPoint(senderButton->parentWidget()->frameGeometry().x(),
               senderButton->parentWidget()->frameGeometry().y()));
    int row = idx.row();
    this->setCurrrow(row);
  }

 signals:
  /**
   * @brief 行选择信号
   * 
   * 当用户双击表格行时发出，通知外部组件有行被选中。
   */
  void selectRow();

 public:
  /**
   * @brief 当前操作的行索引
   * 
   * 用于记录当前正在操作的行号，供行移动和删除操作使用。
   */
  int currRow;
  
  /**
   * @brief 当前选中的文件名
   * 
   * 用于记录当前选中行对应的PDF文件路径。
   */
  QString currFile;
};
#endif  // MYTABLE_H
