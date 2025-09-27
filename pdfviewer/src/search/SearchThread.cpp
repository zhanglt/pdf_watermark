/**
 * @file SearchThread.cpp
 * @brief Excel文件搜索线程实现文件
 * 
 * 实现了多线程Excel文件内容搜索功能，提供以下主要功能：
 * - 多文件并行搜索Excel单元格内容
 * - 进度报告和错误处理
 * - 搜索结果的格式化和导出
 * - Excel文件格式化和样式设置辅助函数
 * 
 * 依赖库：
 * - QtXlsx (Excel文件读写)
 * - Qt Core (多线程、信号槽)
 * 
 * 该类继承自QObject和QRunnable，支持在线程池中执行
 */

#pragma execution_character_set("utf-8")
#include "include/search/SearchThread.h"
#include "search.h"
#include <QDebug>
#include <QThread>
#include "lib/qtxlsx/include/QtXlsx/xlsxdocument.h"

QTXLSX_USE_NAMESPACE

/**
 * @brief 搜索线程构造函数
 * @param parent 父对象指针
 * 
 * 初始化搜索线程对象：
 * - 继承QObject和QRunnable，支持信号槽和线程池执行
 * - 设置自动删除，执行完毕后自动清理内存
 * - 注册自定义数据类型，支持跨线程信号传递
 */
SearchThread::SearchThread(QObject *parent) : QObject(parent), QRunnable() {
    setAutoDelete(true);  // 任务完成后自动删除对象
    // 注册自定义类型，用于跨线程信号传递
    qRegisterMetaType<SearchResult>("SearchResult");
    qRegisterMetaType<QList<SearchResult>>("QList<SearchResult>");
}

/**
 * @brief 设置要搜索的文件列表
 * @param fileNames Excel文件路径列表
 */
void SearchThread::setFileNames(const QStringList &fileNames) {
    m_fileNames = fileNames;
}

/**
 * @brief 设置搜索关键词
 * @param searchText 要在Excel单元格中搜索的文本
 */
void SearchThread::setSearchText(const QString &searchText) {
    m_searchText = searchText;
}



/**
 * @brief 检查树控件是否为空
 * @param treeWidget 要检查的树控件指针
 * @return true: 树控件为空, false: 树控件有内容
 */
bool SearchThread::isTreeWidgetEmpty(QTreeWidget *treeWidget) {
    return treeWidget->topLevelItemCount() == 0;
}
/**
 * @brief 写入水平对齐的单元格
 * @param xlsx Excel文档对象引用
 * @param cell 单元格地址（如"A1"）
 * @param text 要写入的文本内容
 * @param align 水平对齐方式
 * 
 * 设置单元格的水平对齐方式和边框样式
 */
void SearchThread::writeHorizontalAlignCell(Document &xlsx, const QString &cell, const QString &text,
                              Format::HorizontalAlignment align)
{
    Format format;
    format.setHorizontalAlignment(align);  // 设置水平对齐
    format.setBorderStyle(Format::BorderThin);  // 设置边框样式
    xlsx.write(cell, text, format);
}

/**
 * @brief 写入垂直对齐的合并单元格
 * @param xlsx Excel文档对象引用
 * @param range 单元格范围（如"A1:C1"）
 * @param text 要写入的文本内容
 * @param align 垂直对齐方式
 * 
 * 在指定范围内合并单元格并设置垂直对齐
 */
void SearchThread::writeVerticalAlignCell(Document &xlsx, const QString &range, const QString &text,
                            Format::VerticalAlignment align)
{
    Format format;
    format.setVerticalAlignment(align);  // 设置垂直对齐
    format.setBorderStyle(Format::BorderThin);  // 设置边框样式
    CellRange r(range);  // 创建单元格范围对象
    xlsx.write(r.firstRow(), r.firstColumn(), text);  // 在第一个单元格写入文本
    xlsx.mergeCells(r, format);  // 合并单元格并应用格式
}

/**
 * @brief 写入带指定边框样式的单元格
 * @param xlsx Excel文档对象引用
 * @param cell 单元格地址（如"A1"）
 * @param text 要写入的文本内容
 * @param bs 边框样式枚举值
 */
void SearchThread::writeBorderStyleCell(Document &xlsx, const QString &cell, const QString &text,
                          Format::BorderStyle bs)
{
    Format format;
    format.setBorderStyle(bs);  // 设置边框样式
    xlsx.write(cell, text, format);
}

