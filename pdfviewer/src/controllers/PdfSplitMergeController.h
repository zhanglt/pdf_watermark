/**
 * @file PdfSplitMergeController.h
 * @brief PDF拆分合并控制器类
 * 
 * 负责处理PDF文档的拆分和合并功能
 */

#ifndef PDFSPLITMERGECONTROLLER_H
#define PDFSPLITMERGECONTROLLER_H

#include <QObject>
#include <QString>
#include "../../function.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QTableWidget;
QT_END_NAMESPACE

/**
 * @brief PDF拆分合并控制器类
 * 
 * 管理PDF文档的拆分、合并、页面范围选择等功能
 */
class PdfSplitMergeController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param ui 主窗口UI指针
     * @param parent 父对象
     */
    explicit PdfSplitMergeController(Ui::MainWindow *ui, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~PdfSplitMergeController();

    /**
     * @brief 初始化控制器
     * 
     * 初始化表格和信号连接
     */
    void initialize();

public slots:
    // 拆分功能相关槽函数
    /**
     * @brief 选择拆分文件按钮点击处理
     */
    void onBtnSelectFileSplitClicked();
    
    /**
     * @brief 拆分文件输入框文本变化处理
     * @param filename 文件名
     */
    void onSplitFileTextChanged(const QString &filename);
    
    /**
     * @brief 选择拆分输出目录按钮点击处理
     */
    void onBtnSelectSplitDirClicked();
    
    /**
     * @brief 拆分输出目录文本变化处理
     * @param filepath 目录路径
     */
    void onSplitOutputTextChanged(const QString &filepath);
    
    /**
     * @brief 拆分PDF按钮点击处理
     */
    void onBtnSplitPdfClicked();
    
    // 合并功能相关槽函数
    /**
     * @brief 添加文件到合并列表按钮点击处理
     */
    void onBtnAddFileClicked();
    
    /**
     * @brief 选择合并输出目录按钮点击处理
     */
    void onBtnSelectMergeDirClicked();
    
    /**
     * @brief 合并PDF按钮点击处理
     */
    void onBtnMergeClicked();

signals:
    /**
     * @brief 请求打开PDF文件信号
     * @param url 文件URL
     */
    void requestOpenPdf(const QUrl &url);
    
    /**
     * @brief 请求设置缩放模式信号
     */
    void requestFitToWidth();
    
    /**
     * @brief 日志消息信号
     * @param message 日志内容
     */
    void logMessage(const QString &message);

private:
    /**
     * @brief 初始化合并文件表格
     */
    void initTable();
    
    /**
     * @brief 验证拆分输入参数
     * @return 是否验证通过
     */
    bool validateSplitInput();
    
    /**
     * @brief 验证合并输入参数
     * @return 是否验证通过
     */
    bool validateMergeInput();
    
    /**
     * @brief 执行范围拆分
     * @param input 输入文件
     * @param output 输出文件前缀
     * @param pages 总页数
     * @param range 页面范围字符串
     */
    void performRangeSplit(const std::string &input, const std::string &output, 
                          int pages, const QString &range);
    
    /**
     * @brief 执行页数拆分
     * @param input 输入文件
     * @param output 输出文件前缀
     * @param pages 总页数
     */
    void performPageSplit(const std::string &input, const std::string &output, 
                         int pages);

private:
    Ui::MainWindow *m_ui;  ///< 主窗口UI指针
};

#endif // PDFSPLITMERGECONTROLLER_H