/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file pullsender.cpp
 * @brief Pull 模式发送端实现
 *
 * 本文件实现了 PullSender 类的所有功能。
 *
 * 命令处理流程：
 *
 * 1. LIST 命令（获取共享列表）：
 *    - 接收：{"command": 1, "params": {}, "device_id": "...", "device_name": "..."}
 *    - 处理：调用 SharedFileManager::getShareListJson()
 *    - 响应：{"version": "2.0", "device": {...}, "shares": [...]}
 *
 * 2. BROWSE 命令（浏览文件夹）：
 *    - 接收：{"command": 2, "params": {"share_id": "...", "path": "...", "password": "..."}, ...}
 *    - 处理：验证权限 -> 调用 SharedFileManager::browseShare()
 *    - 响应：{"share_id": "...", "path": "...", "items": [...]} 或 {"error": "..."}
 *
 * 3. GET 命令（下载文件）：
 *    - 接收：{"command": 3, "params": {"share_id": "...", "path": "...", "type": "file", "password": "..."}, ...}
 *    - 处理：验证权限 -> 检查文件 -> 发送响应 -> 发送文件
 *    - 响应：{"status": "ok"} 或 {"error": "..."}
 *    - 后续：发送 Header 包 -> Data 包序列 -> Finish 包
 *
 * 文件传输实现：
 * - 使用定时器（QTimer）分块发送，避免阻塞主线程
 * - 每 10ms 发送一个数据块（大小由 Settings::getFileBufferSize() 决定）
 * - 实时更新传输进度（mInfo->setProgress）
 * - 支持暂停/恢复/取消操作（继承自 Transfer）
 *
 * 安全考虑：
 * - 路径安全：SharedFileManager::getAbsolutePath() 防止目录遍历
 * - 权限验证：SharedFileManager::checkAccess() 验证密码和访问控制列表
 * - 访问日志：SharedFileManager::recordAccess() 记录所有访问行为
 */

#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8，支持中文
#include "pullsender.h"
#include "../ui/settings.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QtDebug>

PullSender::PullSender(QTcpSocket* socket, QObject* parent)
    : Transfer(socket, parent), mFileSize(0), mBytesSent(0) {
    mShareManager = SharedFileManager::getInstance();
    mInfo->setTransferType(TransferType::Upload);

    mSendTimer = new QTimer(this);
    mSendTimer->setInterval(10); // 10ms between chunks
    connect(mSendTimer, &QTimer::timeout, this, &PullSender::sendNextChunk);
}

PullSender::~PullSender() {
    if (mSendTimer->isActive()) {
        mSendTimer->stop();
    }
    delete mSendTimer;
}

void PullSender::processPacket(QByteArray& data, PacketType type) {
    if (type == PacketType::Command) {
        processCommandPacket(data);
    } else {
        Transfer::processPacket(data, type);
    }
}

void PullSender::processCommandPacket(QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject command = doc.object();

    int cmdType = command["command"].toInt();
    QJsonObject params = command["params"].toObject();
    QString deviceId = command["device_id"].toString();
    QString deviceName = command["device_name"].toString();

    // 获取请求设备信息
    Device requestDevice;
    requestDevice.setId(deviceId);
    requestDevice.setName(deviceName);
    requestDevice.setAddress(mSocket->peerAddress());

    // 更新传输信息
    mInfo->setPeer(requestDevice);

    switch (static_cast<CommandType>(cmdType)) {
        case CommandType::LIST:
            handleListCommand(command);
            break;

        case CommandType::BROWSE:
            handleBrowseCommand(command);
            break;

        case CommandType::GET:
            handleGetCommand(command);
            break;

        default:
            sendErrorResponse(tr("Unknown command"));
            break;
    }
}

void PullSender::handleListCommand(const QJsonObject& command) {
    QJsonObject shareList = mShareManager->getShareListJson();
    sendJsonResponse(shareList);
}

void PullSender::handleBrowseCommand(const QJsonObject& command) {
    QJsonObject params = command["params"].toObject();
    QString shareId = params["share_id"].toString();
    QString path = params["path"].toString();

    // 检查访问权限
    Device requestDevice;
    requestDevice.setId(command["device_id"].toString());
    requestDevice.setName(command["device_name"].toString());
    requestDevice.setAddress(mSocket->peerAddress());

    QString password = params["password"].toString();
    if (!mShareManager->checkAccess(shareId, requestDevice, password)) {
        sendErrorResponse(tr("Access denied"));
        return;
    }

    QJsonObject browseResult = mShareManager->browseShare(shareId, path);
    sendJsonResponse(browseResult);

    // 记录访问
    mShareManager->recordAccess(shareId, requestDevice, "browse");
}

