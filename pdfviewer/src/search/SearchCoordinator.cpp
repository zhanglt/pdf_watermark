/**
 * @file SearchCoordinator.cpp
 * @brief Excel搜索协调器实现文件
 *
 * 实现多线程并行搜索Excel文件的协调器，提供以下功能：
 * - 智能分割文件列表到多个线程
 * - 汇总所有线程的搜索结果
 * - 统一管理进度和错误
 */

#pragma execution_character_set("utf-8")
#include "include/search/SearchCoordinator.h"
#include <QThread>
#include <QThreadPool>
#include <QDebug>

/**
 * @brief 构造函数
 */
SearchCoordinator::SearchCoordinator(QObject *parent)
    : QObject(parent)
    , m_completedThreads(0)
    , m_totalThreads(0)
    , m_totalFiles(0)
    , m_processedFiles(0)
{
}

/**
 * @brief 析构函数
 */
SearchCoordinator::~SearchCoordinator()
{
}

/**
 * @brief 获取最优线程数
 * @param fileCount 文件总数
 * @param requestedThreads 请求的线程数（0表示自动）
 * @return 实际使用的线程数
 *
 * 策略：
 * - 如果文件数很少（<4），只使用1个线程
 * - 否则使用 min(文件数, CPU核心数-1, 请求的线程数)
 * - 至少保留1个核心给UI线程，最少使用1个线程
 */
int SearchCoordinator::getOptimalThreadCount(int fileCount, int requestedThreads)
{
    if (fileCount <= 0) {
        return 1;
    }

    // 如果文件数很少，单线程就够了
    if (fileCount < 4) {
        return 1;
    }

    // 获取CPU核心数
    int cpuCores = QThread::idealThreadCount();
    if (cpuCores < 1) {
        cpuCores = 2; // 默认值，防止返回-1
    }

    // 保留至少1个核心给UI线程
    int maxThreads = qMax(1, cpuCores - 1);

    // 如果指定了线程数，使用指定值
    if (requestedThreads > 0) {
        maxThreads = qMin(maxThreads, requestedThreads);
    }

    // 线程数不能超过文件数（每个线程至少处理1个文件）
    return qMin(maxThreads, fileCount);
}

/**
 * @brief 将文件列表分割成多个批次
 * @param fileNames 原始文件列表
 * @param batchCount 要分割的批次数量
 * @return 分割后的文件列表数组
 *
 * 采用平均分配策略，尽量让每个批次的文件数相近
 */
QVector<QStringList> SearchCoordinator::splitFileList(const QStringList &fileNames, int batchCount)
{
    QVector<QStringList> batches;
    int totalFiles = fileNames.size();

    if (totalFiles == 0 || batchCount <= 0) {
        return batches;
    }

    // 如果批次数大于文件数，调整为文件数
    batchCount = qMin(batchCount, totalFiles);

    // 计算每个批次的基本文件数
    int filesPerBatch = totalFiles / batchCount;
    int remainder = totalFiles % batchCount; // 余数，前面的批次多分配一个

    int startIndex = 0;
    for (int i = 0; i < batchCount; ++i) {
        // 前 remainder 个批次多分配一个文件
        int currentBatchSize = filesPerBatch + (i < remainder ? 1 : 0);

        QStringList batch;
        for (int j = 0; j < currentBatchSize && startIndex < totalFiles; ++j) {
            batch.append(fileNames[startIndex++]);
        }

        if (!batch.isEmpty()) {
            batches.append(batch);
        }
    }

    return batches;
}

/**
 * @brief 开始多线程搜索
 * @param fileNames 要搜索的文件列表
 * @param searchText 搜索关键字
 * @param maxThreads 最大线程数（0表示自动）
 */
