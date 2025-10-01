/**
 * @file ExcelSearchController.cpp
 * @brief Excel搜索控制器实现
 */

#include "ExcelSearchController.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QThreadPool>
#include <QDebug>
#include "lib/qtxlsx/include/QtXlsx/xlsxdocument.h"
#include "lib/qtxlsx/include/QtXlsx/xlsxformat.h"

using namespace QXlsx;

/**
 * @brief 构造函数
 */
ExcelSearchController::ExcelSearchController(Ui::MainWindow *ui, QObject *parent)
    : QObject(parent)
    , m_ui(ui)
{
}

/**
 * @brief 析构函数
 */
ExcelSearchController::~ExcelSearchController()
{
}

/**
 * @brief 初始化控制器
 */
void ExcelSearchController::initialize()
{
    // 初始化搜索结果树
    m_ui->treeWidget_Search->setHeaderLabels(QStringList() << "文件/工作表" << "单元格" << "内容");
    m_ui->treeWidget_Search->setColumnWidth(0, 300);
    m_ui->treeWidget_Search->setColumnWidth(1, 50);
}

/**
 * @brief 选择搜索目录按钮点击处理
 */
void ExcelSearchController::onBtnSelectInputSearchClicked()
{
    QString directory = QFileDialog::getExistingDirectory(
        nullptr, "选择输入目录", "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!directory.isEmpty()) {
        qDebug() << "选择的目录：" << directory;
        m_ui->lineEditInput_Search->setText(directory);
        
        // 保存搜索目录到临时文件
        saveSearchDirectory(directory);
    }
}

/**
 * @brief 保存搜索目录到临时文件
 */
