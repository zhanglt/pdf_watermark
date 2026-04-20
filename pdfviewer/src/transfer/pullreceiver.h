/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file pullreceiver.h
 * @brief Pull 模式接收端类定义
 *
 * 本文件定义了 PullReceiver 类，它实现了"拉取"模式的文件传输接收端。
 * 与传统的 Receiver（推送模式）不同，PullReceiver 是主动请求模式。
 *
 * Pull 模式 vs Push 模式：
 *
 * Push 模式（Receiver）：
 * - 被动接收：等待其他设备推送文件
 * - 流程：监听连接 -> 接收 Header -> 接收 Data -> 接收 Finish
 * - 用例：接受其他用户主动发送的文件
 *
 * Pull 模式（PullReceiver）：
 * - 主动拉取：用户浏览远程共享并选择下载
 * - 流程：连接远程 -> 发送命令 -> 接收响应 -> 接收文件数据
 * - 用例：用户浏览远程设备的共享文件夹，选择性下载文件
 *
 * PullReceiver 可以发送的命令：
 * 1. LIST：请求远程设备的共享列表
 * 2. BROWSE：浏览远程共享文件夹的内容
 * 3. GET：下载指定的文件或文件夹
 *
 * 工作流程：
 * 1. 用户选择远程设备，创建 PullReceiver 实例
 * 2. 调用 requestShareList() 获取共享列表
 * 3. 用户浏览共享，调用 browseShare() 查看文件夹内容
 * 4. 用户选择文件，调用 pullFile() 下载
 * 5. PullReceiver 处理响应和文件数据
 *
 * 关键特性：
 * - 断点续传：支持暂停和恢复（继承自 Transfer）
 * - 文件冲突处理：自动重命名或覆盖已存在的文件
 * - 进度跟踪：实时报告下载进度
 * - 错误处理：友好的错误提示（访问拒绝、文件不存在等）
 */

#ifndef PULLRECEIVER_H
#define PULLRECEIVER_H

#include "transfer.h"
#include "../model/device.h"
#include <QJsonObject>
#include <QJsonArray>

/**
 * @class PullReceiver
 * @brief Pull 模式接收端（主动拉取）
 *
 * PullReceiver 继承自 Transfer，实现了拉取模式的文件接收。
 * 它主动连接远程设备，发送命令请求，并接收响应和文件数据。
 *
 * 与 Receiver 的区别：
 * - Receiver：被动接收，等待文件推送
 * - PullReceiver：主动拉取，浏览并选择下载
 *
 * 使用场景：
 * 1. 浏览远程共享：用户想查看其他设备共享了哪些文件
 * 2. 选择性下载：只下载需要的文件，而不是全部接收
 * 3. 文件夹浏览：深入文件夹结构，选择特定文件
 *
 * 信号机制：
 * - shareListReceived：共享列表获取完成，UI 可显示列表
 * - browseResultReceived：文件夹浏览结果，UI 可显示文件列表
 * - pullStarted/pullProgress/pullCompleted：下载进度通知
 * - pullError：错误通知（权限拒绝、文件不存在等）
 */
class PullReceiver : public Transfer {
    Q_OBJECT

private:
    Device mRemoteDevice;               ///< 远程设备信息
    QString mRemoteShareId;             ///< 当前操作的共享ID
    QString mRemotePath;                ///< 远程文件的相对路径
    QString mLocalPath;                 ///< 本地保存路径
    CommandType mCurrentCommand;        ///< 当前执行的命令类型
    QJsonObject mPendingRequest;        ///< 待处理的请求（预留）
    qint64 mBytesReceived;              ///< 已接收字节数
    qint64 mTotalFileSize;              ///< 文件总大小

public:
    /**
     * @brief 构造函数
     * @param remoteDevice 远程设备信息
     * @param parent 父对象
     *
     * 初始化 PullReceiver，设置传输类型为 Download
     */
    PullReceiver(const Device& remoteDevice, QObject* parent = nullptr);

    // ========== 拉取操作接口 ==========

    /**
     * @brief 请求远程设备的共享列表
     *
     * 发送 LIST 命令，结果通过 shareListReceived 信号返回
     */
    void requestShareList();

    /**
     * @brief 浏览共享文件夹的内容
     * @param shareId 共享ID
     * @param path 相对路径（默认为根目录）
     *
     * 发送 BROWSE 命令，结果通过 browseResultReceived 信号返回
     */
    void browseShare(const QString& shareId, const QString& path = "");

