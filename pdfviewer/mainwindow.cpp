/**
 * @file mainwindow.cpp
 * @brief PDF工具主窗口类实现文件
 * 
 * 实现了PDF查看器的主要功能，包括：
 * - PDF文档查看和导航
 * - 水印添加和预览
 * - PDF与图片格式相互转换
 * - PDF文档拆分和合并
 * - Excel文件内容搜索
 * - 多线程处理和进度显示
 * 
 * 该类是整个应用程序的核心，协调各个功能模块的工作
 */

#include <iostream>

#include "qpainter.h"
#pragma execution_character_set("utf-8")

#include <windows.h>
#include <QDesktopServices>
#include <QDebug>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QPdfBookmarkModel>
#include <QPdfDocument>
#include <QPdfPageNavigation>
#include <QProgressDialog>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QtConcurrent/QtConcurrent>
#include <QtMath>
#include <QtSvg/QSvgGenerator>

#include "QStandardItem"
#include "function.h"
#include "include/QProgressIndicator.h"
#include "include/mark/mark.h"
#include "mainwindow.h"
#include "pageselector.h"
#include "qColordialog.h"
#include "ui_mainwindow.h"
#include "zoomselector.h"
// 全局变量声明
QWidget *horizontalLayoutWidget;  // 水平布局容器组件
QHBoxLayout *horizontalLayout;    // 水平布局管理器

// 缩放倍数常量，使用√2作为缩放步长，符合常见的缩放习惯
const qreal zoomMultiplier = qSqrt(2.0);

// 定义日志分类，用于调试输出
Q_LOGGING_CATEGORY(lcExample, "qt.examples.pdfviewer")

/**
 * @brief MainWindow构造函数
 * @param parent 父窗口指针
 * 
 * 初始化主窗口的所有组件和功能模块：
 * - PDF查看器和导航控件
 * - 工具栏和自定义控件
 * - 信号槽连接
 * - 线程池配置
 * - UI样式设置
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)                    // 调用父类构造函数
      ,
      m_zoomSelector(new ZoomSelector(this)) // 创建缩放选择器控件
      ,
      ui(new Ui::MainWindow)                 // 创建UI界面对象
      ,
      m_pageSelector(new PageSelector(this)) // 创建页面选择器控件
      ,
      m_document(new QPdfDocument(this))     // 创建PDF文档对象
      ,
      m_title(new QLabel(this))              // 创建标题标签
{
  ui->setupUi(this);  // 初始化用户界面



  // ================================
  // 工具栏配置和控件添加
  // ================================
  
  // 在放大按钮之前插入缩放选择器
  ui->mainToolBar->insertWidget(ui->actionZoom_In, m_zoomSelector);

  // 设置缩放选择器的尺寸限制
  m_zoomSelector->setMaximumWidth(400);   // 最大宽度400像素
  m_zoomSelector->setMaximumHeight(25);   // 最大高度25像素

  // 在工具栏中添加页面选择器
  ui->mainToolBar->addWidget(m_pageSelector);
  
  // 设置应用标题和样式
  m_title->setText("数科-泛生态业务组支撑工具-仅限内部使用");
  m_title->setStyleSheet(QString::fromUtf8("color: rgb(30, 144, 255);font: 12pt ;border: 0px solid #b8d4f0;"));
  ui->mainToolBar->addWidget(m_title);

  // ================================
  // 页面选择器配置
  // ================================
  
  // 设置页面选择器各组件的尺寸
  m_pageSelector->setMinimumWidth(200);                     // 整体最小宽度
  m_pageSelector->m_pageNumberEdit->setMinimumWidth(50);    // 页码输入框最小宽度
  m_pageSelector->m_pageNumberEdit->setMaximumWidth(50);    // 页码输入框最大宽度
  m_pageSelector->m_pageCountLabel->setMinimumWidth(40);    // 总页数标签最小宽度
  
  // 将页面选择器与PDF查看器的导航功能关联
  m_pageSelector->setPageNavigation(ui->pdfView->pageNavigation());

  // ================================
  // 信号槽连接 - 缩放控制
  // ================================
  
  // 连接缩放选择器和PDF查看器的信号
  connect(m_zoomSelector, &ZoomSelector::zoomModeChanged, ui->pdfView, &QPdfView::setZoomMode);
  connect(m_zoomSelector, &ZoomSelector::zoomFactorChanged, ui->pdfView, &QPdfView::setZoomFactor);
  m_zoomSelector->reset();  // 重置缩放选择器到默认状态

  // ================================
  // PDF文档查看器配置
  // ================================
  
  // 书签功能目前被注释掉，保留代码供后续使用
  // QPdfBookmarkModel *bookmarkModel = new QPdfBookmarkModel(this);
  // bookmarkModel->setDocument(m_document);
  // ui->bookmarkView->setModel(bookmarkModel);
  // connect(ui->bookmarkView, SIGNAL(activated(QModelIndex)), this, SLOT(bookmarkSelected(QModelIndex)));

  ui->tabWidget->setTabEnabled(1, true);    // 启用第二个Tab页
  ui->pdfView->setDocument(m_document);     // 将PDF文档关联到查看器

  // 连接PDF查看器的缩放因子变化信号到缩放选择器
  connect(ui->pdfView, &QPdfView::zoomFactorChanged, m_zoomSelector, &ZoomSelector::setZoomFactor);

  // ================================
  // 元类型注册和线程池配置
  // ================================
  
  // 注册自定义类型，供信号槽机制使用
  qRegisterMetaType<wMap>("wMap");                              // 水印处理结果映射类型
  qRegisterMetaType<QList<SearchResult>>("QList<SearchResult>"); // 搜索结果列表类型

  // 设置线程池的最大线程数为10，提高并发处理能力
  threadPool.setMaxThreadCount(10);

  // 当lineEditWaterText文本变动且失去焦点时候出发viewWatermark()函数用来更新文档预览
  connect(ui->lineEditWaterText, &CustomTextEdit::dataChanged, this,
          &MainWindow::viewWatermark);
  // 拖动滑块改变数值，后同步更新预览
  connect(ui->sliderOpacity, &QSlider::sliderReleased, this,
          &MainWindow::viewWatermark);
  connect(ui->sliderRotate, &QSlider::sliderReleased, this,
          &MainWindow::viewWatermark);

  connect(ui->sliderFontsize, &QSlider::sliderReleased, this,
          &MainWindow::viewWatermark);

  // 更新slider后同步更新
  // 数值显示框，这个功能可以合并到MainWindow::viewWatermark中？
  connect(ui->sliderRotate, &QSlider::valueChanged, this, [=]() {
    ui->lineEditRotate->setText(QString::number(ui->sliderRotate->value()));
  });
  connect(ui->sliderOpacity, &QSlider::valueChanged, this, [=]() {
    ui->lineEditOpacity->setText(QString::number(ui->sliderOpacity->value()));
  });
  connect(ui->sliderFontsize, &QSlider::valueChanged, this, [=]() {
    ui->lineEdit_fs->setText(QString::number(ui->sliderFontsize->value()));
  });
  // connect(m_slider, &QSlider::valueChanged, this, &MyWidget::valueChanged);
  // 检测输出目录lineedit ，保证其值必须是一个正确的文件目录
  connect(ui->lineEditOutput, &CustomLineEdit::dataChanged, this, [=]() {
    QFileInfo dir(ui->lineEditOutput->text());
    if (!dir.isDir()) {
      ui->lineEditOutput->setText("");
      QMessageBox::information(nullptr, "警告！",
                               "选择、拖入或者输入一个存在的正确目录");
    }
  });

  connect(ui->tableWidgetMerge, &mytable::selectRow, this, [=]() {
    QString file = ui->tableWidgetMerge->getCurrFile();
    QPdfDocument::DocumentError err;

    if (!file.isEmpty()) {
      this->open(QUrl::fromLocalFile(file), err);
    }
  });

  // ================================
  // 界面样式和初始化设置
  // ================================
  
  // 设置PDF查看器的背景颜色为浅灰色
  QPalette palette = ui->pdfView->palette();
  QColor color(199, 199, 199);  // RGB(199,199,199) 浅灰色
  palette.setBrush(QPalette::Dark, color);
  ui->pdfView->setPalette(palette);
  
  // 初始状态下导出PDF按钮不可用（需要先添加水印后才可用）
  ui->btnExportPDF->setEnabled(false);
  
  // 初始化文件合并表格
  initTable();
}

/**
 * @brief MainWindow析构函数
 * 清理UI资源和其他动态分配的内存
 */