void ExcelSearchController::saveSearchDirectory(const QString &directory)
{
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
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

/**
 * @brief 从临时文件加载搜索目录
 */
QString ExcelSearchController::loadSearchDirectory()
{
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString testFilePath = tempPath + "/search.txt";
    QFile searchfile(testFilePath);
    
    if (!searchfile.exists()) {
        qDebug() << "文件不存在:" << testFilePath;
        return QString();
    }
    
    if (!searchfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << searchfile.fileName();
        return QString();
    }
    
    QTextStream in(&searchfile);
    QString content = in.readAll();
    searchfile.close();
    
    return content;
}

/**
 * @brief Tab页切换处理
 */
void ExcelSearchController::onTabWidgetCurrentChanged(int index)
{
    // 当切换到搜索tab时(假设索引为4)，自动填充搜索目录
    if (index == 4) {
        QString directory = loadSearchDirectory();
        if (!directory.isEmpty()) {
            m_ui->lineEditInput_Search->setText(directory);
        }
    }
}

/**
 * @brief 验证搜索输入参数
 */
bool ExcelSearchController::validateSearchInput()
{
    QString key = m_ui->lineEditInput_Search_Key->text();
    QString inputDir = m_ui->lineEditInput_Search->text();
    QDir dir(inputDir);
    
    if (!dir.exists()) {
        QMessageBox::information(nullptr, "提示", "搜索目录不存在,请重新选择");
        return false;
    }
    
    if (key.isEmpty() || inputDir.isEmpty()) {
        QMessageBox::information(nullptr, "提示", "搜索目录、搜索关键字不能为空");
        return false;
    }
    
    return true;
}

/**
 * @brief 清空并初始化搜索结果树
 */
void ExcelSearchController::initializeSearchTree()
{
    m_ui->treeWidget_Search->clear();
    m_ui->treeWidget_Search->setHeaderLabels(QStringList() << "文件/工作表" << "单元格" << "内容");
    m_ui->treeWidget_Search->setColumnWidth(0, 300);
    m_ui->treeWidget_Search->setColumnWidth(1, 50);
}

/**
 * @brief 搜索按钮点击处理
 */
void ExcelSearchController::onBtnSearchClicked()
{
    if (!validateSearchInput()) {
        return;
    }
    
    // 清空搜索结果
    initializeSearchTree();
    
    QString key = m_ui->lineEditInput_Search_Key->text();
    QString inputDir = m_ui->lineEditInput_Search->text();
    QDir dir(inputDir);
    
    QStringList fileNames;
    traverseDirectory(dir, fileNames, "xlsx", "_out_");
    
    // 创建搜索线程
    SearchThread *searchTask = new SearchThread();
    searchTask->setFileNames(fileNames);
    searchTask->setSearchText(key);
    
    // 连接信号
    connect(searchTask, &SearchThread::searchFinished, 
            this, &ExcelSearchController::onSearchFinished);
    connect(searchTask, &SearchThread::searchProgress, 
            this, &ExcelSearchController::onSearchProgress);
    connect(searchTask, &SearchThread::searchError, 
            this, &ExcelSearchController::onSearchError);
    
    // 启动线程
    QThreadPool::globalInstance()->start(searchTask);
    
    qDebug() << "搜索任务已提交到线程池";
}

/**
 * @brief 搜索完成槽函数
 */
void ExcelSearchController::onSearchFinished(const QList<SearchResult> &results)
{
    qDebug() << "搜索完成，结果数量:" << results.size();
    
    QString currentFile;
    QTreeWidgetItem *fileItem = nullptr;
    
    for (const SearchResult &result : results) {
        // 当切换到新文件时，创建新的文件节点
        if (result.fileName != currentFile) {
            currentFile = result.fileName;
            fileItem = new QTreeWidgetItem(m_ui->treeWidget_Search);
            fileItem->setText(0, "文件: " + result.fileName);
            fileItem->setExpanded(true); // 展开文件节点
        }
        
        // 创建结果子节点
        QTreeWidgetItem *resultItem = new QTreeWidgetItem(fileItem);
        resultItem->setText(0, "" + result.sheetName);
        resultItem->setText(1, result.cellReference);
        resultItem->setText(2, result.cellValue.toString());
    }
    
    m_ui->treeWidget_Search->show();
    
    if (results.isEmpty()) {
        QMessageBox::information(nullptr, "搜索结果", "未找到匹配的结果");
    } else {
        QMessageBox::information(nullptr, "搜索完成", 
                               QString("搜索完成！找到 %1 个匹配结果").arg(results.size()));
    }
}

/**
 * @brief 搜索进度槽函数
 */
void ExcelSearchController::onSearchProgress(int processed, int total, const QString &currentFileName)
{
    qDebug() << "搜索进度:" << processed << "/" << total << ", 当前处理文件:" << currentFileName;
    emit logMessage("文件名称:" + currentFileName);
    emit logMessage(QString("搜索进度---：<%1/%2>").arg(processed).arg(total));
}

/**
 * @brief 搜索错误槽函数
 */
void ExcelSearchController::onSearchError(const QString &error)
{
    qDebug() << "搜索错误:" << error;
    QMessageBox::warning(nullptr, "搜索错误", error);
}

/**
 * @brief 检查树形控件是否为空
 */
bool ExcelSearchController::isTreeWidgetEmpty(QTreeWidget *treeWidget)
{
    return treeWidget->topLevelItemCount() == 0;
}

/**
 * @brief 导出树形控件内容到Excel
 */
QString ExcelSearchController::exportTreeWidgetToExcel(QTreeWidget &treeWidget, 
                                                      const QString &exportFilePath)
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
            QString cellReference = resultItem->text(1);
            QString cellValue = resultItem->text(2);
            
            // 写入数据到Excel
            exportDoc.currentWorksheet()->writeHyperlink(currentRow, 1,QUrl::fromLocalFile(fileName),hyperlinkFormat,fileName);
            //exportDoc.write(currentRow, 1, fileName);
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

/**
 * @brief 导出搜索结果按钮点击处理
 */
void ExcelSearchController::onBtnSearchExportClicked()
{
    if (isTreeWidgetEmpty(m_ui->treeWidget_Search)) {
        QMessageBox::information(nullptr, "数据导出", "搜索结果数据为空");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(nullptr, "保存文件", 
                                                  "search.xlsx", 
                                                  "Excel Files (*.xlsx)");
    QString result;
    if (!fileName.isEmpty()) {
        result = exportTreeWidgetToExcel(*m_ui->treeWidget_Search, fileName);
        if (!result.isNull()) {
            QMessageBox::information(nullptr, "导出结果", result);
        }
    }
}
