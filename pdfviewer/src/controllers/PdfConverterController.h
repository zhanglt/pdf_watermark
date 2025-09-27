/**
 * @file PdfConverterController.h
 * @brief PDF转换控制器类
 * 
 * 负责处理PDF与图片格式之间的相互转换功能
 */

#ifndef PDFCONVERTERCONTROLLER_H
#define PDFCONVERTERCONTROLLER_H

#include <QObject>
#include <QDir>
#include <QThreadPool>
#include <QAtomicInt>
#include "../../function.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class pdf2imageThreadSingle;
class QProgressIndicator;

/**
 * @brief PDF转换控制器类
 * 
 * 管理PDF转图片、图片转PDF、批量转换等功能
 */
class PdfConverterController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param ui 主窗口UI指针
     * @param threadPool 线程池引用
     * @param parent 父对象
     */
    explicit PdfConverterController(Ui::MainWindow *ui, QThreadPool &threadPool, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~PdfConverterController();

    /**
     * @brief 初始化控制器
     * 
     * 设置初始状态
     */
    void initialize();

public slots:
    /**
     * @brief 选择图片文件按钮点击处理
     */
    void onBtnSelectImageFileClicked();
    
    /**
     * @brief 图片转PDF按钮点击处理
     */
    void onBtnTransformClicked();
    
    /**
     * @brief 选择图片目录按钮点击处理
     */
    void onBtnSelectImageDirClicked();
    
    /**
     * @brief 批量图片转PDF按钮点击处理
     */
    void onBtnTransformBatClicked();
    
    /**
     * @brief 选择PDF文件按钮点击处理
     */
    void onBtnSelectPDFFileClicked();
    
    /**
     * @brief PDF转图片按钮点击处理
     */
    void onBtnPdfToImageClicked();
    
    /**
     * @brief 导出PDF按钮点击处理
     * 
     * 将文字PDF转换为图片PDF
     */
    void onBtnExportPDFClicked();
    
    /**
     * @brief 导出PDF功能实现
     * @param pdfdir 包含PDF文件的目录
     */
    void exportPdf(QDir pdfdir);

signals:
    /**
     * @brief 转换完成信号
     */
    void Finished();
    
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
     * @brief 显示进度指示器
     */
    void showProgressIndicator();
    
    /**
     * @brief 验证图片文件
     * @param file 文件路径
     * @return 是否为有效的图片文件
     */
    bool validateImageFile(const QString &file);
    
    /**
     * @brief 验证PDF文件
     * @param file 文件路径
     * @return 是否为有效的PDF文件
     */
    bool validatePdfFile(const QString &file);

private:
    Ui::MainWindow *m_ui;                     ///< 主窗口UI指针
    QThreadPool &m_threadPool;                ///< 线程池引用
    pdf2imageThreadSingle *m_pdf2imageThread; ///< PDF转图片线程
    QAtomicInt m_completedCount;              ///< 已完成任务计数
    int m_totalFiles;                         ///< 总文件数
};

#endif // PDFCONVERTERCONTROLLER_H