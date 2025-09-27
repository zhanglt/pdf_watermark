/**
 * @file mainwindow_refactored.h
 * @brief 重构后的主窗口类
 * 
 * 简化后的MainWindow类，主要负责UI初始化和协调各控制器
 */

#ifndef MAINWINDOW_REFACTORED_H
#define MAINWINDOW_REFACTORED_H

#include <QMainWindow>
#include <QThreadPool>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 前向声明控制器类
class PdfViewerController;
class WatermarkController;
class PdfConverterController;
class PdfSplitMergeController;
class ExcelSearchController;

/**
 * @brief 重构后的主窗口类
 * 
 * 负责：
 * - UI初始化
 * - 创建和管理控制器
 * - 协调控制器之间的通信
 * - 处理顶层菜单事件
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~MainWindow();

private slots:
    // 顶层菜单动作处理
    /**
     * @brief 退出应用程序
     */
    void on_actionQuit_triggered();
    
    /**
     * @brief 显示关于对话框
     */
    void on_actionAbout_triggered();
    
    /**
     * @brief 显示关于Qt对话框
     */
    void on_actionAbout_Qt_triggered();

    /**
     * @brief 处理文档打开成功
     * @param title 文档标题
     */
    void onDocumentOpened(const QString &title);

    /**
     * @brief 更新日志
     * @param message 日志消息
     */
    void updateLog(const QString &message);

private:
    /**
     * @brief 初始化UI
     */
    void initializeUI();
    
    /**
     * @brief 创建控制器
     */
    void createControllers();
    
    /**
     * @brief 连接控制器信号
     */
    void connectControllers();
    
    /**
     * @brief 连接UI信号到控制器
     */
    void connectUIToControllers();
    
    /**
     * @brief 设置工具栏
     */
    void setupToolBar();

private:
    Ui::MainWindow *ui;                              ///< UI指针
    QThreadPool m_threadPool;                        ///< 线程池
    QLabel *m_titleLabel;                            ///< 标题标签
    
    // 控制器
    PdfViewerController *m_pdfViewerController;      ///< PDF查看控制器
    WatermarkController *m_watermarkController;      ///< 水印控制器
    PdfConverterController *m_pdfConverterController; ///< PDF转换控制器
    PdfSplitMergeController *m_splitMergeController; ///< PDF拆分合并控制器
    ExcelSearchController *m_excelSearchController;   ///< Excel搜索控制器
};

#endif // MAINWINDOW_REFACTORED_H