/**
 * @file PdfViewerController.cpp
 * @brief PDF查看器控制器实现
 */

#include "PdfViewerController.h"
#include "ui_mainwindow.h"
#include "../../zoomselector.h"
#include "../../pageselector.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QtMath>
#include <QPalette>
#include <QPdfBookmarkModel>
#include <QPdfPageNavigation>

// 缩放倍数常量，使用√2作为缩放步长
const qreal PdfViewerController::zoomMultiplier = qSqrt(2.0);

/**
 * @brief 构造函数
 */
PdfViewerController::PdfViewerController(Ui::MainWindow *ui, QObject *parent)
    : QObject(parent)
    , m_ui(ui)
    , m_document(new QPdfDocument(this))
    , m_zoomSelector(new ZoomSelector(qobject_cast<QWidget*>(parent)))
    , m_pageSelector(new PageSelector(qobject_cast<QWidget*>(parent)))
{
}

/**
 * @brief 析构函数
 */
PdfViewerController::~PdfViewerController()
{
}

/**
 * @brief 初始化控制器
 */
void PdfViewerController::initialize()
{
    // 在工具栏添加缩放和页面选择器
    m_ui->mainToolBar->insertWidget(m_ui->actionZoom_In, m_zoomSelector);
    m_zoomSelector->setMaximumWidth(400);
    m_zoomSelector->setMaximumHeight(25);
    
    m_ui->mainToolBar->addWidget(m_pageSelector);
    m_pageSelector->setMinimumWidth(200);
    m_pageSelector->m_pageNumberEdit->setMinimumWidth(50);
    m_pageSelector->m_pageNumberEdit->setMaximumWidth(50);
    m_pageSelector->m_pageCountLabel->setMinimumWidth(40);
    
    // 设置页面导航
    m_pageSelector->setPageNavigation(m_ui->pdfView->pageNavigation());
    
    // 连接信号槽
    connect(m_zoomSelector, &ZoomSelector::zoomModeChanged, 
            m_ui->pdfView, &QPdfView::setZoomMode);
    connect(m_zoomSelector, &ZoomSelector::zoomFactorChanged, 
            m_ui->pdfView, &QPdfView::setZoomFactor);
    connect(m_ui->pdfView, &QPdfView::zoomFactorChanged, 
            m_zoomSelector, &ZoomSelector::setZoomFactor);
    
    m_zoomSelector->reset();
    
    // 设置PDF文档
    m_ui->pdfView->setDocument(m_document);
    
    // 设置查看器样式
    setupViewerStyle();
}

/**
 * @brief 设置PDF查看器背景颜色
 */
void PdfViewerController::setupViewerStyle()
{
    QPalette palette = m_ui->pdfView->palette();
    QColor color(199, 199, 199);  // RGB(199,199,199) 浅灰色
    palette.setBrush(QPalette::Dark, color);
    m_ui->pdfView->setPalette(palette);
}

/**
 * @brief 打开PDF文档
 */
void PdfViewerController::open(const QUrl &docLocation, QPdfDocument::DocumentError &err)
{
    if (docLocation.isLocalFile()) {
        QPdfDocument::DocumentError error = m_document->load(docLocation.toLocalFile());
        
        if (error != QPdfDocument::NoError) {
            err = error;
            emit openFailed(tr("无法打开文件"));
        } else {
            const auto documentTitle = m_document->metaData(QPdfDocument::Title).toString();
            const QString title = !documentTitle.isEmpty() ? documentTitle : "PDF浏览器";
            emit documentOpened(title);
            err = error;
        }
    } else {
        QMessageBox::critical(nullptr, tr("文件打开错误"), 
                              tr("%1 无效的本地文件").arg(docLocation.toString()));
        emit openFailed(tr("无效的本地文件"));
    }
}

/**
 * @brief 书签选中事件处理
 */
void PdfViewerController::bookmarkSelected(const QModelIndex &index)
{
    if (!index.isValid()) return;

    const int page = index.data(QPdfBookmarkModel::PageNumberRole).toInt();
    m_ui->pdfView->pageNavigation()->setCurrentPage(page);
}

/**
 * @brief 打开文件动作处理
 */
void PdfViewerController::onActionOpenTriggered()
{
    QUrl toOpen = QFileDialog::getOpenFileUrl(
        nullptr, tr("选择打开PDF文件"),
        QUrl("file:///" + m_ui->lineEditOutput->text()), "文档 (*.pdf)");

    QPdfDocument::DocumentError err;

    if (toOpen.isValid()) {
        open(toOpen, err);
        m_zoomSelector->zoomModeChanged(QPdfView::FitToWidth);
    }
}

/**
 * @brief 放大PDF显示
 */
void PdfViewerController::onActionZoomInTriggered()
{
    m_ui->pdfView->setZoomFactor(m_ui->pdfView->zoomFactor() * zoomMultiplier);
}

/**
 * @brief 缩小PDF显示
 */
void PdfViewerController::onActionZoomOutTriggered()
{
    m_ui->pdfView->setZoomFactor(m_ui->pdfView->zoomFactor() / zoomMultiplier);
}

/**
 * @brief 跳转到上一页
 */
void PdfViewerController::onActionPreviousPageTriggered()
{
    m_ui->pdfView->pageNavigation()->goToPreviousPage();
}

/**
 * @brief 跳转到下一页
 */
void PdfViewerController::onActionNextPageTriggered()
{
    m_ui->pdfView->pageNavigation()->goToNextPage();
}

/**
 * @brief 切换PDF的页面显示模式
 */
void PdfViewerController::onActionContinuousTriggered()
{
    m_ui->pdfView->setPageMode(m_ui->actionContinuous->isChecked()
                               ? QPdfView::MultiPage
                               : QPdfView::SinglePage);
}