/**
 * @file WatermarkController.h
 * @brief 水印控制器类
 * 
 * 负责处理PDF文档的水印添加、预览和相关参数设置功能
 */

#ifndef WATERMARKCONTROLLER_H
#define WATERMARKCONTROLLER_H

#include <QObject>
#include <QDir>
#include <QThreadPool>
#include <QMutex>
#include "../../function.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class watermarkThreadSingle;
class multiWatermarkThreadSingle;

/**
 * @brief 水印控制器类
 * 
 * 管理PDF文档的水印添加、预览、颜色选择、透明度和旋转角度等功能
 */
class WatermarkController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param ui 主窗口UI指针
     * @param threadPool 线程池引用
     * @param parent 父对象
     */
    explicit WatermarkController(Ui::MainWindow *ui, QThreadPool &threadPool, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~WatermarkController();

    /**
     * @brief 初始化控制器
     * 
     * 设置信号槽连接和初始状态
     */
    void initialize();

public slots:
    /**
     * @brief 水印预览功能
     * 
     * 实时更新PDF文档的水印预览效果
     */
    void viewWatermark();
    
    /**
     * @brief 添加水印按钮点击处理
     * 
     * 批量为PDF文件添加水印
     */
    void onBtnAddWaterClicked();
    
    /**
     * @brief 颜色选择按钮点击处理
     * 
     * 弹出颜色选择对话框
     */
    void onBtnColorSelectClicked();
    
    /**
     * @brief 字体选择变更处理
     * 
     * 更新水印字体并刷新预览
     */
    void onFontChanged();
    
    /**
     * @brief 批量添加水印
     * @param text 水印文本
     * @param inputDir 输入目录
     * @param outputDir 输出目录
     * @param color 水印颜色
     * @param opacity 透明度
     * @param rotate 旋转角度
     * @param font 字体
     * @param fontSize 字体大小
     */
    void addWatermarkSingle(QString text, QString inputDir, QString outputDir,
                           QString color, QString opacity, QString rotate,
                           QString font, QString fontSize);

signals:
    /**
     * @brief 水印添加完成信号
     * @param message 完成消息
     */
    void watermarkCompleted(const QString &message);

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
     * @brief 导出按钮状态变更信号
     * @param enabled 是否启用
     */
    void exportButtonStateChanged(bool enabled);

private:
    /**
     * @brief 验证水印输入参数
     * @return 验证是否通过
     */
    bool validateWatermarkInput();
    
    /**
     * @brief 获取当前选择的字体
     * @return 字体名称
     */
    QString getCurrentFont() const;

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

private:
    Ui::MainWindow *m_ui;                           ///< 主窗口UI指针
    QThreadPool &m_threadPool;                      ///< 线程池引用
    watermarkThreadSingle *m_wmThreadSingle;        ///< 单行水印线程
    multiWatermarkThreadSingle *m_mwmThreadSingle;  ///< 多行水印线程
    QMutex m_mutex;                                 ///< 互斥锁
};

#endif // WATERMARKCONTROLLER_H