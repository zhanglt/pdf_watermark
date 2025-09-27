/**
 * @file PdfConverterController.cpp
 * @brief PDF转换控制器实现
 */

#include "PdfConverterController.h"
#include "ui_mainwindow.h"
#include "../../include/pdf2image/pdf2ImageThreadSingle.h"
#include "../../include/QProgressIndicator.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QStandardPaths>
#include <QPdfDocument>

/**
 * @brief 构造函数
 */
PdfConverterController::PdfConverterController(Ui::MainWindow *ui, QThreadPool &threadPool, QObject *parent)
    : QObject(parent)
    , m_ui(ui)
    , m_threadPool(threadPool)
    , m_pdf2imageThread(nullptr)
    , m_completedCount(0)
    , m_totalFiles(0)
{
}

/**
 * @brief 析构函数
 */
PdfConverterController::~PdfConverterController()
{
}

/**
 * @brief 初始化控制器
 */
void PdfConverterController::initialize()
{
    // 初始化状态
}

/**
 * @brief 验证图片文件
 */
bool PdfConverterController::validateImageFile(const QString &file)
{
    if (!isImageByExtension(file) || !isImageByMagicNumber(file) || file.isEmpty()) {
        QMessageBox::information(nullptr, "提示信息！", "请选择正确的图像文件");
        return false;
    }
    return true;
}

/**
 * @brief 验证PDF文件
 */
bool PdfConverterController::validatePdfFile(const QString &file)
{
    if (file.isEmpty()) {
        QMessageBox::information(nullptr, "提示信息！", "请选择PDF文件");
        return false;
    }
    
    if (!isPDFByExtension(file) || !isPDFByMagicNumber(file)) {
        QMessageBox::information(nullptr, "提示信息！", "请选择正确的PDF文件");
        return false;
    }
    return true;
}

/**
 * @brief 选择图片文件按钮点击处理
 */
void PdfConverterController::onBtnSelectImageFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr, "选择图像文件", QDir::homePath(),
        "图像文件 (*.png *.PNG *.jpg *.JPG *.bmp *.BMP)");
    
    if (!filePath.isEmpty()) {
        m_ui->lineEditImageFile->setText(filePath);
    }
}

/**
 * @brief 图片转PDF按钮点击处理
 */
void PdfConverterController::onBtnTransformClicked()
{
    QString file = m_ui->lineEditImageFile->text();
    
    if (!validateImageFile(file)) {
        m_ui->lineEditImageFile->setText("");
        return;
    }
    
    QFileInfo info(file);
    QString outputFile = info.filePath() + ".pdf";
    
    if (image2pdf(info.filePath().toStdString(), outputFile.toStdString()) == 0) {
        emit logMessage("文件保存在：" + outputFile + "\n");
        emit requestOpenPdf(QUrl::fromLocalFile(outputFile));
        emit requestFitToWidth();
        emit logMessage("PDF转换完成保存在：" + outputFile);
        QMessageBox::information(nullptr, "PDF转换完成！", 
                               "文件保存在：" + outputFile + "\n");
    }
}

/**
 * @brief 选择图片目录按钮点击处理
 */
void PdfConverterController::onBtnSelectImageDirClicked()
{
    QString directory = QFileDialog::getExistingDirectory(
        nullptr, "选择图片目录", "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!directory.isEmpty()) {
        m_ui->lineEditImageDir->setText(directory);
    }
}

/**
 * @brief 批量图片转PDF按钮点击处理
 */
void PdfConverterController::onBtnTransformBatClicked()
{
    if (m_ui->lineEditImageDir->text().isEmpty()) {
        QMessageBox::information(nullptr, "提示信息！", "请选择图像目录");
        return;
    }
    
    QFileInfo file(m_ui->lineEditImageDir->text());
    if (!file.isDir()) {
        QMessageBox::information(nullptr, "提示信息！", "请选择正确的图像目录");
        m_ui->lineEditImageDir->setText("");
        return;
    }
    
    QDir dir(m_ui->lineEditImageDir->text());
    QStringList files;
    tDirectory(dir, files, {"png", "jpg", "bmp"});
    
    for (QString &file : files) {
        image2pdf(file.toStdString(), (file + ".pdf").toStdString());
    }
    
    QMessageBox::information(nullptr, "PDF转换完成！", "PDF保存在图片源目录中");
    emit logMessage("PDF保存源目录中:" + m_ui->lineEditImageDir->text());
}

/**
 * @brief 选择PDF文件按钮点击处理
 */
void PdfConverterController::onBtnSelectPDFFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr, "选择PDF文件", QDir::homePath(), "PDF文件 (*.pdf *.PDF)");
    
    if (!filePath.isEmpty()) {
        m_ui->lineEditPdfFile->setText(filePath);
        emit requestOpenPdf(QUrl::fromLocalFile(filePath));
    }
}

/**
 * @brief PDF转图片按钮点击处理
 */
