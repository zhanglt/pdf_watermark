/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file pullsender.h
 * @brief Pull 模式发送端类定义
 *
 * 本文件定义了 PullSender 类，它实现了"拉取"模式的文件传输发送端。
 * 与传统的 Sender（推送模式）不同，PullSender 是被动响应模式。
 *
 * Pull 模式 vs Push 模式：
 *
 * Push 模式（Sender）：
 * - 发送方主动：发送方选择文件并推送给接收方
 * - 流程：Sender 连接 -> 发送 Header -> 发送 Data -> 发送 Finish
 * - 用例：用户主动向其他设备发送文件
 *
 * Pull 模式（PullSender）：
 * - 接收方主动：接收方浏览共享列表并拉取所需文件
 * - 流程：接收方连接 -> 发送命令（LIST/BROWSE/GET）-> PullSender 响应
 * - 用例：用户浏览远程设备的共享，选择性下载文件
 *
 * PullSender 支持的命令：
 * 1. LIST：获取共享列表
 * 2. BROWSE：浏览共享文件夹内容
 * 3. GET：下载指定文件
 *
 * 工作流程：
 * 1. TransferServer 接受连接，创建 PullSender 实例
 * 2. PullSender 等待接收 Command 包
 * 3. 根据命令类型调用相应的处理函数
 * 4. 发送 Response 包返回结果
 * 5. 对于 GET 命令，后续发送文件数据
 *
 * 安全特性：
 * - 访问权限验证（密码、白名单/黑名单）
 * - 路径安全检查（防止目录遍历攻击）
 * - 访问日志记录
 */

#ifndef PULLSENDER_H
#define PULLSENDER_H

#include "transfer.h"
#include "../share/sharedfilemanager.h"
#include <QJsonObject>

/**
 * @class PullSender
 * @brief Pull 模式发送端（被动响应）
 *
 * PullSender 继承自 Transfer，实现了拉取模式的文件传输。
 * 它响应远程设备的命令请求，提供共享列表、文件浏览和文件下载功能。
 *
 * 与 Sender 的区别：
 * - Sender：主动推送，发送方选择文件并发送
 * - PullSender：被动响应，接收方选择文件并拉取
 *
 * 关键特性：
 * 1. 命令驱动：根据 Command 包执行不同操作
 * 2. 访问控制：与 SharedFileManager 集成，验证权限
 * 3. 分块发送：使用定时器分块发送文件，避免阻塞
 * 4. 状态管理：继承 Transfer 的状态机，支持暂停/恢复/取消
 */
class PullSender : public Transfer {
    Q_OBJECT

private:
    SharedFileManager* mShareManager;   ///< 共享文件管理器引用
    QString mCurrentShareId;            ///< 当前正在传输的共享ID
    QString mCurrentPath;               ///< 当前正在传输的文件相对路径
    qint64 mFileSize;                   ///< 当前文件总大小
    qint64 mBytesSent;                  ///< 当前文件已发送字节数
    QTimer* mSendTimer;                 ///< 发送定时器，每10ms发送一个数据块

public:
    /**
     * @brief 构造函数
     * @param socket TCP socket（由 TransferServer 创建）
     * @param parent 父对象
     *
     * 初始化 PullSender，设置传输类型为 Upload
     */
    PullSender(QTcpSocket* socket, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     *
     * 停止发送定时器，清理资源
     */
    ~PullSender();

protected:
    /**
     * @brief 处理接收到的数据包（重写）
     * @param data 数据包内容
     * @param type 数据包类型
     *
     * 重写 Transfer::processPacket，添加对 Command 包的处理
     */
    void processPacket(QByteArray& data, PacketType type) override;

    /**
     * @brief 处理命令请求包
     * @param data JSON 格式的命令数据
     *
     * 解析命令类型（LIST/BROWSE/GET），调用相应的处理函数
     */
    void processCommandPacket(QByteArray& data);

private:
    /**
     * @brief 处理 LIST 命令
     * @param command 命令对象
     *
     * 返回当前设备的共享列表（JSON 格式）
     */
    void handleListCommand(const QJsonObject& command);

    /**
     * @brief 处理 BROWSE 命令
     * @param command 命令对象（包含 share_id 和 path）
     *
     * 返回指定共享文件夹的内容列表
     * 需要验证访问权限
     */
    void handleBrowseCommand(const QJsonObject& command);

    /**
     * @brief 处理 GET 命令
     * @param command 命令对象（包含 share_id、path、type）
     *
     * 发送指定文件给请求方
     * 流程：
     * 1. 验证访问权限
     * 2. 检查文件是否存在
     * 3. 发送成功响应
     * 4. 发送文件 Header
     * 5. 启动定时器，分块发送文件数据
     */
    void handleGetCommand(const QJsonObject& command);

    /**
     * @brief 发送错误响应
     * @param error 错误信息
     *
     * 构造包含错误信息的 Response 包并发送
     */
    void sendErrorResponse(const QString& error);

    /**
     * @brief 发送 JSON 响应
     * @param response JSON 对象
     *
     * 将 JSON 对象序列化为字节数组，以 Response 包发送
     */
    void sendJsonResponse(const QJsonObject& response);

    /**
     * @brief 开始发送文件内容
     * @param filePath 文件绝对路径
     *
     * 打开文件，发送 Header 包，启动发送定时器
     */
    void sendFileContent(const QString& filePath);

private Q_SLOTS:
    /**
     * @brief 发送下一个数据块（定时器槽函数）
     *
     * 每次读取一个缓冲区大小的数据并发送
     * 更新进度信息
     * 发送完成后发送 Finish 包
     */
    void sendNextChunk();
};

#endif // PULLSENDER_H