MainWindow::~MainWindow() { 
  delete ui; 
}

/**
 * @brief 打开PDF文档
 * @param docLocation PDF文件的URL路径
 * @param err 返回的错误信息
 * 
 * 加载指定的PDF文件并在查看器中显示
 * 如果加载成功，会自动设置窗口标题为文档标题
 */
void MainWindow::open(const QUrl &docLocation, QPdfDocument::DocumentError &err) {
  if (docLocation.isLocalFile()) {
    // 尝试加载本地PDF文件
    QPdfDocument::DocumentError error = m_document->load(docLocation.toLocalFile());
    
    if (error != QPdfDocument::NoError) {
      err = error;
    } else {
      // 获取文档标题并设置为窗口标题
      const auto documentTitle = m_document->metaData(QPdfDocument::Title).toString();
      setWindowTitle(!documentTitle.isEmpty() ? documentTitle : "PDF浏览器");
      err = error;
    }
  } else {
    // 如果不是本地文件，显示错误信息
    QMessageBox::critical(this, tr("文件打开错误"), 
                          tr("%1 无效的本地文件").arg(docLocation.toString()));
  }
}

/**
 * @brief 书签选中事件处理
 * @param index 选中的书签索引
 * 
 * 当用户点击书签时，跳转到对应的页面
 * 注：目前书签功能被注释掉了
 */
void MainWindow::bookmarkSelected(const QModelIndex &index) {
  if (!index.isValid()) return;

  const int page = index.data(QPdfBookmarkModel::PageNumberRole).toInt();
  ui->pdfView->pageNavigation()->setCurrentPage(page);
}

/**
 * @brief 菜单或工具栏“打开文件”动作处理
 * 
 * 弹出文件选择对话框，让用户选择PDF文件并打开
 * 打开后自动设置为“适合宽度”的显示模式
 */
/**
 * @brief 打开PDF文件菜单项触发事件
 * 
 * 响应用户点击"打开"菜单项或工具栏按钮：
 * 1. 弹出文件选择对话框，筛选PDF文件
 * 2. 调用open()函数加载选中的文件
 * 3. 自动调整显示模式为适合宽度
 * 
 * @note 默认打开路径为当前输出目录
 */
void MainWindow::on_actionOpen_triggered() {
  // 弹出文件选择对话框，默认路径为输出目录
  QUrl toOpen = QFileDialog::getOpenFileUrl(
      this, tr("选择打开PDF文件"),
      QUrl("file:///" + ui->lineEditOutput->text()), "文档 (*.pdf)");

  QPdfDocument::DocumentError err;

  // 如果用户选择了文件，则打开它
  if (toOpen.isValid()) open(toOpen, err);
  
  // 设置缩放模式为适合宽度
  emit m_zoomSelector->zoomModeChanged(QPdfView::FitToWidth);
}

/**
 * @brief 退出应用程序
 */
void MainWindow::on_actionQuit_triggered() { 
  QApplication::quit(); 
}

/**
 * @brief 显示关于应用程序的信息
 */
void MainWindow::on_actionAbout_triggered() {
  QMessageBox::about(this, tr("关于pdf处理"),
                     tr("数科-泛生态业务线工具-仅限内部使用"));
}

/**
 * @brief 显示关于Qt的信息
 */
void MainWindow::on_actionAbout_Qt_triggered() { 
  QMessageBox::aboutQt(this); 
}

/**
 * @brief 放大PDF显示
 * 按照固定倍数（√2）放大当前的缩放系数
 */
void MainWindow::on_actionZoom_In_triggered() {
  ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor() * zoomMultiplier);
}

/**
 * @brief 缩小PDF显示
 * 按照固定倍数（√2）缩小当前的缩放系数
 */
void MainWindow::on_actionZoom_Out_triggered() {
  ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor() / zoomMultiplier);
}

/**
 * @brief 跳转到上一页
 */
void MainWindow::on_actionPrevious_Page_triggered() {
  ui->pdfView->pageNavigation()->goToPreviousPage();
}

/**
 * @brief 跳转到下一页
 */
void MainWindow::on_actionNext_Page_triggered() {
  ui->pdfView->pageNavigation()->goToNextPage();
}

/**
 * @brief 切换PDF的页面显示模式
 * 根据菜单项的选中状态在单页和继续页模式之间切换
 */
void MainWindow::on_actionContinuous_triggered() {
  ui->pdfView->setPageMode(ui->actionContinuous->isChecked()
                               ? QPdfView::MultiPage      // 继续页模式（多页显示）
                               : QPdfView::SinglePage);   // 单页模式
}

/**
 * @brief 水印颜色选择器事件处理
 * 
 * 弹出颜色选择对话框，让用户选择水印颜色
 * 选择后自动更新颜色显示和水印预览
 */
void MainWindow::on_btnColorSelect_clicked() {
  QColorDialog dialog(Qt::red, this);                // 创建颜色选择对话框，默认颜色为红色
  dialog.setOption(QColorDialog::ShowAlphaChannel);  // 显示透明度选项
  dialog.exec();                                     // 以模态方式显示对话框
  
  QColor color = dialog.currentColor();  // 获取用户选择的颜色
  if (color.isValid()) {
    // 提取RGB颜色分量
    int m_red = color.red();
    int m_blue = color.blue();
    int m_green = color.green();

    // 将RGB值转换为16进制字符串格式（#RRGGBB）
    QString rStr = QString("%1").arg(m_red & 0xFF, 2, 16, QChar('0'));
    QString bstr = QString("%1").arg(m_blue & 0xFF, 2, 16, QChar('0'));
    QString gStr = QString("%1").arg(m_green & 0xFF, 2, 16, QChar('0'));
    
    QString colorCode = "#" + rStr + gStr + bstr;
    ui->lineEditColor->setText(colorCode);                      // 设置颜色代码
    ui->lineEditColor->setStyleSheet("color:" + colorCode);    // 更新显示颜色

    viewWatermark();  // 更新水印预览
  }
}