/**
 * @brief 写入纯色填充的单元格
 * @param xlsx Excel文档对象引用
 * @param cell 单元格地址（如"A1"）
 * @param color 填充颜色
 */
void SearchThread::writeSolidFillCell(Document &xlsx, const QString &cell, const QColor &color)
{
    Format format;
    format.setPatternBackgroundColor(color);  // 设置背景颜色
    xlsx.write(cell, QVariant(), format);  // 写入空值，仅应用格式
}

/**
 * @brief 写入图案填充的单元格
 * @param xlsx Excel文档对象引用
 * @param cell 单元格地址（如"A1"）
 * @param pattern 填充图案类型
 * @param color 图案颜色
 */
void SearchThread::writePatternFillCell(Document &xlsx, const QString &cell, Format::FillPattern pattern,
                          const QColor &color)
{
    Format format;
    format.setPatternForegroundColor(color);  // 设置图案前景颜色
    format.setFillPattern(pattern);  // 设置填充图案
    xlsx.write(cell, QVariant(), format);  // 写入空值，仅应用格式
}

/**
 * @brief 写入带边框和字体颜色的单元格
 * @param xlsx Excel文档对象引用
 * @param cell 单元格地址（如"A1"）
 * @param text 要写入的文本内容
 * @param color 边框和字体颜色
 * 
 * 同时设置边框颜色和字体颜色为相同颜色
 */
void SearchThread::writeBorderAndFontColorCell(Document &xlsx, const QString &cell, const QString &text,
                                 const QColor &color)
{
    Format format;
    format.setBorderStyle(Format::BorderThin);  // 设置细边框
    format.setBorderColor(color);  // 设置边框颜色
    format.setFontColor(color);  // 设置字体颜色
    xlsx.write(cell, text, format);
}

/**
 * @brief 写入指定字体名称的单元格
 * @param xlsx Excel文档对象引用
 * @param cell 单元格地址（如"A1"）
 * @param text 要写入的文本内容（同时作为字体名称）
 * 
 * 使用文本内容作为字体名称，字体大小设为16
 */
void SearchThread::writeFontNameCell(Document &xlsx, const QString &cell, const QString &text)
{
    Format format;
    format.setFontName(text);  // 设置字体名称为文本内容
    format.setFontSize(16);  // 设置字体大小
    xlsx.write(cell, text, format);
}

/**
 * @brief 写入指定字体大小的单元格
 * @param xlsx Excel文档对象引用
 * @param cell 单元格地址（如"A1"）
 * @param size 字体大小（点数）
 * 
 * 在指定单元格写入"Qt Xlsx"文本，并设置指定字体大小
 */
void SearchThread::writeFontSizeCell(Document &xlsx, const QString &cell, int size)
{
    Format format;
    format.setFontSize(size);  // 设置字体大小
    xlsx.write(cell, "Qt Xlsx", format);  // 写入固定文本
}

/**
 * @brief 写入带内置数字格式的单元格
 * @param xlsx Excel文档对象引用
 * @param row 行号（从1开始）
 * @param value 要写入的数值
 * @param numFmt 内置数字格式索引
 * 
 * 在指定行的第1列写入原始数值，第2列写入格式描述，第3列写入带格式的数值
 */
void SearchThread::writeInternalNumFormatsCell(Document &xlsx, int row, double value, int numFmt)
{
    Format format;
    format.setNumberFormatIndex(numFmt);  // 设置内置数字格式索引
    xlsx.write(row, 1, value);  // 第1列：原始数值
    xlsx.write(row, 2, QString("Builtin NumFmt %1").arg(numFmt));  // 第2列：格式描述
    xlsx.write(row, 3, value, format);  // 第3列：带格式的数值
}

/**
 * @brief 写入带自定义数字格式的单元格
 * @param xlsx Excel文档对象引用
 * @param row 行号（从1开始）
 * @param value 要写入的数值
 * @param numFmt 自定义数字格式字符串
 * 
 * 在指定行的第1列写入原始数值，第2列写入格式字符串，第3列写入带格式的数值
 */
void SearchThread::writeCustomNumFormatsCell(Document &xlsx, int row, double value, const QString &numFmt)
{
    Format format;
    format.setNumberFormat(numFmt);  // 设置自定义数字格式
    xlsx.write(row, 1, value);  // 第1列：原始数值
    xlsx.write(row, 2, numFmt);  // 第2列：格式字符串
    xlsx.write(row, 3, value, format);  // 第3列：带格式的数值
}

















