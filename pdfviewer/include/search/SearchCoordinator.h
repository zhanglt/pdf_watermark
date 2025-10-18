#ifndef SEARCHCOORDINATOR_H
#define SEARCHCOORDINATOR_H

#include <QObject>
#include <QStringList>
#include <QMutex>
#include <QAtomicInt>
#include "SearchThread.h"

/**
 * @brief Excel搜索协调器类
 *
 * 该类负责协调多个SearchThread并行搜索大量Excel文件：
 * - 根据CPU核心数自动分割文件列表
 * - 为每个批次创建独立的SearchThread线程
 * - 聚合所有线程的搜索结果
 * - 统一管理进度更新和错误处理
 *
 * 使用多线程并行搜索可以显著提升大文件列表的搜索性能。
 */
class SearchCoordinator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit SearchCoordinator(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SearchCoordinator();

    /**
     * @brief 开始多线程搜索
     * @param fileNames 要搜索的文件列表
     * @param searchText 搜索关键字
     * @param maxThreads 最大线程数（0表示自动根据CPU核心数决定）
     *
     * 该方法会自动分割文件列表并启动多个搜索线程
     */
    void startSearch(const QStringList &fileNames, const QString &searchText, int maxThreads = 0);

signals:
    /**
     * @brief 搜索完成信号
     * @param results 所有线程汇总的搜索结果
     *
     * 当所有线程完成搜索后发出此信号
     */
    void searchFinished(const QList<SearchResult> &results);

    /**
     * @brief 搜索进度信号
     * @param processedFiles 已处理的文件数
     * @param totalFiles 总文件数
     * @param currentFileName 当前正在处理的文件名
     *
     * 汇总所有线程的进度并发出统一的进度信号
     */
    void searchProgress(int processedFiles, int totalFiles, const QString &currentFileName);

    /**
     * @brief 搜索错误信号
     * @param errorMessage 错误信息
     *
     * 当任意线程发生错误时发出此信号
     */
    void searchError(const QString &errorMessage);

private slots:
    /**
     * @brief 处理单个线程完成的槽函数
     * @param results 该线程的搜索结果
     */
    void onThreadFinished(const QList<SearchResult> &results);

    /**
     * @brief 处理线程进度更新的槽函数
     * @param processed 该线程已处理的文件数
     * @param total 该线程的总文件数
     * @param currentFileName 当前文件名
     */
    void onThreadProgress(int processed, int total, const QString &currentFileName);

    /**
     * @brief 处理线程错误的槽函数
     * @param error 错误信息
     */
    void onThreadError(const QString &error);

private:
    /**
     * @brief 将文件列表分割成多个批次
     * @param fileNames 原始文件列表
     * @param batchCount 要分割的批次数量
     * @return 分割后的文件列表数组
     */
    QVector<QStringList> splitFileList(const QStringList &fileNames, int batchCount);

    /**
     * @brief 获取最优线程数
     * @param fileCount 文件总数
     * @param requestedThreads 请求的线程数（0表示自动）
     * @return 实际使用的线程数
     */
    int getOptimalThreadCount(int fileCount, int requestedThreads);

private:
    QMutex m_resultsMutex;                    ///< 保护结果列表的互斥锁
    QList<SearchResult> m_allResults;         ///< 汇总的所有搜索结果

    QAtomicInt m_completedThreads;            ///< 已完成的线程数
    int m_totalThreads;                       ///< 总线程数

    QMutex m_progressMutex;                   ///< 保护进度计数器的互斥锁
    int m_totalFiles;                         ///< 总文件数
    int m_processedFiles;                     ///< 已处理的文件数
};

#endif // SEARCHCOORDINATOR_H