// 选择输入目录
void MainWindow::on_btnSelectInput_clicked() {
  QString directory = QFileDialog::getExistingDirectory(
      nullptr, "选择输入目录", "",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  // 如果用户选择了目录，则输出所选目录的路径
  if (!directory.isEmpty()) {
    qDebug() << "选择的目录：" << directory;
    ui->lineEditInput->setText(directory);
    if (ui->lineEditOutput->text().isEmpty()) {
      ui->lineEditOutput->setText(directory);
    }
  }
}
// 选择输出目录
void MainWindow::on_btnSelectOutput_clicked() {
  QString directory = QFileDialog::getExistingDirectory(
      nullptr, "选择输出目录", "",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  // 如果用户选择了目录，则输出所选目录的路径
  if (!directory.isEmpty()) {
    // qDebug() << "选择的目录：" << directory;
    ui->lineEditOutput->setText(directory);
  }
}

// 增加水印
void MainWindow::on_btnAddWater_clicked() {
  QString text = ui->lineEditWaterText->toPlainText();
  if (text.isEmpty()) {
    QMessageBox::information(nullptr, "提示", "水印文本不能为空");
    return;
  }
  QString inputDir = ui->lineEditInput->text();
  QFileInfo dir(inputDir);
  if (inputDir.isEmpty() || !dir.isDir()) {
    QMessageBox::information(nullptr, "提示", "输入目录或者选择正确目录");
    ui->lineEditInput->setFocus();
    ui->lineEditInput->setText("");
    return;
  }
  QString outputDir = ui->lineEditOutput->text();
  QFileInfo output(outputDir);
  if (outputDir.isEmpty() || !output.isDir()) {
    QMessageBox::information(nullptr, "提示", "输入目录或者选择正确目录");
    ui->lineEditOutput->setFocus();
    ui->lineEditOutput->setText("");
    return;
  }
  QString color = ui->lineEditColor->text();
  QString opacity = ui->lineEditOpacity->text();
  QString rotate = ui->lineEditRotate->text();
  int fontIndex = ui->cBoxFont->currentIndex();
  QString fontsize = ui->lineEdit_fs->text();
  QString font;
  switch (fontIndex) {
    case 0:
      font = "NSimSun";
      break;
    case 1:
      font = "simkai";
      break;
    case 2:
      font = "simfang";
      break;
    case 3:
      font = "simhei";
      break;
    default:
      font = "simkai";
  }
  QElapsedTimer time;
  time.start();
  // 多行水印文本处理
  if (ui->lineEditWaterText->document()->blockCount() > 1) {
    if (dir.isFile() && dir.suffix().toLower() == "pdf") {  //文件处理
      // addWatermark(inputDir, outputDir + "/_out_/" + dir.fileName(), text,
      //            opacity + "%", color, "-" + rotate, font);
      addWatermark_multiline(
          inputDir.toStdString(), outputDir.toStdString() + "/_out_/", text,
          font, fontsize.toInt(), color, rotate.toDouble(), opacity.toDouble());

    } else {
      //目录处理
      addWatermarkSingle(text, inputDir, outputDir, color, opacity, rotate,
                         ui->cBoxFont->currentText(), fontsize);
    }

  } else {
    //单行水印文本处理
    if (dir.isFile() && dir.suffix().toLower() == "pdf") {  //文件处理
      addWatermark(inputDir, outputDir + "/_out_/" + dir.fileName(), text,
                   opacity + "%", color, "-" + rotate, font);
    } else {
      //目录出路
      addWatermarkSingle(text, inputDir, outputDir, color, opacity,
                         "-" + rotate, font, fontsize);
    }
  }

  // qDebug() << "总共用时:" << time.elapsed() << "毫秒";
  ui->textEditLog->append("增加水印用时:" + QString::number(time.elapsed()) +
                          "毫秒,结果文件保存在:" + ui->lineEditOutput->text() +
                          "/_out_/");
  ui->btnExportPDF->setEnabled(true);
}

/**
 * @brief 水印参数修改后的实时预览函数
 * 
 * 当用户修改水印相关参数时（文本、颜色、透明度、旋转角度、字体大小等）
 * 会触发此函数来实时更新PDF文档的水印预览效果
 * 
 * 主要功能：
 * - 获取当前水印参数设置
 * - 生成临时水印SVG文件
 * - 应用水印到当前显示的PDF页面
 * - 更新预览显示
 * 
 * @note 使用QElapsedTimer计时以监控预览生成性能
 */
void MainWindow::viewWatermark() {
  QElapsedTimer time;
  time.start();
  QString text = ui->lineEditWaterText->toPlainText();
  if (text.isEmpty()) {
    QMessageBox::information(nullptr, "提示", "水印文本不能为空");
    return;
  }
  /*
  QString inputDir = ui->lineEditInput->text();
  if (inputDir.isEmpty()) {
    QMessageBox::information(nullptr, "提示", "输入目录不能为空");
    return;
  }
  QString outputDir = ui->lineEditOutput->text();
  if (outputDir.isEmpty()) {
    QMessageBox::information(nullptr, "提示", "输出目录不能为空");
    return;
  }
*/
  QString color = ui->lineEditColor->text();
  QString opacity = ui->lineEditOpacity->text() + "%";
  QString rotate = ui->lineEditRotate->text();
  int fontIndex = ui->cBoxFont->currentIndex();
  // qDebug() << "fontindex:" << fontIndex;
  QString font;

  switch (fontIndex) {
    case 0:
      font = "NSimSun";
      break;
    case 1:
      font = "simkai";
      break;
    case 2:
      font = "simfang";
      break;
    case 3:
      font = "simhei";
      break;
    default:
      font = "simkai";
  }
  /*
      qDebug()<<"text"<<text;
      qDebug()<<"opacity"<<opacity;
      qDebug()<<"color"<<color;
      qDebug()<<"rotate"<<rotate;
      qDebug()<<"font"<<font;
  */

  // 多行文本处理
  if (ui->lineEditWaterText->document()->blockCount() > 1) {
    addWatermark_multiline(
        "doc/1.pdf", "doc/2.pdf", ui->lineEditWaterText->toPlainText(),
        ui->cBoxFont->currentText(), ui->lineEdit_fs->text().toInt(),
        ui->lineEditColor->text(), ui->lineEditRotate->text().toDouble(),
        ui->lineEditOpacity->text().toDouble() / 100);
    //由于多行文本用create_annotation处理，在预览中不显示，所以做转换处理
    pdf2image("doc/2.pdf", "doc/", 72);
    image2pdf("doc/0.png", "doc/2.pdf");
  } else {
    //单上文本处理
    addWatermark("doc/1.pdf", "doc/2.pdf", text, opacity, color, "-" + rotate,
                 font);
  }
  QPdfDocument::DocumentError err;

  this->open(QUrl::fromLocalFile("doc/2.pdf"), err);
  emit this->m_zoomSelector->zoomModeChanged(QPdfView::FitToWidth);
}
/**
 * 增加水印功能函数
 **/

void MainWindow::addWatermarkSingle(QString text, QString inputDir,
                                    QString outputDir, QString color,
                                    QString opacity, QString rotate,
                                    QString font, QString fontSize) {
  QDir dir(inputDir);
  QStringList files;
  traverseDirectory(dir, files, "pdf", "_out_");
  QMutex m_mutex;
  wMap map;
  if (files.length() == 0) {
    QMessageBox::information(nullptr, "提示！", "输入目录中文件为空");
  } else {
    for (const auto &file : files) {
      // qDebug() << "file:" << file;
      QString outfile =
          pathChange(inputDir, outputDir, file, "_out_").replace("//", "/");

      if (ui->lineEditWaterText->document()->blockCount() > 1) {
        mwmThreadSinge = new multiWatermarkThreadSingle(&m_mutex, &map);
        mwmThreadSinge->setText(text);
        mwmThreadSinge->setFont(font);
        mwmThreadSinge->setColor(color);
        mwmThreadSinge->setRotate(rotate);
        mwmThreadSinge->setOpacity(opacity);
        mwmThreadSinge->setFontsize(fontSize);
        mwmThreadSinge->setInputFilename(file);
        mwmThreadSinge->setOutputFilename(outfile);
        threadPool.start(mwmThreadSinge);
      } else {
        wmThreadSinge = new watermarkThreadSingle(&m_mutex, &map);
        /*
        connect(wmThreadSinge, &watermarkThreadSingle::addFinish, this,
                [=](wMap map) {

                              QMap<QString, int>::const_iterator i;
                              for (i = map.constBegin(); i != map.constEnd();
                     ++i)
                              {
                                  qDebug() << i.key() << ":" << i.value();
                              }
                  // qDebug() << "-------:"<< map.size();
                });*/
        wmThreadSinge->setText(text);
        wmThreadSinge->setFont(font);
        wmThreadSinge->setColor(color);
        wmThreadSinge->setRotate(rotate);
        wmThreadSinge->setOpacity(opacity);
        wmThreadSinge->setFontsize(fontSize);
        wmThreadSinge->setInputFilename(file);
        wmThreadSinge->setOutputFilename(outfile);
        threadPool.start(wmThreadSinge);
      }
    }

    threadPool.waitForDone();
    QMessageBox::information(nullptr, "提示！",
                             "增加水印完成！<br>共" +
                                 QString::number(files.length()) +
                                 "个文件参与处理，<br>实际处理文件" +
                                 QString::number(map.size()) + "个");
  }
}

/**
 * @brief 导出PDF按钮点击事件处理函数
 * 
 * 执行批量PDF水印添加操作，主要流程：
 * 1. 验证输出目录有效性
 * 2. 获取水印参数设置
 * 3. 遍历输入目录中的PDF文件
 * 4. 使用多线程为每个PDF文件添加水印
 * 5. 显示处理结果统计
 * 
 * @note 支持单行和多行水印模式，根据文本内容自动选择
 */
void MainWindow::on_btnExportPDF_clicked() {
  QString outputDir = ui->lineEditOutput->text();
  QFileInfo output(outputDir);
  if (!output.isDir()) {
    ui->lineEditOutput->setFocus();
    QMessageBox::information(nullptr, "提示", "输入目录或者选择正确目录");
    return;
  }
  QElapsedTimer time;
  time.start();
  QDir dir(ui->lineEditOutput->text() + "/_out_/");
  exportPdf(dir);
  // qDebug() << "导出图片用时:" << time.elapsed() << "毫秒";
  ui->textEditLog->append("导出图片用时:" + QString::number(time.elapsed()) +
                          "毫秒,结果文件保存在:" + ui->lineEditOutput->text() +
                          "/_pdf_/");
}

/**
 *本函数是将文字pdf转为图片pdf
 *将源目录pdfdir及子目录中的pdf文件，导出为png图片，然后再将png转换为pdf文件
 *操作过程通过Qthreadpool线程池来启用多线程处理（很恶心...）
 **/
void MainWindow::exportPdf(QDir pdfdir) {
  ui->btnAddWater->setEnabled(false);
  QStringList files;
  traverseDirectory(pdfdir, files, "pdf", "_pdf_");
  if (files.length() < 1) {
    QMessageBox::information(
        nullptr, "系统提示！",
        "目标目录：" + ui->lineEditOutput->text() + "/_out_ 中没有文件");
    ui->btnAddWater->setEnabled(true);
    return;
  }
  
  // 初始化原子计数器和总文件数
  m_completedCount.storeRelaxed(0);
  m_totalFiles = files.length();
  
  // 保存原始目录路径，避免在lambda中被修改
  QString originalPdfDirPath = pdfdir.path();
  
  for (const QString &originalFile : files) {  // 使用const引用，按值遍历
    // 使用局部变量替代成员变量，避免竞态条件
    auto* thread = new pdf2imageThreadSingle();
    // 正确处理文件路径，避免修改原始字符串
    QString file = originalFile;  // 创建副本
    //将pdf源文件的全路径中的_out_更换为__image__，并作为保存源文件转换为图片的目录
    QString imagePath = QString(file).replace("_out_", "_image_");
    //创建保存图片文件目录
    createDirectory(imagePath);
    //ui->textEditLog->append("imagePath:"   + imagePath);
    //将图片文件的全路径中的_image_更换为__pdf__
    QString pdfPath = QString(imagePath).replace("_image_", "_pdf_");
    //提取目录作为保存目标pdf文件的目录
    QFileInfo fileInfo(pdfPath);
    QString path = fileInfo.absolutePath();
    //提取文件名作为保存目标pdf文件的文件
    QString filename = fileInfo.fileName();
    createDirectory(path);
    
    // 设置线程参数
    thread->setSourceFile(file);
    thread->setImagePath(imagePath);
    thread->setTargetFile(path + "/" + filename);
    thread->setIs2pdf(true);
    thread->setResolution(ui->cBoxResolution->currentText().toInt());
    //ui->textEditLog->append("分辨率:"   + ui->cBoxResolution->currentText());

    // 使用原子计数器管理完成状态，避免多次触发完成逻辑
    connect(thread, &pdf2imageThreadSingle::addFinish, this, 
        [this, originalPdfDirPath]() {
            int completed = m_completedCount.fetchAndAddRelaxed(1) + 1;
            if (completed == m_totalFiles) {
                // 只在最后一个线程完成时执行完成逻辑
                emit this->Finished();
                ui->btnAddWater->setEnabled(true);
                QMessageBox::information(nullptr, "导出PDF完成！",
                                       "导出PDF完成！<br>共" +
                                           QString::number(m_totalFiles) +
                                           "个文件参与处理");
                
                // 获取图片根目录，转换工作完成后删除之
                QDir imageRootDir(QString(originalPdfDirPath).replace("_out_", "_image_"));
                bool success = imageRootDir.removeRecursively();
                if (success) {
                    ui->textEditLog->append("图片目录：" + imageRootDir.path() + "删除成功.");
                } else {
                    ui->textEditLog->append("图片目录：" + imageRootDir.path() + "删除失败");
                }
            }
        });

    threadPool.start(thread);

  }
  
 if (files.length() > 0) {
    qprogresssindicat();
  }

}

/**
 * @brief 字体选择下拉框变更事件
 * 
 * 当用户更改水印字体时，立即更新预览效果
 */
void MainWindow::on_cBoxFont_currentIndexChanged() {
    viewWatermark();
}

/**
 * @brief 选择图片文件按钮点击事件
 * 
 * 弹出文件选择对话框，让用户选择要转换为PDF的图片文件
 * 支持的格式：PNG、JPG、BMP
 */
void MainWindow::on_btnSelectImageFile_clicked() {
  QString filePath = QFileDialog::getOpenFileName(
      nullptr, "选择图像文件", QDir::homePath(),
      "图像文件 (*.png *.PNG *.jpg *.JPG *.bmp *.BMP)");
  if (!filePath.isEmpty()) {
    ui->lineEditImageFile->setText(filePath);
  }
}
/**
 * @brief 图片转PDF按钮点击事件
 * 
 * 将用户选择的图片文件转换为PDF格式：
 * 1. 验证输入文件的有效性（扩展名和文件头检查）
 * 2. 调用image2pdf函数执行转换
 * 3. 转换成功后自动打开生成的PDF文件
 * 4. 更新日志和状态信息
 * 
 * @note 转换后的PDF文件保存在原图片文件同目录，文件名为"原名.pdf"
 */
void MainWindow::on_btnTransform_clicked() {
  QString file = ui->lineEditImageFile->text();
  QPdfDocument::DocumentError err;

  if (!isImageByExtension(file) || !isImageByMagicNumber(file) || file == "") {
    QMessageBox::information(nullptr, "提示信息！", "请选择正确的图像文件");
    ui->lineEditImageFile->setText("");
    return;
  }
  QFileInfo info(ui->lineEditImageFile->text());
  if (image2pdf(info.filePath().toStdString(),
                (info.filePath() + ".pdf").toStdString()) == 0) {
    ui->textEditLog->append("文件保存在：" + info.filePath() + ".pdf\n");
    this->open(QUrl::fromLocalFile(info.filePath() + ".pdf"), err);
    emit this->m_zoomSelector->zoomModeChanged(QPdfView::FitToWidth);
    ui->textEditLog->append("PDF转换完成保存在：" + info.filePath() + ".pdf");
    QMessageBox::information(nullptr, "PDF转换完成！",
                             "文件保存在：" + info.filePath() + ".pdf\n");
  }
}

// 选择图片目录
void MainWindow::on_btnSelectImageDir_clicked() {
  QString directory = QFileDialog::getExistingDirectory(
      nullptr, "选择图片目录", "",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!directory.isEmpty()) {
    // qDebug() << "选择的目录：" << directory;
    ui->lineEditImageDir->setText(directory);
  }
}

// 批量转换
void MainWindow::on_btnTransformBat_clicked() {
  if (ui->lineEditImageDir->text() == "") {
    QMessageBox::information(nullptr, "提示信息！", "请选择图像目录");
    return;
  }
  QFileInfo file(ui->lineEditImageDir->text());
  if (!file.isDir()) {
    QMessageBox::information(nullptr, "提示信息！", "请选择正确的图像目录");
    ui->lineEditImageDir->setText("");
    return;
  }
  QDir dir(ui->lineEditImageDir->text());
  QStringList files;
  tDirectory(dir, files, {"png", "jpg", "bmp"});
  for (QString &file : files) {
    image2pdf(file.toStdString(), (file + ".pdf").toStdString());
  }
  QMessageBox::information(nullptr, "PDF转换完成！", "PDF保存在图片源目录中");
  ui->textEditLog->append("PDF保存源目录中:" + ui->lineEditImageDir->text());
}

/**
 * @brief PDF转图片模块-选择PDF文件按钮点击事件
 * 
 * 让用户选择要转换为图片的PDF文件：
 * 1. 弹出文件选择对话框，筛选PDF文件
 * 2. 设置选中的文件路径到界面
 * 3. 自动在预览区域打开选中的PDF文件
 */
void MainWindow::on_btnSelectPDFFile_clicked() {
  QString filePath = QFileDialog::getOpenFileName(
      nullptr, "选择PDF文件", QDir::homePath(), "PDF文件 (*.pdf *.PDF)");
  if (!filePath.isEmpty()) {
    qDebug() << "选择的目录：" << filePath;
    ui->lineEditPdfFile->setText(filePath);
  }
  QPdfDocument::DocumentError err;

  this->open(QUrl::fromLocalFile(filePath), err);
}
/**
 * @brief PDF转图片操作按钮点击事件
 * 
 * 执行PDF到图片的转换操作：
 * 1. 验证选择的PDF文件有效性
 * 2. 创建专用的转换线程处理转换任务
 * 3. 生成包含特殊标识的输出目录
 * 4. 使用多线程方式执行转换以避免界面阻塞
 * 
 * @note 输出图片文件保存在原PDF文件目录下的专用文件夹中
 */
void MainWindow::on_btnPdfToImage_clicked() {
  QString file = ui->lineEditPdfFile->text();
  if (file == "") {
    QMessageBox::information(nullptr, "提示信息！", "请选择PDF文件");
    return;
  }
  if (!isPDFByExtension(file) || !isPDFByMagicNumber(file)) {
    QMessageBox::information(nullptr, "提示信息！", "请选择正确的PDF文件");
    ui->lineEditPdfFile->setText("");
    return;
  }
  QString path = file.left(file.length() - 4) + "_YXNkZmRzZmFzZGZhZHNmYWRzZgo";
  pdf2imageThread = new pdf2imageThreadSingle();
  pdf2imageThread->setSourceFile(file);
  pdf2imageThread->setIs2pdf(false);
  //通过pdf文件目录，生成图片目录，并创建。
  qDebug() << "file:" << path;
  if (!createDirectory(path)) {
  }
  pdf2imageThread->setImagePath(path);
  pdf2imageThread->setResolution(100);
  //为每个线程连接一个信号，用于监测线程池的线程工作完成情况，建议Qthreadpoll增加一个finish信号。
  connect(pdf2imageThread, &pdf2imageThreadSingle::addFinish, this, [=]() {
    // 判断线程池中的活动线程数，如果为0则认为所有工作线程结束。
    if (threadPool.activeThreadCount() == 0) {
      //将增加水印按钮设置为可用状态
      emit this->Finished();
      ui->textEditLog->append("图片保存目录:" + path);
      QMessageBox::information(nullptr, "导出图片完成！", "导出图片完成！");
    }
  });

  threadPool.start(pdf2imageThread);
  qprogresssindicat();
}
// 长时间操作动画延时函数
void MainWindow::qprogresssindicat() {
  QProgressIndicator *pIndicator = nullptr;
  QDialog dialog;
  QObject::connect(this, &MainWindow::Finished, &dialog, &QDialog::close);
  //转换开始 增加水印按钮设置为不可用
  dialog.setWindowTitle("文件转换处理...");
  dialog.resize(200, 20);
  // /dialog.setWindowFlags(Qt::CustomizeWindowHint |
  // Qt::WindowCloseButtonHint);
  dialog.setWindowFlags(dialog.windowFlags() | Qt::FramelessWindowHint);
  pIndicator = new QProgressIndicator(&dialog);
  QVBoxLayout *hLayout = new QVBoxLayout(&dialog);  // 水平布局
  QLabel *l1 = new QLabel(
      "正在转换。。。。"
      ""
      "");
  hLayout->setMargin(1);           // 与窗体边无距离 尽量占满
  hLayout->addWidget(pIndicator);  // 加入控件
  hLayout->addWidget(l1);
  hLayout->setAlignment(pIndicator, Qt::AlignCenter);  // 控件居中
  hLayout->setAlignment(l1, Qt::AlignCenter);          // 控件居中
  // ui->tab_2->setLayout(hLayout);
  dialog.setLayout(hLayout);

  pIndicator->setColor(QColor(12, 52, 255));
  pIndicator->startAnimation();  //启动动画
  dialog.exec();
}
// 选择拆分pdf文件
void MainWindow::on_btnSelectFilesplit_clicked() {
  QPdfDocument::DocumentError err;
  QString fileName = QFileDialog::getOpenFileName(
      nullptr, "选择PDF文件", QDir::homePath(), "PDF文件 (*.pdf *.PDF)");
  if (!fileName.isEmpty()) {
    ui->lineEditInputFilesplit->setText(fileName);
  }
  if (fileName.length() > 0) {
    this->open(QUrl::fromLocalFile(fileName), err);
    if (err != QPdfDocument::NoError) {
      QMessageBox::information(nullptr, "警告！", "请选择正确的pdf文件");
    } else {
      ui->lineEditSplitpages->setText(
          QString::number(getPages(fileName.toStdString())));
      ui->lineEditSplitEnd->setText(
          QString::number(getPages(fileName.toStdString())));
      QFileInfo path(fileName);
      ui->lineEditSplitOutput->setText(path.absolutePath());
    }
  }
}
// 拆分pdf的文件检查
void MainWindow::on_lineEditInputFilesplit_textChanged(
    const QString &filename) {
  QPdfDocument::DocumentError err;

  // 判断是否为pdf文件。
  QFileInfo fileInfo(filename);
  if ((fileInfo.isFile()) && (filename.contains(".pdf"))) {
    this->open(QUrl::fromLocalFile(filename), err);
    if (err != QPdfDocument::NoError) {
      QMessageBox::information(nullptr, "警告！", "目标文件错误");

    } else {
      ui->lineEditSplitpages->setText(
          QString::number(getPages(filename.toStdString())));
      ui->lineEditSplitEnd->setText(
          QString::number(getPages(filename.toStdString())));
      QFileInfo path(filename);
      ui->lineEditSplitOutput->setText(path.absolutePath());
    }

  } else {
    if (!filename.isEmpty()) {
      QMessageBox::information(nullptr, "警告！", "请选择正确的pdf文件");
    }
    ui->lineEditInputFilesplit->setText("");
    return;
  }
}
// 选择拆分目录
void MainWindow::on_btnSelectSplitDir_clicked() {
  QString directory = QFileDialog::getExistingDirectory(
      nullptr, "选择输出目录", "",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  // 如果用户选择了目录，则输出所选目录的路径
  if (!directory.isEmpty()) {
    // qDebug() << "选择的目录：" << directory;
    ui->lineEditSplitOutput->setText(directory);
  }
}
// 拆分目录进行有效性检查
void MainWindow::on_lineEditSplitOutput_textChanged(const QString &filepath) {
  QFileInfo path(filepath);
  if (!path.isDir()) {
    if (!filepath.isEmpty()) {
      QMessageBox::information(nullptr, "警告！", "请选择正确的目录");
    }
    ui->lineEditSplitOutput->setText("");
    return;
  }
}

//文件拆分操作
/**
 * @brief PDF拆分按钮点击事件
 * 
 * 将PDF文件按指定页数拆分成多个文件：
 * 1. 验证输入文件和输出目录的有效性
 * 2. 获取拆分页数设置
 * 3. 调用splitPdf函数执行拆分操作
 * 4. 在输出目录生成多个拆分后的PDF文件
 * 
 * @note 拆分后的文件命名格式为：原文件名_1.pdf、原文件名_2.pdf...
 */
void MainWindow::on_btnSplitPdf_clicked() {
  if ((ui->lineEditInputFilesplit->text() == "") ||
      (ui->lineEditSplitOutput->text() == "")) {
    QMessageBox::information(nullptr, "警告！", "请选择输入文件、输出目录");
    return;
  }
  // 文件页数
  int pages = ui->lineEditSplitpages->text().toInt();
  string in = ui->lineEditInputFilesplit->text().toStdString();
  QFileInfo file(ui->lineEditInputFilesplit->text());
  string filename =
      file.fileName().left(file.fileName().length() - 4).toStdString();
  string out = ui->lineEditSplitOutput->text().toStdString() + "/" + filename;

  //通过radiobutton来选择两种拆分方式
  if (ui->radioButtonSpliterange->isChecked()) {
    //第一种拆分方式：从原始文件中提取区间页数为新文件
    QString range, sub_range;
    QStringList parts, sub_parts;
    int start = 0, end = 0;
    range = ui->lineEditSplitscope->text();
    // 通过正则表达式来验证拆分规则的有效性
    QRegularExpression regex("^\\d+-\\d+(,\\d+-\\d+)*$");
    QRegularExpressionMatch match = regex.match(range);
    if (match.hasMatch()) {
      if (regex.match(range).hasMatch()) {
        parts = range.split(",");
        for (int i = 0; i < parts.size(); ++i) {
          sub_range = parts.at(i);
          sub_parts = sub_range.split("-");
          start = sub_parts[0].toInt() - 1;
          end = sub_parts[1].toInt();
          qDebug() << "start:" << start;
          qDebug() << "end:" << end;
          if (end > pages) {
            QMessageBox::information(nullptr, "警告！",
                                     "拆分范围不能大于文档总页数");
            return;
          }
          if (start > end) {
            QMessageBox::information(nullptr, "警告！",
                                     "拆分范围开始值不能大于结束值");
            return;
          }
        }
      }
    } else {
      QMessageBox::information(nullptr, "警告！",
                               "目前只支持n-n,n-n...格式的拆分");
      return;
    }

    if (regex.match(range).hasMatch()) {
      parts = range.split(",");
      for (int i = 0; i < parts.size(); ++i) {
        sub_range = parts.at(i);
        sub_parts = sub_range.split("-");
        start = sub_parts[0].toInt() - 1;
        end = sub_parts[1].toInt();
        string sub_out = out + "_" + sub_parts[0].toStdString() + "-" +
                         sub_parts[1].toStdString();
        splitPdf(in, sub_out, start, end);
      }
    }

    QMessageBox::information(
        nullptr, "PDF拆分完成！",
        "文件保存在保存：" + ui->lineEditSplitOutput->text());

    ui->textEditLog->append("拆分PDF保存目录中:" +
                            ui->lineEditSplitOutput->text());

  } else {
    /*第二种拆分方式：
    先按照拆分范围生成一个临时文件保存在临时文件夹
    再将临时文件按照规则拆分成多个文件
    */
    int start = ui->lineEditSplitStart->text().toInt() - 1;
    int end = ui->lineEditSplitEnd->text().toInt();
    if ((end > pages) || (start > pages)) {
      QMessageBox::information(nullptr, "警告！", "拆分范围不能大于文档总页数");
      return;
    }
    if (start > end) {
      QMessageBox::information(nullptr, "警告！",
                               "拆分范围开始值不能大于结束值");
      return;
    }
    QString tempDir =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    // 第一次拆分生成中间结果文件
    splitPdf(in, tempDir.toStdString() + "/" + filename, start, end);
    // 每个拆分子文件的页数
    int subPages = ui->lineEditSubPages->text().toInt();
    // 第二次拆分
    splitPdf(tempDir.toStdString() + "/" + filename + "_split.pdf", out,
             subPages);
    QFile tempfile(tempDir + "/" + QString::fromStdString(filename) +
                   "_split.pdf");
    tempfile.remove();
    QMessageBox::information(
        nullptr, "PDF拆分完成！",
        "文件保存在保存：" + ui->lineEditSplitOutput->text());

    ui->textEditLog->append("拆分PDF保存目录中:" +
                            ui->lineEditSplitOutput->text());
  }
}

// 初始化文件合并表格
void MainWindow::initTable() {
  // 设置表头
  QStringList headerLabels;
  headerLabels << "文件名称"
               << "页数"
               << "范围"
               << "文件操作";
  ui->tableWidgetMerge->setHorizontalHeaderLabels(headerLabels);
  ui->tableWidgetMerge->setColumnWidth(0, 150);
  ui->tableWidgetMerge->setColumnWidth(1, 45);
  ui->tableWidgetMerge->setColumnWidth(2, 85);
  ui->tableWidgetMerge->setColumnWidth(3, 120);

}

// 增加并文件列表
void MainWindow::on_btnAddFile_clicked() {
  QString fileName = QFileDialog::getOpenFileName(
      nullptr, "选择PDF文件", QDir::homePath(), "PDF文件 (*.pdf *.PDF )");
  if (fileName.isEmpty()) {
    QMessageBox::information(nullptr, "提示信息！", "文件名为空");
    return;
  }
  ui->tableWidgetMerge->AddFile(fileName);
}

// 选择输出文件目录
void MainWindow::on_btnSelectMergeDir_clicked() {
  QString directory = QFileDialog::getExistingDirectory(
      nullptr, "选择输出目录", "",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  // 如果用户选择了目录，则输出所选目录的路径
  if (!directory.isEmpty()) {
    // qDebug() << "选择的目录：" << directory;
    ui->lineEditOutMerge->setText(directory);
  }
}

// PDF合并操作
/**
 * @brief PDF合并按钮点击事件
 * 
 * 将多个PDF文件合并为一个文件：
 * 1. 验证输出目录和文件名设置
 * 2. 从表格中获取要合并的文件列表
 * 3. 创建临时目录处理中间文件
 * 4. 调用mergePdf函数执行合并操作
 * 5. 生成最终的合并PDF文件
 * 
 * @note 合并顺序按照表格中文件的显示顺序
 */
void MainWindow::on_btnMerge_clicked() {
  // 临时文件列表
  std::list<std::string> fileList;
  QString outDir = ui->lineEditOutMerge->text();
  QString outfileName = ui->lineEditMergeOutFile->text();
  // 合并前的临时文件目录
  QString tempFile = outDir + "/" + "YXNkZmRzZmFzZGZhZHNmYWRzZgo";
  if (outfileName == "") {
    QMessageBox::information(nullptr, "提示信息！", "请输入合并后的文件名");
    ui->lineEditMergeOutFile->setFocus();
    return;
  }
  if (outDir == "") {
    QMessageBox::information(nullptr, "提示信息！", "合并后的输出目录");
    ui->lineEditOutMerge->setFocus();
    return;
  }

  if (outfileName.right(4).toUpper() != ".PDF") {
    outfileName = outfileName + ".pdf";
  }
  // 创建临时目录
  if (!createDirectory(tempFile)) {
    QMessageBox::information(nullptr, "提示信息！", "请选择正确的输出目录");
    ui->lineEditOutMerge->setFocus();
    return;
  }

  QString infilename;
  for (int row = 0; row < ui->tableWidgetMerge->rowCount(); ++row) {
    // 从第一列中获取原始文件名
    QTableWidgetItem *item = ui->tableWidgetMerge->item(row, 0);
    if (item) {
      infilename = item->text();
    } else {
      qDebug() << "行:" << row << ", 列:" << 0 << ", 文本: (Empty)";
      return;
    }
    int pStart, pEnd;
    // 从第三列中获取 承载开始行 结束行的lineedit对象，并获取开始、结束页码
    QWidget *currenCell = ui->tableWidgetMerge->cellWidget(row, 2);
    QList<QLineEdit *> currLine = currenCell->findChildren<QLineEdit *>();
    pStart = currLine[0]->text().toInt();
    pEnd = currLine[1]->text().toInt();
    // 按行拆到临时文件
    if (splitPdf(infilename.toStdString(),
                 tempFile.toStdString() + "/" + std::to_string(row), pStart - 1,
                 pEnd) == 0) {
      //临时文件放到列表中
      fileList.push_back(tempFile.toStdString() + "/" + std::to_string(row) +
                         "_split.pdf");
    }
  }
  outfileName = outDir + "/" + outfileName;
  // 将临时文件列表中的文件进行合并
  if (mergePdf(fileList, outfileName.toStdString()) == 0) {
    QFileInfo info(outfileName);
    QPdfDocument::DocumentError err;

    // 在预览中打开合并完成的文件
    this->open(QUrl::fromLocalFile(outfileName), err);
    emit this->m_zoomSelector->zoomModeChanged(QPdfView::FitToWidth);
    ui->textEditLog->append("PDF合并完成保存在：" + info.filePath());
    QMessageBox::information(nullptr, "PDF合并完成！",
                             "文件保存在：" + info.filePath() + "\n");
    QDir temp(tempFile);
    // 删除整个目录及其子目录
    temp.removeRecursively();
  }
}

void MainWindow::on_lineEditInputFilesplit_editingFinished() {}



// excle搜索目录设置
void MainWindow::on_btnSelectInput_Search_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(
        nullptr, "选择输入目录", "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // 如果用户选择了目录，则输出所选目录的路径
    if (!directory.isEmpty()) {
      qDebug() << "选择的目录：" << directory;
      ui->lineEditInput_Search->setText(directory);
      QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
      // 在临时文件夹创建一个文件,用于保存搜索目录
      QString testFilePath = tempPath + "/search.txt";

      QFile file(testFilePath);
      if (file.exists() && file.remove()) {
          qDebug() << "文件删除成功";
      }
      if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {

          QTextStream out(&file);
          out << directory;
          file.close();
          qDebug() << "文件已保存到:" << testFilePath;

      } else {
          qDebug() << "无法创建文件!";
      }

    }





}

// 切换到搜索tab时，填充搜索框
void MainWindow::on_tabWidget_currentChanged(int index)
{
  if(index==4) {
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString testFilePath = tempPath + "/search.txt";
    QFile searchfile(testFilePath);
    if (!searchfile.exists()) {
          qDebug() << "文件不存在:" << testFilePath;
      }else{
        if (!searchfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
               qDebug() << "无法打开文件:" << searchfile;
           }else{

           // 使用 QTextStream 读取文件内容
           QTextStream in(&searchfile);
           QString content = in.readAll();  // 读取全部内容
           ui->lineEditInput_Search->setText(content);
           searchfile.close();
        }

    }
}
}

// 搜索完成槽函数
void MainWindow::onSearchFinished(const QList<SearchResult> &results)
{
    qDebug() << "搜索完成，结果数量:" << results.size();
    
    QString currentFile;
    QTreeWidgetItem *fileItem = nullptr;

    for (const SearchResult &result : results) {
        // 当切换到新文件时，创建新的文件节点
        if (result.fileName != currentFile) {
            currentFile = result.fileName;
            fileItem = new QTreeWidgetItem(ui->treeWidget_Search);
            fileItem->setText(0, "文件: " + result.fileName);
            fileItem->setExpanded(true); // 展开文件节点
        }

        // 创建结果子节点
        QTreeWidgetItem *resultItem = new QTreeWidgetItem(fileItem);
        resultItem->setText(0, "" + result.sheetName);
        resultItem->setText(1, result.cellReference);
        resultItem->setText(2, result.cellValue.toString());
    }
    
    ui->treeWidget_Search->show();
    
    if (results.isEmpty()) {
        QMessageBox::information(this, "搜索结果", "未找到匹配的结果");
    } else {
        QMessageBox::information(this, "搜索完成", 
                                QString("搜索完成！找到 %1 个匹配结果").arg(results.size()));
    }
}

// 搜索进度槽函数
void MainWindow::onSearchProgress(int processed, int total, const QString &currentFileName)
{
    qDebug() << "搜索进度--------------:" << processed << "/" << total << ", 当前处理文件:" << currentFileName;
    ui->textEditLog->append("文件名称:"+currentFileName);
    ui->textEditLog->append("搜索进度---：<"+QString::number(processed) + "/" + QString::number(total)+ ">" );
    // 这里可以添加进度条更新逻辑
}

// 搜索错误槽函数
void MainWindow::onSearchError(const QString &error)
{
    qDebug() << "搜索错误:" << error;
    QMessageBox::warning(this, "搜索错误", error);
}

// 检查树控件是否为空
bool MainWindow::isTreeWidgetEmpty(QTreeWidget *treeWidget) {
    return treeWidget->topLevelItemCount() == 0;
}

// 导出搜索结果到Excel
QString MainWindow::exportTreeWidgetToExcel(QTreeWidget &treeWidget, const QString &exportFilePath)
{

    Document exportDoc;
    // 设置数据行格式
    Format dataFormat;
    dataFormat.setBorderStyle(Format::BorderThin);

    Format hyperlinkFormat;
        // 设置超链接的字体颜色（通常是蓝色）
        hyperlinkFormat.setFontColor(QColor(0, 0, 255));  // 蓝色
        // 设置下划线（超链接通常有下划线）
        //hyperlinkFormat.setFontUnderline(hyperlinkFormat.FontUnderlineSingle);
        // 设置边框（可选）
        hyperlinkFormat.setBorderStyle(Format::BorderThin);

    // 设置Excel文件的表头
    exportDoc.write(1, 1, "源文件");
    exportDoc.write(1, 2, "工作表");
    exportDoc.write(1, 3, "单元格位置");
    exportDoc.write(1, 4, "单元格内容");

    // 设置表头格式
    Format headerFormat;
    headerFormat.setFontBold(true);
    headerFormat.setPatternBackgroundColor(QColor(200, 200, 200));
    headerFormat.setBorderStyle(Format::BorderThin);

    for (int col = 1; col <= 4; ++col) {
        exportDoc.write(1, col, exportDoc.read(1, col), headerFormat);
    }

    int currentRow = 2; // 从第2行开始写数据

    // 遍历树控件的所有顶级项目（文件节点）
    for (int i = 0; i < treeWidget.topLevelItemCount(); ++i) {
        QTreeWidgetItem *fileItem = treeWidget.topLevelItem(i);
        QString fileName = fileItem->text(0);

        // 移除"文件: "前缀，只保留文件名
        if (fileName.startsWith("文件: ")) {
            fileName = fileName.mid(4);
        }

        // 遍历该文件下的所有子项目（搜索结果）
        for (int j = 0; j < fileItem->childCount(); ++j) {
            QTreeWidgetItem *resultItem = fileItem->child(j);

            QString sheetName = resultItem->text(0);
            // 移除"工作表: "前缀，只保留工作表名
            if (sheetName.startsWith("工作表: ")) {
                sheetName = sheetName.mid(5);
            }

            QString cellReference = resultItem->text(1);
            QString cellValue = resultItem->text(2);

            // 写入数据到Excel
            exportDoc.currentWorksheet()->writeHyperlink(currentRow, 1,QUrl::fromLocalFile(fileName),hyperlinkFormat,fileName);
            exportDoc.write(currentRow, 1, fileName);
            exportDoc.write(currentRow, 2, sheetName);
            exportDoc.write(currentRow, 3, cellReference);
            exportDoc.write(currentRow, 4, cellValue);



            for (int col = 2; col <= 4; ++col) {
                exportDoc.write(currentRow, col, exportDoc.read(currentRow, col), dataFormat);
            }

            currentRow++;
        }
    }

    // 保存Excel文件
    if (exportDoc.saveAs(exportFilePath)) {
        return "搜索结果已成功导出到:" + exportFilePath;
    } else {
        return "导出失败!";
    }
}
// 在指定目录中搜索所有excle文件
/**
 * @brief Excel文件搜索按钮点击事件
 * 
 * 在指定目录中搜索Excel文件内容：
 * 1. 验证搜索关键词和目录的有效性
 * 2. 创建搜索线程进行后台处理
 * 3. 连接进度更新和结果处理信号
 * 4. 启动搜索任务并更新界面状态
 * 
 * @note 支持在Excel文件的所有工作表中搜索指定关键词
 */
void MainWindow::on_btnSearch_clicked()
{

    QString key = ui->lineEditInput_Search_Key->text();
    QString inputDir= ui->lineEditInput_Search->text();
    QDir dir(inputDir);
    if(!dir.exists()){
        QMessageBox::information(nullptr, "提示", "搜索目录不存在,请重新选择");
        return;
    }

    if (key.isEmpty()||inputDir.isEmpty()) {
      QMessageBox::information(nullptr, "提示", "搜索目录、搜索关键字不能为空");
      return;
    }
    ui->treeWidget_Search->clear();
    ui->treeWidget_Search->setHeaderLabels(QStringList() << "文件/工作表" << "单元格" << "内容");
    ui->treeWidget_Search->setColumnWidth(0, 300);
    ui->treeWidget_Search->setColumnWidth(1, 50);



    QStringList fileNames ;
    traverseDirectory(dir, fileNames, "xlsx", "_out_");

    // 创建搜索线程
    SearchThread *searchTask = new SearchThread();
    searchTask->setFileNames(fileNames);
    searchTask->setSearchText(key);

    // 连接信号
    connect(searchTask, &SearchThread::searchFinished, this, &MainWindow::onSearchFinished);
    connect(searchTask, &SearchThread::searchProgress, this, &MainWindow::onSearchProgress);
    connect(searchTask, &SearchThread::searchError, this, &MainWindow::onSearchError);

    // 启动线程
    QThreadPool::globalInstance()->start(searchTask);

    qDebug() << "搜索任务已提交到线程池";
}

//搜索结果导出到excel文件
void MainWindow::on_btnSearch_export_clicked()
{
    if (isTreeWidgetEmpty(ui->treeWidget_Search)) {
           QMessageBox::information(nullptr, "数据导出", "搜索结果数据为空");
       } else
    {
    QString fileName = QFileDialog::getSaveFileName(this, "保存文件", "c:/search.xlsx", "Excel Files (*.xlsx)");
    QString result;
       if (!fileName.isEmpty()) {
           result=exportTreeWidgetToExcel(*ui->treeWidget_Search, fileName);
           if (!result.isNull()){
               int reply = QMessageBox::question(
                      nullptr,
                      "导出结果",
                      "打开导出文件吗？",
                      QMessageBox::Yes | QMessageBox::No,QMessageBox::No
                  );
                  // 判断用户选择
                  if (reply == QMessageBox::Yes) {
                      QUrl url = QUrl::fromLocalFile(fileName);
                        // 打开目录或文件
                       QDesktopServices::openUrl(url);
                  }

           }else{
               QMessageBox::information(nullptr,"搜索结果导出","导出失败");
           }
        }
    }
}

void MainWindow::on_lineEditInput_Search_Key_returnPressed()
{
on_btnSearch_clicked();
}