/**
 * @brief 线程主执行函数
 * 
 * 负责执行实际的搜索任务：
 * 1. 遍历所有指定的Excel文件
 * 2. 在每个文件中搜索包含关键词的单元格
 * 3. 发送进度更新信号
 * 4. 处理错误情况并发送错误信号
 * 5. 汇总所有搜索结果并发送完成信号
 * 
 * @note 该函数在线程池中异步执行，不会阻塞主线程
 */
void SearchThread::run() {
    QList<SearchResult> allResults;  // 存储所有搜索结果
    
    qDebug() << "搜索线程开始执行，当前线程地址:" << QThread::currentThread();
    qDebug() << "正在搜索" << m_fileNames.size() << "个文件中包含'" << m_searchText << "'的单元格...";
    
    int processedFiles = 0;  // 已处理的文件数量
    
    // 遍历所有指定的Excel文件
    for (const QString &fileName : m_fileNames) {
        try {
            qDebug() << "正在搜索文件:" << fileName;
            
            // 打开Excel文档
            Document doc(fileName);
            // 在当前文件中执行搜索
            QList<SearchResult> fileResults = searchInWorkbook(doc, fileName, m_searchText);
            // 将搜索结果添加到总结果中
            allResults.append(fileResults);
            
            qDebug() << "在文件" << fileName << "中找到" << fileResults.size() << "个匹配项";
            
            processedFiles++;  // 更新已处理文件计数
            // 发送进度更新信号
            emit searchProgress(processedFiles, m_fileNames.size(), fileName);
            
        } catch (const std::exception &e) {
            // 捕获标准异常并发送错误信号
            QString errorMsg = QString("处理文件 %1 时发生错误: %2").arg(fileName, e.what());
            qDebug() << errorMsg;
            emit searchError(errorMsg);
        } catch (...) {
            // 捕获所有其他异常
            QString errorMsg = QString("处理文件 %1 时发生未知错误").arg(fileName);
            qDebug() << errorMsg;
            emit searchError(errorMsg);
        }
    }
    
    qDebug() << "搜索完成，总共找到" << allResults.size() << "个匹配的单元格";

    // 发送搜索完成信号，带上所有结果
    emit searchFinished(allResults);
}








/**
 * @brief 在单个Excel工作簿中搜索关键词
 * @param doc Excel文档对象引用
 * @param fileName 文件路径（用于结果标识）
 * @param searchText 要搜索的关键词
 * @return 搜索结果列表
 * 
 * 遍历Excel文件中的所有工作表，在每个工作表的所有单元格中搜索包含关键词的内容
 * 搜索不区分大小写，只要单元格内容包含关键词即为匹配
 */
QList<SearchResult> SearchThread::searchInWorkbook(Document &doc, const QString &fileName, const QString &searchText)
{
    QList<SearchResult> results;  // 存储该文件的搜索结果

    // 获取所有工作表名称
    QStringList sheetNames = doc.sheetNames();

    // 遍历所有工作表
    for (const QString &sheetName : sheetNames) {
        // 选择当前工作表
        if (!doc.selectSheet(sheetName)) {
            qDebug() << "无法选择工作表:" << sheetName;
            continue;
        }

        // 获取当前工作表指针
        AbstractSheet *sheet = doc.currentSheet();
        if (!sheet) {
            continue;  // 工作表为空，跳过
        }

        // 检查是否为工作表类型（而不是图表等其他类型）
        if (sheet->sheetType() != AbstractSheet::ST_WorkSheet) {
            continue;
        }

        // 转换为工作表指针
        Worksheet *worksheet = static_cast<Worksheet*>(sheet);

        // 获取工作表的使用范围（即实际包含数据的区域）
        CellRange usedRange = worksheet->dimension();

        if (!usedRange.isValid()) {
            continue;  // 没有有效数据范围，跳过
        }

        // 遍历所有使用的单元格（按行列遍历）
        for (int row = usedRange.firstRow(); row <= usedRange.lastRow(); ++row) {
            for (int col = usedRange.firstColumn(); col <= usedRange.lastColumn(); ++col) {
                // 获取指定位置的单元格
                Cell *cell = worksheet->cellAt(row, col);
                if (!cell) {
                    continue;  // 单元格为空，跳过
                }

                // 获取单元格的值并转换为字符串
                QVariant value = cell->value();
                QString cellText = value.toString();

                // 检查单元格内容是否包含搜索文本（不区分大小写）
                if (cellText.contains(searchText, Qt::CaseInsensitive)) {
                    // 创建搜索结果对象
                    SearchResult result;
                    result.fileName = fileName;          // 文件名
                    result.sheetName = sheetName;        // 工作表名
                    result.cellReference = CellReference(row, col).toString();  // 单元格引用（如A1）
                    result.cellValue = value;            // 单元格值
                    results.append(result);              // 添加到结果列表
                }
            }
        }
    }

    return results;  // 返回该文件的所有搜索结果
}

