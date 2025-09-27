/**
 * @file mainwindow_refactored.cpp
 * @brief 重构后的主窗口类实现
 * 
 * 简化的MainWindow实现，将具体功能委托给各控制器处理
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

// 包含控制器头文件
#include "src/controllers/PdfViewerController.h"
#include "src/controllers/WatermarkController.h"
#include "src/controllers/PdfConverterController.h"
#include "src/controllers/PdfSplitMergeController.h"
#include "src/controllers/ExcelSearchController.h"

#include <QMessageBox>
#include <QPdfDocument>
#include "include/textedit/CustomTextEdit.h"
#include "include/lineedit/CustomLineEdit.h"

#pragma execution_character_set("utf-8")

/**
 * @brief 构造函数
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_titleLabel(new QLabel(this))
{
    ui->setupUi(this);
    
    // 初始化UI
    initializeUI();
    
    // 创建控制器
    createControllers();
    
    // 连接控制器信号
    connectControllers();
    
    // 连接UI信号到控制器
    connectUIToControllers();
    
    // 设置工具栏
    setupToolBar();
}

/**
 * @brief 析构函数
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief 初始化UI
 */
void MainWindow::initializeUI()
{
    // 设置线程池
    m_threadPool.setMaxThreadCount(10);
    
    // 注册元类型
    qRegisterMetaType<wMap>("wMap");
    qRegisterMetaType<QList<SearchResult>>("QList<SearchResult>");
    
    // 启用第二个Tab页
    ui->tabWidget->setTabEnabled(1, true);
}

/**
 * @brief 创建控制器
 */
void MainWindow::createControllers()
{
    // 创建PDF查看控制器
    m_pdfViewerController = new PdfViewerController(ui, this);
    m_pdfViewerController->initialize();
    
    // 创建水印控制器
    m_watermarkController = new WatermarkController(ui, m_threadPool, this);
    m_watermarkController->initialize();
    
    // 创建PDF转换控制器
    m_pdfConverterController = new PdfConverterController(ui, m_threadPool, this);
    m_pdfConverterController->initialize();
    
    // 创建PDF拆分合并控制器
    m_splitMergeController = new PdfSplitMergeController(ui, this);
    m_splitMergeController->initialize();
    
    // 创建Excel搜索控制器
    m_excelSearchController = new ExcelSearchController(ui, this);
    m_excelSearchController->initialize();
}

/**
 * @brief 连接控制器信号
 */
void MainWindow::connectControllers()
{
    // PDF查看控制器信号
    connect(m_pdfViewerController, &PdfViewerController::documentOpened,
            this, &MainWindow::onDocumentOpened);
    
    // 水印控制器信号
    connect(m_watermarkController, &WatermarkController::requestOpenPdf,
            [this](const QUrl &url) {
                QPdfDocument::DocumentError err;
                m_pdfViewerController->open(url, err);
            });
    
    connect(m_watermarkController, &WatermarkController::requestFitToWidth,
            [this]() {
                m_pdfViewerController->zoomSelector()->zoomModeChanged(QPdfView::FitToWidth);
            });
    
    connect(m_watermarkController, &WatermarkController::exportButtonStateChanged,
            [this](bool enabled) {
                ui->btnExportPDF->setEnabled(enabled);
            });
    
    // PDF转换控制器信号
    connect(m_pdfConverterController, &PdfConverterController::requestOpenPdf,
            [this](const QUrl &url) {
                QPdfDocument::DocumentError err;
                m_pdfViewerController->open(url, err);
            });
    
    connect(m_pdfConverterController, &PdfConverterController::requestFitToWidth,
            [this]() {
                m_pdfViewerController->zoomSelector()->zoomModeChanged(QPdfView::FitToWidth);
            });
    
    connect(m_pdfConverterController, &PdfConverterController::logMessage,
            this, &MainWindow::updateLog);
    
    // PDF拆分合并控制器信号
    connect(m_splitMergeController, &PdfSplitMergeController::requestOpenPdf,
            [this](const QUrl &url) {
                QPdfDocument::DocumentError err;
                m_pdfViewerController->open(url, err);
            });
    
    connect(m_splitMergeController, &PdfSplitMergeController::requestFitToWidth,
            [this]() {
                m_pdfViewerController->zoomSelector()->zoomModeChanged(QPdfView::FitToWidth);
            });
    
    connect(m_splitMergeController, &PdfSplitMergeController::logMessage,
            this, &MainWindow::updateLog);
    
    // Excel搜索控制器信号
    connect(m_excelSearchController, &ExcelSearchController::logMessage,
            this, &MainWindow::updateLog);
}

/**
 * @brief 连接UI信号到控制器
 */