void PullSender::handleGetCommand(const QJsonObject& command) {
    QJsonObject params = command["params"].toObject();
    QString shareId = params["share_id"].toString();
    QString path = params["path"].toString();
    QString type = params["type"].toString();

    // 检查访问权限
    Device requestDevice;
    requestDevice.setId(command["device_id"].toString());
    requestDevice.setName(command["device_name"].toString());
    requestDevice.setAddress(mSocket->peerAddress());

    QString password = params["password"].toString();
    if (!mShareManager->checkAccess(shareId, requestDevice, password)) {
        sendErrorResponse(tr("Access denied"));
        return;
    }

    // 获取文件的绝对路径
    QString absolutePath = mShareManager->getAbsolutePath(shareId, path);
    if (absolutePath.isEmpty()) {
        sendErrorResponse(tr("File not found"));
        return;
    }

    QFileInfo fileInfo(absolutePath);
    if (!fileInfo.exists()) {
        sendErrorResponse(tr("File does not exist"));
        return;
    }

    if (type == "folder") {
        // TODO: 实现文件夹压缩后发送
        sendErrorResponse(tr("Folder download not yet implemented"));
        return;
    }

    if (!fileInfo.isFile()) {
        sendErrorResponse(tr("Not a file"));
        return;
    }

    // 发送成功响应
    QJsonObject response;
    response["status"] = "ok";
    sendJsonResponse(response);

    // 发送文件
    mCurrentShareId = shareId;
    mCurrentPath = path;
    sendFileContent(absolutePath);

    // 记录访问
    mShareManager->recordAccess(shareId, requestDevice, "download");
}

void PullSender::sendJsonResponse(const QJsonObject& response) {
    QByteArray data = QJsonDocument(response).toJson(QJsonDocument::Compact);
    writePacket(data.size(), PacketType::Response, data);
}

void PullSender::sendErrorResponse(const QString& error) {
    QJsonObject response;
    response["error"] = error;
    sendJsonResponse(response);
}

void PullSender::sendFileContent(const QString& filePath) {
    if (mFile) {
        delete mFile;
        mFile = nullptr;
    }

    mFile = new QFile(filePath, this);
    if (!mFile->open(QIODevice::ReadOnly)) {
        sendErrorResponse(tr("Cannot open file"));
        return;
    }

    mFileSize = mFile->size();
    mBytesSent = 0;

    // 发送文件头
    QJsonObject header;
    header["name"] = QFileInfo(filePath).fileName();
    header["size"] = mFileSize;
    header["type"] = "file";

    QByteArray headerData = QJsonDocument(header).toJson(QJsonDocument::Compact);
    writePacket(headerData.size(), PacketType::Header, headerData);

    // 设置传输状态
    mInfo->setFilePath(filePath);
    mInfo->setDataSize(mFileSize);
    mInfo->setState(TransferState::Transfering);

    // 开始发送文件数据
    mSendTimer->start();
}

void PullSender::sendNextChunk() {
    if (!mFile || !mFile->isOpen()) {
        mSendTimer->stop();
        return;
    }

    // 检查是否已暂停或取消
    if (mInfo->getState() == TransferState::Paused ||
        mInfo->getState() == TransferState::Cancelled) {
        mSendTimer->stop();
        return;
    }

    qint32 chunkSize = Settings::instance()->getFileBufferSize();
    QByteArray chunk = mFile->read(chunkSize);

    if (!chunk.isEmpty()) {
        writePacket(chunk.size(), PacketType::Data, chunk);
        mBytesSent += chunk.size();

        // 更新进度
        int progress = (mBytesSent * 100) / mFileSize;
        mInfo->setProgress(progress);

        // 检查是否完成
        if (mBytesSent >= mFileSize) {
            mSendTimer->stop();

            // 发送完成标志
            writePacket(0, PacketType::Finish, QByteArray());
            mInfo->setState(TransferState::Finish);

            mFile->close();
            delete mFile;
            mFile = nullptr;
        }
    } else {
        mSendTimer->stop();

        // 文件读取错误或已到结尾
        if (mBytesSent < mFileSize) {
            // 读取错误
            mInfo->setState(TransferState::Cancelled);
            qWarning() << "File read error at" << mBytesSent << "of" << mFileSize;
        } else {
            // 正常结束
            writePacket(0, PacketType::Finish, QByteArray());
            mInfo->setState(TransferState::Finish);
        }

        if (mFile) {
            mFile->close();
            delete mFile;
            mFile = nullptr;
        }
    }
}
