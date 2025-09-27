/**
 * @file ExcelSearchController.h
 * @brief Excel搜索控制器类
 * 
 * 负责处理Excel文件内容的搜索功能
 */

#ifndef EXCELSEARCHCONTROLLER_H
#define EXCELSEARCHCONTROLLER_H

#include <QObject>
#include <QString>
#include <QList>
#include "../../include/search/SearchThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE

/**
 * @brief Excel搜索控制器类
 * 
 * 管理Excel文件的搜索、结果显示和导出功能
 */
class ExcelSearchController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param ui 主窗口UI指针
     * @param parent 父对象
     */
    explicit ExcelSearchController(Ui::MainWindow *ui, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ExcelSearchController();

    /**
     * @brief 初始化控制器
     * 
     * 设置初始状态和连接信号
     */
    void initialize();

public slots:
    /**
     * @brief 选择搜索目录按钮点击处理
     */
    void onBtnSelectInputSearchClicked();
    
    /**
     * @brief Tab页切换处理
     * @param index 当前Tab索引
     */
    void onTabWidgetCurrentChanged(int index);
    
    /**
     * @brief 搜索按钮点击处理
     */
    void onBtnSearchClicked();
    
    /**
     * @brief 导出搜索结果按钮点击处理
     */
    void onBtnSearchExportClicked();
    
    /**
     * @brief 搜索完成槽函数
     * @param results 搜索结果列表
     */
    void onSearchFinished(const QList<SearchResult> &results);
    
    /**
     * @brief 搜索进度槽函数
     * @param processed 已处理文件数
     * @param total 总文件数
     * @param currentFileName 当前处理的文件名
     */
    void onSearchProgress(int processed, int total, const QString &currentFileName);
    
    /**
     * @brief 搜索错误槽函数
     * @param error 错误信息
     */
    void onSearchError(const QString &error);

signals:
    /**
     * @brief 日志消息信号
     * @param message 日志内容
     */
    void logMessage(const QString &message);

private:
    /**
     * @brief 检查树形控件是否为空
     * @param treeWidget 树形控件指针
     * @return 是否为空
     */
    bool isTreeWidgetEmpty(QTreeWidget *treeWidget);
    
    /**
     * @brief 导出树形控件内容到Excel
     * @param treeWidget 树形控件引用
     * @param exportFilePath 导出文件路径
     * @return 导出结果信息
     */
    QString exportTreeWidgetToExcel(QTreeWidget &treeWidget, const QString &exportFilePath);
    
    /**
     * @brief 保存搜索目录到临时文件
     * @param directory 目录路径
     */
    void saveSearchDirectory(const QString &directory);
    
    /**
     * @brief 从临时文件加载搜索目录
     * @return 搜索目录路径
     */
    QString loadSearchDirectory();
    
    /**
     * @brief 清空并初始化搜索结果树
     */
    void initializeSearchTree();
    
    /**
     * @brief 验证搜索输入参数
     * @return 是否验证通过
     */
    bool validateSearchInput();

private:
    Ui::MainWindow *m_ui;  ///< 主窗口UI指针
};

#endif // EXCELSEARCHCONTROLLER_H