void SearchCoordinator::startSearch(const QStringList &fileNames, const QString &searchText, int maxThreads)
{
    if (fileNames.isEmpty()) {
        qDebug() << "SearchCoordinator: 文件列表为空，无需搜索";
        emit searchFinished(QList<SearchResult>());
        return;
    }

    // 重置状态
    m_allResults.clear();
    m_completedThreads = 0;
    m_processedFiles = 0;
    m_totalFiles = fileNames.size();

    // 计算最优线程数
    int threadCount = getOptimalThreadCount(fileNames.size(), maxThreads);
    m_totalThreads = threadCount;

    qDebug() << "SearchCoordinator: 开始搜索";
    qDebug() << "  总文件数:" << m_totalFiles;
    qDebug() << "  线程数:" << threadCount;
    qDebug() << "  搜索关键字:" << searchText;

    // 分割文件列表
    QVector<QStringList> batches = splitFileList(fileNames, threadCount);

    qDebug() << "  文件批次分割:";
    for (int i = 0; i < batches.size(); ++i) {
        qDebug() << "    批次" << (i + 1) << ":" << batches[i].size() << "个文件";
    }

    // 为每个批次创建并启动搜索线程
    for (int i = 0; i < batches.size(); ++i) {
        SearchThread *thread = new SearchThread();
        thread->setFileNames(batches[i]);
        thread->setSearchText(searchText);

        // 连接信号
        connect(thread, &SearchThread::searchFinished,
                this, &SearchCoordinator::onThreadFinished);
        connect(thread, &SearchThread::searchProgress,
                this, &SearchCoordinator::onThreadProgress);
        connect(thread, &SearchThread::searchError,
                this, &SearchCoordinator::onThreadError);

        // 提交到线程池
        QThreadPool::globalInstance()->start(thread);

        qDebug() << "SearchCoordinator: 已启动线程" << (i + 1) << "/" << threadCount;
    }
}

/**
 * @brief 处理单个线程完成的槽函数
 * @param results 该线程的搜索结果
 */
void SearchCoordinator::onThreadFinished(const QList<SearchResult> &results)
{
    // 线程安全地添加结果
    {
        QMutexLocker locker(&m_resultsMutex);
        m_allResults.append(results);
    }

    // 原子操作增加完成计数
    int completed = m_completedThreads.fetchAndAddRelaxed(1) + 1;

    qDebug() << "SearchCoordinator: 线程完成" << completed << "/" << m_totalThreads
             << ", 该线程找到" << results.size() << "个结果";

    // 检查是否所有线程都完成了
    if (completed >= m_totalThreads) {
        qDebug() << "SearchCoordinator: 所有线程完成，总共找到" << m_allResults.size() << "个结果";
        emit searchFinished(m_allResults);
    }
}

/**
 * @brief 处理线程进度更新的槽函数
 * @param processed 该线程已处理的文件数（相对于该线程的文件列表）
 * @param total 该线程的总文件数
 * @param currentFileName 当前文件名
 */
void SearchCoordinator::onThreadProgress(int processed, int total, const QString &currentFileName)
{
    Q_UNUSED(total); // 不使用线程局部的总数

    // 线程安全地更新全局进度
    int globalProcessed;
    {
        QMutexLocker locker(&m_progressMutex);
        m_processedFiles++;
        globalProcessed = m_processedFiles;
    }

    // 发射全局进度信号
    emit searchProgress(globalProcessed, m_totalFiles, currentFileName);

    // 调试输出
    if (globalProcessed % 10 == 0 || globalProcessed == m_totalFiles) {
        qDebug() << "SearchCoordinator: 总体进度" << globalProcessed << "/" << m_totalFiles
                 << QString("(%1%)").arg(globalProcessed * 100 / m_totalFiles);
    }
}

/**
 * @brief 处理线程错误的槽函数
 * @param error 错误信息
 */
void SearchCoordinator::onThreadError(const QString &error)
{
    qDebug() << "SearchCoordinator: 线程错误:" << error;
    // 直接转发错误信号
    emit searchError(error);
}
