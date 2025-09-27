#ifndef SEARCHTHREAD_H
#define SEARCHTHREAD_H

#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <QStringList>
#include <QString>
#include <QList>
#include "lib/qtxlsx/include/QtXlsx/xlsxdocument.h"
#include "lib/qtxlsx/include/QtXlsx/xlsxformat.h"
#include "lib/qtxlsx/include/QtXlsx/xlsxcellrange.h"
#include "lib/qtxlsx/include/QtXlsx/xlsxworksheet.h"
#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QThreadPool>
QTXLSX_USE_NAMESPACE

/**
 * @brief 搜索结果结构体
 * 
 * 用于存储在Excel文件中搜索到的结果信息
 */
struct SearchResult {
    QString fileName;      ///< 文件名
    QString sheetName;     ///< 工作表名称
    QString cellReference; ///< 单元格引用（如A1、B2等）
    QVariant cellValue;    ///< 单元格的值
};

Q_DECLARE_METATYPE(SearchResult)
Q_DECLARE_METATYPE(QList<SearchResult>)

/**
 * @brief Excel文件搜索线程类
 * 
 * 该类继承自QObject和QRunnable，支持在线程池中执行搜索任务。
 * 主要功能包括在Excel文件中搜索文本、导出搜索结果、格式化单元格等。
 */
class SearchThread : public QObject, public QRunnable {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit SearchThread(QObject *parent = nullptr);
    
    /**
     * @brief 设置要搜索的文件列表
     * @param fileNames 文件路径列表
     */
    void setFileNames(const QStringList &fileNames);
    
    /**
     * @brief 设置要搜索的文本
     * @param searchText 搜索关键字
     */
    void setSearchText(const QString &searchText);
    
    /**
     * @brief 线程执行函数
     * 
     * 重写QRunnable的run方法，在线程池中执行搜索任务
     */
    void run() override;

    /**
     * @brief 检查树形控件是否为空
     * @param treeWidget 树形控件指针
     * @return 如果为空返回true，否则返回false
     */
    bool isTreeWidgetEmpty(QTreeWidget *treeWidget);
    
    /**
     * @brief 将树形控件内容导出为Excel文件
     * @param treeWidget 包含搜索结果的树形控件
     * @param exportFilePath 导出文件的路径
     * @return 导出成功返回文件路径，失败返回空字符串
     */
    QString exportTreeWidgetToExcel(QTreeWidget &treeWidget, const QString &exportFilePath);
    
    /**
     * @brief 在单个工作簿中搜索文本
     * @param doc Excel文档对象
     * @param fileName 文件名
     * @param searchText 搜索文本
     * @return 搜索结果列表
     */
    QList<SearchResult> searchInWorkbook(Document &doc, const QString &fileName, const QString &searchText);
    
    /**
     * @brief 在多个文件中搜索文本
     * @param fileNames 文件名列表
     * @param searchText 搜索文本
     * @return 所有文件的搜索结果列表
     */
    QList<SearchResult> searchInMultipleFiles(const QStringList &fileNames, const QString &searchText);

    /**
     * @brief 执行搜索操作
     * @param fileNames 文件名列表
     * @param keyword 搜索关键字
     * @return 搜索结果列表
     */
    QList<SearchResult>  Search( QStringList fileNames,QString keyword);
    
    /**
     * @brief 在线程中执行搜索
     * @param fileNames 文件名列表
     * @param searchText 搜索文本
     * @param receiver 接收搜索结果的对象
     */
    void SearchThreaded(const QStringList &fileNames, const QString &searchText, QObject *receiver);
    
    /**
     * @brief 写入水平对齐的单元格
     * @param xlsx Excel文档
     * @param cell 单元格位置
     * @param text 文本内容
     * @param align 水平对齐方式
     */
    void writeHorizontalAlignCell(Document &xlsx, const QString &cell, const QString &text,
                                  Format::HorizontalAlignment align);

    /**
     * @brief 写入垂直对齐的单元格
     * @param xlsx Excel文档
     * @param range 单元格范围
     * @param text 文本内容
     * @param align 垂直对齐方式
     */
    void writeVerticalAlignCell(Document &xlsx, const QString &range, const QString &text,
                                Format::VerticalAlignment align);

    /**
     * @brief 写入带边框样式的单元格
     * @param xlsx Excel文档
     * @param cell 单元格位置
     * @param text 文本内容
     * @param bs 边框样式
     */
    void writeBorderStyleCell(Document &xlsx, const QString &cell, const QString &text,
                              Format::BorderStyle bs);
    
    /**
     * @brief 写入纯色填充的单元格
     * @param xlsx Excel文档
     * @param cell 单元格位置
     * @param color 填充颜色
     */
    void writeSolidFillCell(Document &xlsx, const QString &cell, const QColor &color);

    /**
     * @brief 写入图案填充的单元格
     * @param xlsx Excel文档
     * @param cell 单元格位置
     * @param pattern 填充图案
     * @param color 填充颜色
     */
    void writePatternFillCell(Document &xlsx, const QString &cell, Format::FillPattern pattern,
                          const QColor &color);
    
    /**
     * @brief 写入带边框和字体颜色的单元格
     * @param xlsx Excel文档
     * @param cell 单元格位置
     * @param text 文本内容
     * @param color 颜色
     */
    void writeBorderAndFontColorCell(Document &xlsx, const QString &cell, const QString &text,
                                 const QColor &color);

    /**
     * @brief 设置单元格字体
     * @param xlsx Excel文档
     * @param cell 单元格位置
     * @param text 文本内容
     */
    void writeFontNameCell(Document &xlsx, const QString &cell, const QString &text);

    /**
     * @brief 设置单元格字体大小
     * @param xlsx Excel文档
     * @param cell 单元格位置
     * @param size 字体大小
     */
    void writeFontSizeCell(Document &xlsx, const QString &cell, int size);

    /**
     * @brief 写入内置数字格式的单元格
     * @param xlsx Excel文档
     * @param row 行号
     * @param value 数值
     * @param numFmt 数字格式代码
     */
    void writeInternalNumFormatsCell(Document &xlsx, int row, double value, int numFmt);

    /**
     * @brief 写入自定义数字格式的单元格
     * @param xlsx Excel文档
     * @param row 行号
     * @param value 数值
     * @param numFmt 自定义数字格式字符串
     */
    void writeCustomNumFormatsCell(Document &xlsx, int row, double value, const QString &numFmt);









signals:
    /**
     * @brief 搜索完成信号
     * @param results 搜索结果列表
     * 
     * 当所有文件搜索完成后发出此信号
     */
    void searchFinished(const QList<SearchResult> &results);
    
    /**
     * @brief 搜索进度信号
     * @param processedFiles 已处理的文件数
     * @param totalFiles 总文件数
     * @param currentFileName 当前正在处理的文件名
     * 
     * 用于更新搜索进度界面
     */
    void searchProgress(int processedFiles, int totalFiles, const QString &currentFileName);
    
    /**
     * @brief 搜索错误信号
     * @param errorMessage 错误信息
     * 
     * 当搜索过程中发生错误时发出此信号
     */
    void searchError(const QString &errorMessage);

private:
    QMutex *m_mutex;           ///< 互斥锁，用于线程安全
    QStringList m_fileNames;   ///< 要搜索的文件名列表
    QString m_searchText;      ///< 搜索文本
};

#endif // SEARCHTHREAD_H
