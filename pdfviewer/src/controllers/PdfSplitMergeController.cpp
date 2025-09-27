/**
 * @file PdfSplitMergeController.cpp
 * @brief PDF拆分合并控制器实现
 */

#include "PdfSplitMergeController.h"
#include "ui_mainwindow.h"
#include "../../include/mytable.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QPdfDocument>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <list>

/**
 * @brief 构造函数
 */
PdfSplitMergeController::PdfSplitMergeController(Ui::MainWindow *ui, QObject *parent)
    : QObject(parent)
    , m_ui(ui)
{
}

/**
 * @brief 析构函数
 */
PdfSplitMergeController::~PdfSplitMergeController()
{
}

/**
 * @brief 初始化控制器
 */
void PdfSplitMergeController::initialize()
{
    // 初始化合并表格
    initTable();
    
    // 连接合并表格的信号
    connect(m_ui->tableWidgetMerge, &mytable::selectRow, this, [this]() {
        QString file = m_ui->tableWidgetMerge->getCurrFile();
        if (!file.isEmpty()) {
            emit requestOpenPdf(QUrl::fromLocalFile(file));
        }
    });
}

/**
 * @brief 初始化合并文件表格
 */
void PdfSplitMergeController::initTable()
{
    // 设置表头
    QStringList headerLabels;
    headerLabels << "文件名称"
                 << "页数"
                 << "范围"
                 << "文件操作";
    m_ui->tableWidgetMerge->setHorizontalHeaderLabels(headerLabels);
    m_ui->tableWidgetMerge->setColumnWidth(0, 150);
    m_ui->tableWidgetMerge->setColumnWidth(1, 45);
    m_ui->tableWidgetMerge->setColumnWidth(2, 85);
    m_ui->tableWidgetMerge->setColumnWidth(3, 120);
}

/**
 * @brief 选择拆分文件按钮点击处理
 */
void PdfSplitMergeController::onBtnSelectFileSplitClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        nullptr, "选择PDF文件", QDir::homePath(), "PDF文件 (*.pdf *.PDF)");
    
    if (!fileName.isEmpty()) {
        m_ui->lineEditInputFilesplit->setText(fileName);
        
        QPdfDocument::DocumentError err;
        emit requestOpenPdf(QUrl::fromLocalFile(fileName));
        
        // 更新页数信息
        int pages = getPages(fileName.toStdString());
        m_ui->lineEditSplitpages->setText(QString::number(pages));
        m_ui->lineEditSplitEnd->setText(QString::number(pages));
        
        QFileInfo path(fileName);
        m_ui->lineEditSplitOutput->setText(path.absolutePath());
    }
}

/**
 * @brief 拆分文件输入框文本变化处理
 */
void PdfSplitMergeController::onSplitFileTextChanged(const QString &filename)
{
    // 判断是否为pdf文件
    QFileInfo fileInfo(filename);
    if ((fileInfo.isFile()) && (filename.contains(".pdf"))) {
        emit requestOpenPdf(QUrl::fromLocalFile(filename));
        
        // 更新页数信息
        int pages = getPages(filename.toStdString());
        m_ui->lineEditSplitpages->setText(QString::number(pages));
        m_ui->lineEditSplitEnd->setText(QString::number(pages));
        
        QFileInfo path(filename);
        m_ui->lineEditSplitOutput->setText(path.absolutePath());
    } else {
        if (!filename.isEmpty()) {
            QMessageBox::information(nullptr, "警告！", "请选择正确的pdf文件");
        }
        m_ui->lineEditInputFilesplit->setText("");
    }
}

/**
 * @brief 选择拆分输出目录按钮点击处理
 */
void PdfSplitMergeController::onBtnSelectSplitDirClicked()
{
    QString directory = QFileDialog::getExistingDirectory(
        nullptr, "选择输出目录", "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!directory.isEmpty()) {
        m_ui->lineEditSplitOutput->setText(directory);
    }
}

/**
 * @brief 拆分输出目录文本变化处理
 */
void PdfSplitMergeController::onSplitOutputTextChanged(const QString &filepath)
{
    QFileInfo path(filepath);
    if (!path.isDir()) {
        if (!filepath.isEmpty()) {
            QMessageBox::information(nullptr, "警告！", "请选择正确的目录");
        }
        m_ui->lineEditSplitOutput->setText("");
    }
}

