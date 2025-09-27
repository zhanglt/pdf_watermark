/**
 * @file PdfViewerController.h
 * @brief PDF查看器控制器类
 * 
 * 负责处理PDF文档的查看、导航和缩放等功能
 */

#ifndef PDFVIEWERCONTROLLER_H
#define PDFVIEWERCONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QPdfDocument>
#include <QPdfView>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ZoomSelector;
class PageSelector;

/**
 * @brief PDF查看器控制器类
 * 
 * 管理PDF文档的打开、查看、缩放和页面导航等功能
 */
class PdfViewerController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param ui 主窗口UI指针
     * @param parent 父对象
     */
    explicit PdfViewerController(Ui::MainWindow *ui, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~PdfViewerController();

    /**
     * @brief 初始化控制器
     * 
     * 设置PDF查看器、创建缩放和页面选择器等
     */
    void initialize();

    /**
     * @brief 获取PDF文档对象
     * @return PDF文档指针
     */
    QPdfDocument* document() const { return m_document; }

    /**
     * @brief 获取缩放选择器
     * @return 缩放选择器指针
     */
    ZoomSelector* zoomSelector() const { return m_zoomSelector; }

    /**
     * @brief 获取页面选择器
     * @return 页面选择器指针
     */
    PageSelector* pageSelector() const { return m_pageSelector; }

public slots:
    /**
     * @brief 打开PDF文档
     * @param docLocation PDF文件的URL路径
     * @param err 返回的错误信息
     */
    void open(const QUrl &docLocation, QPdfDocument::DocumentError &err);
    
    /**
     * @brief 书签选中事件处理
     * @param index 选中的书签索引
     */
    void bookmarkSelected(const QModelIndex &index);
    
    /**
     * @brief 打开文件动作处理
     */
    void onActionOpenTriggered();
    
    /**
     * @brief 放大PDF显示
     */
    void onActionZoomInTriggered();
    
    /**
     * @brief 缩小PDF显示
     */
    void onActionZoomOutTriggered();
    
    /**
     * @brief 跳转到上一页
     */
    void onActionPreviousPageTriggered();
    
    /**
     * @brief 跳转到下一页
     */
    void onActionNextPageTriggered();
    
    /**
     * @brief 切换PDF的页面显示模式
     */
    void onActionContinuousTriggered();

signals:
    /**
     * @brief 文档已打开信号
     * @param title 文档标题
     */
    void documentOpened(const QString &title);

    /**
     * @brief 打开文档失败信号
     * @param error 错误信息
     */
    void openFailed(const QString &error);

private:
    /**
     * @brief 设置PDF查看器背景颜色
     */
    void setupViewerStyle();

private:
    Ui::MainWindow *m_ui;               ///< 主窗口UI指针
    QPdfDocument *m_document;           ///< PDF文档对象
    ZoomSelector *m_zoomSelector;       ///< 缩放选择器
    PageSelector *m_pageSelector;       ///< 页面选择器
    
    static const qreal zoomMultiplier;  ///< 缩放倍数常量
};

#endif // PDFVIEWERCONTROLLER_H