    /**
     * @brief 拉取（下载）文件
     * @param shareId 共享ID
     * @param remotePath 远程文件的相对路径
     * @param localPath 本地保存路径（目录）
     *
     * 发送 GET 命令，下载指定文件
     * 流程：
     * 1. 发送命令
     * 2. 接收响应（成功或失败）
     * 3. 接收 Header 包（文件名、大小）
     * 4. 接收 Data 包序列（文件内容）
     * 5. 接收 Finish 包（完成标志）
     *
     * 文件冲突处理：
     * - 如果启用"替换现有文件"选项，覆盖同名文件
     * - 否则自动重命名（添加 (1)、(2) 等后缀）
     */
    void pullFile(const QString& shareId, const QString& remotePath,
                 const QString& localPath);

    /**
     * @brief 拉取（下载）文件夹
     * @param shareId 共享ID
     * @param remotePath 远程文件夹的相对路径
     * @param localPath 本地保存路径
     *
     * TODO: 当前实现尚未完成，需要先压缩文件夹再传输
     */
    void pullFolder(const QString& shareId, const QString& remotePath,
                   const QString& localPath);

Q_SIGNALS:
    /**
     * @brief 共享列表接收完成信号
     * @param shares JSON 对象，包含远程设备的共享列表
     */
    void shareListReceived(const QJsonObject& shares);

    /**
     * @brief 浏览结果接收完成信号
     * @param content JSON 对象，包含文件和文件夹列表
     */
    void browseResultReceived(const QJsonObject& content);

    /**
     * @brief 拉取开始信号
     * @param fileName 正在下载的文件名
     */
    void pullStarted(const QString& fileName);

    /**
     * @brief 拉取进度信号
     * @param percentage 进度百分比（0-100）
     */
    void pullProgress(int percentage);

    /**
     * @brief 拉取完成信号
     * @param fileName 已完成的文件名
     */
    void pullCompleted(const QString& fileName);

    /**
     * @brief 拉取错误信号
     * @param error 错误信息
     */
    void pullError(const QString& error);

protected:
    /**
     * @brief 处理接收到的数据包（重写）
     * @param data 数据包内容
     * @param type 数据包类型
     *
     * 重写 Transfer::processPacket，添加对 Response 包的处理
     */
    void processPacket(QByteArray& data, PacketType type) override;

    /**
     * @brief 处理命令响应包
     * @param data JSON 格式的响应数据
     *
     * 根据当前命令类型，调用相应的响应处理函数
     */
    void processCommandResponse(QByteArray& data);

    /**
     * @brief 处理文件头包（重写）
     * @param data Header 数据（JSON 格式）
     *
     * 对于 GET 命令，创建本地文件并准备接收数据
     */
    void processHeaderPacket(QByteArray& data) override;

    /**
     * @brief 处理文件数据包（重写）
     * @param data 文件数据
     *
     * 将数据写入本地文件，更新进度
     */
    void processDataPacket(QByteArray& data) override;

    /**
     * @brief 处理传输完成包（重写）
     * @param data 完成标志数据
     *
     * 关闭文件，发射 pullCompleted 信号
     */
    void processFinishPacket(QByteArray& data) override;

private:
    /**
     * @brief 发送命令到远程设备
     * @param cmd 命令类型
     * @param params 命令参数（JSON 对象）
     *
     * 构造命令包并发送，格式：
     * {
     *   "command": <命令代码>,
     *   "params": {...},
     *   "device_id": "...",
     *   "device_name": "..."
     * }
     */
    void sendCommand(CommandType cmd, const QJsonObject& params);

    /**
     * @brief 处理共享列表响应
     * @param response 响应 JSON 对象
     *
     * 发射 shareListReceived 信号
     */
    void handleShareListResponse(const QJsonObject& response);

    /**
     * @brief 处理浏览响应
     * @param response 响应 JSON 对象
     *
     * 发射 browseResultReceived 信号
     */
    void handleBrowseResponse(const QJsonObject& response);

    /**
     * @brief 处理拉取响应
     * @param response 响应 JSON 对象
     *
     * 检查是否有错误，准备接收文件数据
     */
    void handlePullResponse(const QJsonObject& response);

    /**
     * @brief 启动到远程设备的连接
     * @return 连接是否成功
     *
     * 如果尚未连接，则创建 socket 并连接到远程设备
     * 如果已连接，直接返回 true
     */
    bool startConnection();
};

#endif // PULLRECEIVER_H