/**
 * @brief 验证拆分输入参数
 */
bool PdfSplitMergeController::validateSplitInput()
{
    if ((m_ui->lineEditInputFilesplit->text().isEmpty()) ||
        (m_ui->lineEditSplitOutput->text().isEmpty())) {
        QMessageBox::information(nullptr, "警告！", "请选择输入文件、输出目录");
        return false;
    }
    return true;
}

/**
 * @brief 拆分PDF按钮点击处理
 */
void PdfSplitMergeController::onBtnSplitPdfClicked()
{
    if (!validateSplitInput()) {
        return;
    }
    
    // 文件页数
    int pages = m_ui->lineEditSplitpages->text().toInt();
    std::string in = m_ui->lineEditInputFilesplit->text().toStdString();
    QFileInfo file(m_ui->lineEditInputFilesplit->text());
    std::string filename = file.fileName().left(file.fileName().length() - 4).toStdString();
    std::string out = m_ui->lineEditSplitOutput->text().toStdString() + "/" + filename;
    
    // 通过radiobutton来选择两种拆分方式
    if (m_ui->radioButtonSpliterange->isChecked()) {
        // 第一种拆分方式：从原始文件中提取区间页数为新文件
        QString range = m_ui->lineEditSplitscope->text();
        performRangeSplit(in, out, pages, range);
        
        QMessageBox::information(nullptr, "PDF拆分完成！",
                               "文件保存在保存：" + m_ui->lineEditSplitOutput->text());
        emit logMessage("拆分PDF保存目录中:" + m_ui->lineEditSplitOutput->text());
    } else {
        // 第二种拆分方式
        performPageSplit(in, out, pages);
        
        QMessageBox::information(nullptr, "PDF拆分完成！",
                               "文件保存在保存：" + m_ui->lineEditSplitOutput->text());
        emit logMessage("拆分PDF保存目录中:" + m_ui->lineEditSplitOutput->text());
    }
}

/**
 * @brief 执行范围拆分
 */
void PdfSplitMergeController::performRangeSplit(const std::string &input, 
                                               const std::string &output,
                                               int pages, const QString &range)
{
    QString sub_range;
    QStringList parts, sub_parts;
    int start = 0, end = 0;
    
    // 通过正则表达式来验证拆分规则的有效性
    QRegularExpression regex("^\\d+-\\d+(,\\d+-\\d+)*$");
    QRegularExpressionMatch match = regex.match(range);
    
    if (match.hasMatch()) {
        parts = range.split(",");
        for (int i = 0; i < parts.size(); ++i) {
            sub_range = parts.at(i);
            sub_parts = sub_range.split("-");
            start = sub_parts[0].toInt() - 1;
            end = sub_parts[1].toInt();
            
            if (end > pages) {
                QMessageBox::information(nullptr, "警告！", "拆分范围不能大于文档总页数");
                return;
            }
            if (start > end) {
                QMessageBox::information(nullptr, "警告！", "拆分范围开始值不能大于结束值");
                return;
            }
            
            std::string sub_out = output + "_" + sub_parts[0].toStdString() + "-" +
                                 sub_parts[1].toStdString();
            splitPdf(input, sub_out, start, end);
        }
    } else {
        QMessageBox::information(nullptr, "警告！", "目前只支持n-n,n-n...格式的拆分");
    }
}

/**
 * @brief 执行页数拆分
 */
void PdfSplitMergeController::performPageSplit(const std::string &input,
                                              const std::string &output, int pages)
{
    int start = m_ui->lineEditSplitStart->text().toInt() - 1;
    int end = m_ui->lineEditSplitEnd->text().toInt();
    
    if ((end > pages) || (start > pages)) {
        QMessageBox::information(nullptr, "警告！", "拆分范围不能大于文档总页数");
        return;
    }
    if (start > end) {
        QMessageBox::information(nullptr, "警告！", "拆分范围开始值不能大于结束值");
        return;
    }
    
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QFileInfo file(QString::fromStdString(input));
    std::string filename = file.fileName().left(file.fileName().length() - 4).toStdString();
    
    // 第一次拆分生成中间结果文件
    splitPdf(input, tempDir.toStdString() + "/" + filename, start, end);
    
    // 每个拆分子文件的页数
    int subPages = m_ui->lineEditSubPages->text().toInt();
    
    // 第二次拆分
    splitPdf(tempDir.toStdString() + "/" + filename + "_split.pdf", output, subPages);
    
    QFile tempfile(tempDir + "/" + QString::fromStdString(filename) + "_split.pdf");
    tempfile.remove();
}