/**
 * @brief 在多个Excel文件中搜索关键词（同步方式）
 * @param fileNames Excel文件路径列表
 * @param searchText 要搜索的关键词
 * @return 所有文件的搜索结果汇总
 * 
 * @note 该函数为同步执行，不支持进度报告和错误处理
 *       建议使用run()函数进行异步搜索
 */
QList<SearchResult> SearchThread::searchInMultipleFiles(const QStringList &fileNames, const QString &searchText)
{
    QList<SearchResult> allResults;  // 所有文件的搜索结果汇总

    // 逐个处理每个文件
    for (const QString &fileName : fileNames) {
        qDebug() << "正在搜索文件:" << fileName;

        // 打开Excel文档
        Document doc(fileName);
        // 注释掉的代码：早期QtXlsx版本可能需要显式调用load()
        /*
        if (!doc.load()) {
            qDebug() << "无法加载文件:" << fileName;
            continue;
        }*/

        // 在当前文件中搜索
        QList<SearchResult> fileResults = searchInWorkbook(doc, fileName, searchText);
        // 将结果添加到总结果中
        allResults.append(fileResults);

        qDebug() << "在文件" << fileName << "中找到" << fileResults.size() << "个匹配项";
    }

    return allResults;  // 返回所有结果
}
/**
 * @brief 公共搜索接口（同步方式）
 * @param fileNames Excel文件路径列表
 * @param searchText 要搜索的关键词
 * @return 所有文件的搜索结果氇总
 * 
 * 提供一个简单的同步搜索接口，内部调用searchInMultipleFiles
 * 并输出详细的搜索进度和结果统计信息
 */
QList<SearchResult>  SearchThread::Search( QStringList fileNames,QString searchText)
{
        qDebug() << "正在搜索" << fileNames.size() << "个文件中包含'" << searchText << "'的单元格...";
    

        QList<SearchResult> allResults;  // 存储所有搜索结果
        // 执行实际搜索操作
        allResults = searchInMultipleFiles(fileNames, searchText);
        
        // 根据结果输出不同的信息
        if (allResults.isEmpty()) {
            qDebug() << "在所有文件中未找到包含'" << searchText << "'的单元格";
            return allResults;
        } else {
            qDebug() << "在所有文件中总共找到" << allResults.size() << "个匹配的单元格:";

            return allResults;
        }
}

/**
 * @brief 启动异步搜索线程
 * @param fileNames Excel文件路径列表
 * @param searchText 要搜索的关键词
 * @param receiver 信号接收器对象（可为空）
 * 
 * 配置搜索参数并启动异步搜索任务：
 * 1. 设置搜索文件列表和关键词
 * 2. 如果提供了接收器，则连接相关信号
 * 3. 将任务提交到全局线程池执行
 * 
 * @note 该函数立即返回，任务在后台线程池中异步执行
 */
void SearchThread::SearchThreaded(const QStringList &fileNames, const QString &searchText, QObject *receiver)
{
    qDebug() << "启动搜索线程...";

    // 设置搜索参数
    this->setFileNames(fileNames);
    this->setSearchText(searchText);

    // 如果指定了接收器对象，则连接相关信号
    if (receiver) {
        // 连接搜索完成信号
        QObject::connect(this, &SearchThread::searchFinished,
                        receiver, [receiver](const QList<SearchResult> &results) {
            qDebug() << "搜索完成，结果数量:" << results.size();
            // 这里可以发送自定义信号或调用接收器的方法
        });

        // 连接搜索进度信号
        QObject::connect(this, &SearchThread::searchProgress,
                        receiver, [](int processed, int total, const QString &currentFileName) {
            qDebug() << "搜索进度:" << processed << "/" << total << ", 当前处理文件:" << currentFileName;
        });

        // 连接搜索错误信号
        QObject::connect(this, &SearchThread::searchError,
                        receiver, [](const QString &error) {
            qDebug() << "搜索错误:" << error;
        });
    }

    // 使用全局线程池执行任务
    QThreadPool::globalInstance()->start(this);

    qDebug() << "搜索任务已提交到线程池";
}