void PdfConverterController::onBtnPdfToImageClicked()
{
    QString file = m_ui->lineEditPdfFile->text();
    
    if (!validatePdfFile(file)) {
        m_ui->lineEditPdfFile->setText("");
        return;
    }
    
    QString path = file.left(file.length() - 4) + "_YXNkZmRzZmFzZGZhZHNmYWRzZgo";
    
    m_pdf2imageThread = new pdf2imageThreadSingle();
    m_pdf2imageThread->setSourceFile(file);
    m_pdf2imageThread->setIs2pdf(false);
    
    // 创建图片保存目录
    if (!createDirectory(path)) {
        // 目录创建失败处理
    }
    
    m_pdf2imageThread->setImagePath(path);
    m_pdf2imageThread->setResolution(100);
    
    // 连接完成信号
    connect(m_pdf2imageThread, &pdf2imageThreadSingle::addFinish, this, [this, path]() {
        if (m_threadPool.activeThreadCount() == 0) {
            emit Finished();
            emit logMessage("图片保存目录:" + path);
            QMessageBox::information(nullptr, "导出图片完成！", "导出图片完成！");
        }
    });
    
    m_threadPool.start(m_pdf2imageThread);
    showProgressIndicator();
}

/**
 * @brief 导出PDF按钮点击处理
 */
void PdfConverterController::onBtnExportPDFClicked()
{
    QString outputDir = m_ui->lineEditOutput->text();
    QFileInfo output(outputDir);
    
    if (!output.isDir()) {
        m_ui->lineEditOutput->setFocus();
        QMessageBox::information(nullptr, "提示", "输入目录或者选择正确目录");
        return;
    }
    
    QElapsedTimer time;
    time.start();
    
    QDir dir(m_ui->lineEditOutput->text() + "/_out_/");
    exportPdf(dir);
    
    emit logMessage(QString("导出图片用时:%1毫秒,结果文件保存在:%2/_pdf_/")
                    .arg(time.elapsed())
                    .arg(m_ui->lineEditOutput->text()));
}

/**
 * @brief 导出PDF功能实现
 */
void PdfConverterController::exportPdf(QDir pdfdir)
{
    m_ui->btnAddWater->setEnabled(false);
    
    QStringList files;
    traverseDirectory(pdfdir, files, "pdf", "_pdf_");
    
    if (files.length() < 1) {
        QMessageBox::information(nullptr, "系统提示！",
                               "目标目录：" + m_ui->lineEditOutput->text() + "/_out_ 中没有文件");
        m_ui->btnAddWater->setEnabled(true);
        return;
    }
    
    // 初始化计数器
    m_completedCount.storeRelaxed(0);
    m_totalFiles = files.length();
    
    QString originalPdfDirPath = pdfdir.path();
    
    for (const QString &originalFile : files) {
        auto* thread = new pdf2imageThreadSingle();
        QString file = originalFile;
        
        // 设置转换路径
        QString imagePath = QString(file).replace("_out_", "_image_");
        createDirectory(imagePath);
        
        QString pdfPath = QString(imagePath).replace("_image_", "_pdf_");
        QFileInfo fileInfo(pdfPath);
        QString path = fileInfo.absolutePath();
        QString filename = fileInfo.fileName();
        createDirectory(path);
        
        // 设置线程参数
        thread->setSourceFile(file);
        thread->setImagePath(imagePath);
        thread->setTargetFile(path + "/" + filename);
        thread->setIs2pdf(true);
        thread->setResolution(m_ui->cBoxResolution->currentText().toInt());
        
        // 连接完成信号
        connect(thread, &pdf2imageThreadSingle::addFinish, this, 
            [this, originalPdfDirPath]() {
                int completed = m_completedCount.fetchAndAddRelaxed(1) + 1;
                if (completed == m_totalFiles) {
                    emit Finished();
                    m_ui->btnAddWater->setEnabled(true);
                    QMessageBox::information(nullptr, "导出PDF完成！",
                                           QString("导出PDF完成！<br>共%1个文件参与处理")
                                           .arg(m_totalFiles));
                    
                    // 删除临时图片目录
                    QDir imageRootDir(QString(originalPdfDirPath).replace("_out_", "_image_"));
                    bool success = imageRootDir.removeRecursively();
                    if (success) {
                        emit logMessage("图片目录：" + imageRootDir.path() + "删除成功.");
                    } else {
                        emit logMessage("图片目录：" + imageRootDir.path() + "删除失败");
                    }
                }
            });
        
        m_threadPool.start(thread);
    }
    
    if (files.length() > 0) {
        showProgressIndicator();
    }
}

/**
 * @brief 显示进度指示器
 */
void PdfConverterController::showProgressIndicator()
{
    QProgressIndicator *pIndicator = nullptr;
    QDialog dialog;
    
    QObject::connect(this, &PdfConverterController::Finished, &dialog, &QDialog::close);
    
    dialog.setWindowTitle("文件转换处理...");
    dialog.resize(200, 20);
    dialog.setWindowFlags(dialog.windowFlags() | Qt::FramelessWindowHint);
    
    pIndicator = new QProgressIndicator(&dialog);
    QVBoxLayout *hLayout = new QVBoxLayout(&dialog);
    QLabel *l1 = new QLabel("正在转换。。。。");
    
    hLayout->setMargin(1);
    hLayout->addWidget(pIndicator);
    hLayout->addWidget(l1);
    hLayout->setAlignment(pIndicator, Qt::AlignCenter);
    hLayout->setAlignment(l1, Qt::AlignCenter);
    dialog.setLayout(hLayout);
    
    pIndicator->setColor(QColor(12, 52, 255));
    pIndicator->startAnimation();
    dialog.exec();
}