void MainWindow::connectUIToControllers()
{
    // 文件菜单
    connect(ui->actionOpen, &QAction::triggered,
            m_pdfViewerController, &PdfViewerController::onActionOpenTriggered);
    
    // 查看菜单
    connect(ui->actionZoom_In, &QAction::triggered,
            m_pdfViewerController, &PdfViewerController::onActionZoomInTriggered);
    connect(ui->actionZoom_Out, &QAction::triggered,
            m_pdfViewerController, &PdfViewerController::onActionZoomOutTriggered);
    connect(ui->actionPrevious_Page, &QAction::triggered,
            m_pdfViewerController, &PdfViewerController::onActionPreviousPageTriggered);
    connect(ui->actionNext_Page, &QAction::triggered,
            m_pdfViewerController, &PdfViewerController::onActionNextPageTriggered);
    connect(ui->actionContinuous, &QAction::triggered,
            m_pdfViewerController, &PdfViewerController::onActionContinuousTriggered);
    
    // 水印功能
    connect(ui->btnAddWater, &QPushButton::clicked,
            m_watermarkController, &WatermarkController::onBtnAddWaterClicked);
    connect(ui->btnColorSelect, &QPushButton::clicked,
            m_watermarkController, &WatermarkController::onBtnColorSelectClicked);
    connect(ui->cBoxFont, SIGNAL(currentIndexChanged(int)),
            m_watermarkController, SLOT(onFontChanged()));
    
    // 选择目录按钮
    connect(ui->btnSelectInput, &QPushButton::clicked, [this]() {
        QString directory = QFileDialog::getExistingDirectory(
            this, "选择输入目录", "",
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        
        if (!directory.isEmpty()) {
            ui->lineEditInput->setText(directory);
            if (ui->lineEditOutput->text().isEmpty()) {
                ui->lineEditOutput->setText(directory);
            }
        }
    });
    
    connect(ui->btnSelectOutput, &QPushButton::clicked, [this]() {
        QString directory = QFileDialog::getExistingDirectory(
            this, "选择输出目录", "",
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        
        if (!directory.isEmpty()) {
            ui->lineEditOutput->setText(directory);
        }
    });
    
    // 检测输出目录有效性
    connect(ui->lineEditOutput, &CustomLineEdit::dataChanged, [this]() {
        QFileInfo dir(ui->lineEditOutput->text());
        if (!dir.isDir()) {
            ui->lineEditOutput->setText("");
            QMessageBox::information(this, "警告！",
                                   "选择、拖入或者输入一个存在的正确目录");
        }
    });
    
    // PDF转换功能
    connect(ui->btnSelectImageFile, &QPushButton::clicked,
            m_pdfConverterController, &PdfConverterController::onBtnSelectImageFileClicked);
    connect(ui->btnTransform, &QPushButton::clicked,
            m_pdfConverterController, &PdfConverterController::onBtnTransformClicked);
    connect(ui->btnSelectImageDir, &QPushButton::clicked,
            m_pdfConverterController, &PdfConverterController::onBtnSelectImageDirClicked);
    connect(ui->btnTransformBat, &QPushButton::clicked,
            m_pdfConverterController, &PdfConverterController::onBtnTransformBatClicked);
    connect(ui->btnSelectPDFFile, &QPushButton::clicked,
            m_pdfConverterController, &PdfConverterController::onBtnSelectPDFFileClicked);
    connect(ui->btnPdfToImage, &QPushButton::clicked,
            m_pdfConverterController, &PdfConverterController::onBtnPdfToImageClicked);
    connect(ui->btnExportPDF, &QPushButton::clicked,
            m_pdfConverterController, &PdfConverterController::onBtnExportPDFClicked);
    
    // PDF拆分合并功能
    connect(ui->btnSelectFilesplit, &QPushButton::clicked,
            m_splitMergeController, &PdfSplitMergeController::onBtnSelectFileSplitClicked);
    connect(ui->lineEditInputFilesplit, &QLineEdit::textChanged,
            m_splitMergeController, &PdfSplitMergeController::onSplitFileTextChanged);
    connect(ui->btnSelectSplitDir, &QPushButton::clicked,
            m_splitMergeController, &PdfSplitMergeController::onBtnSelectSplitDirClicked);
    connect(ui->lineEditSplitOutput, &QLineEdit::textChanged,
            m_splitMergeController, &PdfSplitMergeController::onSplitOutputTextChanged);
    connect(ui->btnSplitPdf, &QPushButton::clicked,
            m_splitMergeController, &PdfSplitMergeController::onBtnSplitPdfClicked);
    connect(ui->btnAddFile, &QPushButton::clicked,
            m_splitMergeController, &PdfSplitMergeController::onBtnAddFileClicked);
    connect(ui->btnSelectMergeDir, &QPushButton::clicked,
            m_splitMergeController, &PdfSplitMergeController::onBtnSelectMergeDirClicked);
    connect(ui->btnMerge, &QPushButton::clicked,
            m_splitMergeController, &PdfSplitMergeController::onBtnMergeClicked);
    
    // Excel搜索功能
    connect(ui->btnSelectInput_Search, &QPushButton::clicked,
            m_excelSearchController, &ExcelSearchController::onBtnSelectInputSearchClicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged,
            m_excelSearchController, &ExcelSearchController::onTabWidgetCurrentChanged);
    connect(ui->btnSearch, &QPushButton::clicked,
            m_excelSearchController, &ExcelSearchController::onBtnSearchClicked);
    connect(ui->btnSearch_export, &QPushButton::clicked,
            m_excelSearchController, &ExcelSearchController::onBtnSearchExportClicked);
}

/**
 * @brief 设置工具栏
 */
void MainWindow::setupToolBar()
{
    // 设置应用标题
    m_titleLabel->setText("数科-泛生态业务组支撑工具-仅限内部使用");
    m_titleLabel->setStyleSheet(QString::fromUtf8("color: rgb(30, 144, 255);font: 12pt ;border: 0px solid #b8d4f0;"));
    ui->mainToolBar->addWidget(m_titleLabel);
}

/**
 * @brief 退出应用程序
 */
void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

/**
 * @brief 显示关于对话框
 */
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("关于pdf处理"),
                       tr("数科-泛生态业务线工具-仅限内部使用"));
}

/**
 * @brief 显示关于Qt对话框
 */
void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

/**
 * @brief 处理文档打开成功
 */
void MainWindow::onDocumentOpened(const QString &title)
{
    setWindowTitle(title);
}

/**
 * @brief 更新日志
 */
void MainWindow::updateLog(const QString &message)
{
    ui->textEditLog->append(message);
}