/**
 * @brief 将树控件中的搜索结果导出为Excel文件
 * @param treeWidget 包含搜索结果的树控件引用
 * @param exportFilePath 导出Excel文件的保存路径
 * @return 导出结果消息字符串
 * 
 * 将树控件中的搜索结果数据转换为Excel表格格式：
 * - 第一行为表头：源文件、工作表、单元格位置、单元格内容
 * - 后续行为实际的搜索结果数据
 * - 设置适当的表格格式和边框样式
 */
 QString SearchThread::exportTreeWidgetToExcel(QTreeWidget &treeWidget, const QString &exportFilePath)
{
    Document exportDoc;  // 创建新的Excel文档

    // 设置Excel文件的表头（第一行）
    exportDoc.write(1, 1, "源文件");      // A1: 文件名列
    exportDoc.write(1, 2, "工作表");      // B1: 工作表名列
    exportDoc.write(1, 3, "单元格位置");  // C1: 单元格引用列
    exportDoc.write(1, 4, "单元格内容");  // D1: 单元格内容列

    // 设置表头格式（粗体、灰色背景、边框）
    Format headerFormat;
    headerFormat.setFontBold(true);  // 设置粗体
    headerFormat.setPatternBackgroundColor(QColor(200, 200, 200));  // 设置灰色背景
    headerFormat.setBorderStyle(Format::BorderThin);  // 设置细边框

    // 为表头的四个列应用格式
    for (int col = 1; col <= 4; ++col) {
        exportDoc.write(1, col, exportDoc.read(1, col), headerFormat);
    }

    int currentRow = 2;  // 从第2行开始写入实际数据

    // 遍历树控件的所有顶级项目（每个顶级项目代表一个文件）
    for (int i = 0; i < treeWidget.topLevelItemCount(); ++i) {
        QTreeWidgetItem *fileItem = treeWidget.topLevelItem(i);
        QString fileName = fileItem->text(0);

        // 处理文件名显示格式，移除"文件: "前缀
        if (fileName.startsWith("文件: ")) {
            fileName = fileName.mid(4);  // 去掉前4个字符
        }

        // 遍历当前文件下的所有子项目（每个子项目代表一个搜索结果）
        for (int j = 0; j < fileItem->childCount(); ++j) {
            QTreeWidgetItem *resultItem = fileItem->child(j);

            // 获取工作表名并处理显示格式
            QString sheetName = resultItem->text(0);
            if (sheetName.startsWith("工作表: ")) {
                sheetName = sheetName.mid(5);  // 移除"工作表: "前缀
            }

            // 获取单元格引用和内容
            QString cellReference = resultItem->text(1);  // 如"A1", "B5"等
            QString cellValue = resultItem->text(2);      // 单元格实际内容

            // 在Excel中写入一行数据
            exportDoc.write(currentRow, 1, fileName);      // A列: 文件名
            exportDoc.write(currentRow, 2, sheetName);     // B列: 工作表名
            exportDoc.write(currentRow, 3, cellReference); // C列: 单元格位置
            exportDoc.write(currentRow, 4, cellValue);     // D列: 单元格内容

            // 为数据行设置格式（仅边框）
            Format dataFormat;
            dataFormat.setBorderStyle(Format::BorderThin);

            // 为当前行的所有列应用数据格式
            for (int col = 1; col <= 4; ++col) {
                exportDoc.write(currentRow, col, exportDoc.read(currentRow, col), dataFormat);
            }

            currentRow++;  // 移动到下一行
        }
    }

    // 自动调整列宽（注释掉的代码，可根需要启用）
    // 这些代码用于设置各列的宽度，优化显示效果
    //exportDoc.currentWorksheet()->setColumnWidth(1, 30); // 源文件列宽度
    //exportDoc.currentWorksheet()->setColumnWidth(2, 15); // 工作表列宽度
    //exportDoc.currentWorksheet()->setColumnWidth(3, 10); // 单元格位置列宽度
    //exportDoc.currentWorksheet()->setColumnWidth(4, 25); // 单元格内容列宽度

    // 尝试保存Excel文件到指定路径
    if (exportDoc.saveAs(exportFilePath)) {
        // 保存成功，返回成功消息
        return "搜索结果已成功导出到:" + exportFilePath;
    } else {
        // 保存失败，返回失败消息
        return "导出失败!";
    }
}