/**
 * @brief 添加文件到合并列表按钮点击处理
 */
void PdfSplitMergeController::onBtnAddFileClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        nullptr, "选择PDF文件", QDir::homePath(), "PDF文件 (*.pdf *.PDF )");
    
    if (fileName.isEmpty()) {
        QMessageBox::information(nullptr, "提示信息！", "文件名为空");
        return;
    }
    
    m_ui->tableWidgetMerge->AddFile(fileName);
}

/**
 * @brief 选择合并输出目录按钮点击处理
 */
void PdfSplitMergeController::onBtnSelectMergeDirClicked()
{
    QString directory = QFileDialog::getExistingDirectory(
        nullptr, "选择输出目录", "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!directory.isEmpty()) {
        m_ui->lineEditOutMerge->setText(directory);
    }
}

/**
 * @brief 验证合并输入参数
 */
bool PdfSplitMergeController::validateMergeInput()
{
    QString outfileName = m_ui->lineEditMergeOutFile->text();
    QString outDir = m_ui->lineEditOutMerge->text();
    
    if (outfileName.isEmpty()) {
        QMessageBox::information(nullptr, "提示信息！", "请输入合并后的文件名");
        m_ui->lineEditMergeOutFile->setFocus();
        return false;
    }
    
    if (outDir.isEmpty()) {
        QMessageBox::information(nullptr, "提示信息！", "合并后的输出目录");
        m_ui->lineEditOutMerge->setFocus();
        return false;
    }
    
    return true;
}

/**
 * @brief 合并PDF按钮点击处理
 */
void PdfSplitMergeController::onBtnMergeClicked()
{
    if (!validateMergeInput()) {
        return;
    }
    
    // 临时文件列表
    std::list<std::string> fileList;
    QString outDir = m_ui->lineEditOutMerge->text();
    QString outfileName = m_ui->lineEditMergeOutFile->text();
    
    // 合并前的临时文件目录
    QString tempFile = outDir + "/" + "YXNkZmRzZmFzZGZhZHNmYWRzZgo";
    
    if (outfileName.right(4).toUpper() != ".PDF") {
        outfileName = outfileName + ".pdf";
    }
    
    // 创建临时目录
    if (!createDirectory(tempFile)) {
        QMessageBox::information(nullptr, "提示信息！", "请选择正确的输出目录");
        m_ui->lineEditOutMerge->setFocus();
        return;
    }
    
    QString infilename;
    for (int row = 0; row < m_ui->tableWidgetMerge->rowCount(); ++row) {
        // 从第一列中获取原始文件名
        QTableWidgetItem *item = m_ui->tableWidgetMerge->item(row, 0);
        if (item) {
            infilename = item->text();
        } else {
            return;
        }
        
        int pStart, pEnd;
        // 从第三列中获取开始结束页码
        QWidget *currenCell = m_ui->tableWidgetMerge->cellWidget(row, 2);
        QList<QLineEdit *> currLine = currenCell->findChildren<QLineEdit *>();
        pStart = currLine[0]->text().toInt();
        pEnd = currLine[1]->text().toInt();
        
        // 按行拆到临时文件
        if (splitPdf(infilename.toStdString(),
                    tempFile.toStdString() + "/" + std::to_string(row), pStart - 1, pEnd) == 0) {
            // 临时文件放到列表中
            fileList.push_back(tempFile.toStdString() + "/" + std::to_string(row) + "_split.pdf");
        }
    }
    
    outfileName = outDir + "/" + outfileName;
    
    // 将临时文件列表中的文件进行合并
    if (mergePdf(fileList, outfileName.toStdString()) == 0) {
        QFileInfo info(outfileName);
        
        // 在预览中打开合并完成的文件
        emit requestOpenPdf(QUrl::fromLocalFile(outfileName));
        emit requestFitToWidth();
        emit logMessage("PDF合并完成保存在：" + info.filePath());
        
        QMessageBox::information(nullptr, "PDF合并完成！",
                               "文件保存在：" + info.filePath() + "\n");
        
        QDir temp(tempFile);
        // 删除整个目录及其子目录
        temp.removeRecursively();
    }
}