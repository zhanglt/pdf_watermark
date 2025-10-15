/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file pullreceiver.cpp
 * @brief Pull 模式接收端实现
 *
 * 本文件实现了 PullReceiver 类的所有功能。
 *
 * 命令发送格式：
 *
 * 1. LIST 命令（请求共享列表）：
 *    - 发送：{"command": 1, "params": {}, "device_id": "...", "device_name": "..."}
 *    - 接收：{"version": "2.0", "device": {...}, "shares": [...]}
 *
 * 2. BROWSE 命令（浏览文件夹）：
 *    - 发送：{"command": 2, "params": {"share_id": "...", "path": "..."}, ...}
 *    - 接收：{"share_id": "...", "path": "...", "items": [...]} 或 {"error": "..."}
 *
 * 3. GET 命令（下载文件）：
 *    - 发送：{"command": 3, "params": {"share_id": "...", "path": "...", "type": "file"}, ...}
 *    - 接收：{"status": "ok"} 或 {"error": "..."}
 *    - 后续：Header 包 -> Data 包序列 -> Finish 包
 *
 * 文件接收实现：
 * - processHeaderPacket：创建本地文件，处理文件名冲突
 * - processDataPacket：写入文件数据，更新进度
 * - processFinishPacket：关闭文件，发射完成信号
 *
 * 文件名冲突处理策略：
 * 1. 如果设置中启用"替换现有文件"，直接覆盖
 * 2. 否则自动重命名：
 *    - 原文件名：document.txt
 *    - 冲突后：document (1).txt
 *    - 再次冲突：document (2).txt
 *    - 以此类推...
 *
 * 断点续传支持：
 * - 继承 Transfer 的暂停/恢复/取消功能
 * - 暂停：停止接收数据包，保持连接
 * - 恢复：继续接收数据包
 * - 取消：关闭连接，删除未完成的文件（可选）
 *
 * 错误处理：
 * - 连接失败：发射 pullError("Failed to connect to remote device")
 * - 访问拒绝：发射 pullError("Access denied")
 * - 文件不存在：发射 pullError("File does not exist")
 * - 写入失败：发射 pullError("Write error")
 */

#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8，支持中文
#include "pullreceiver.h"
#include "../ui/settings.h"
#include "../ui/util.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtDebug>

PullReceiver::PullReceiver(const Device& remoteDevice, QObject* parent)
    : Transfer(nullptr, parent), mRemoteDevice(remoteDevice),
      mBytesReceived(0), mTotalFileSize(0) {

    mInfo->setTransferType(TransferType::Download);
    mInfo->setPeer(remoteDevice);
}

bool PullReceiver::startConnection() {
    if (!mSocket) {
        mSocket = new QTcpSocket(this);
        setSocket(mSocket);

        connect(mSocket, &QTcpSocket::connected, [this]() {
            mInfo->setState(TransferState::Waiting);
        });

        connect(mSocket, &QTcpSocket::disconnected, [this]() {
            mInfo->setState(TransferState::Disconnected);
        });
    }

    if (mSocket->state() != QTcpSocket::ConnectedState) {
        quint16 port = Settings::instance()->getTransferPort();
        mSocket->connectToHost(mRemoteDevice.getAddress(), port);
        return mSocket->waitForConnected(5000);
    }

    return true;
}

void PullReceiver::requestShareList() {
    if (!startConnection()) {
        emit pullError(tr("Failed to connect to remote device"));
        return;
    }

    QJsonObject params;
    sendCommand(CommandType::LIST, params);
    mCurrentCommand = CommandType::LIST;
}

void PullReceiver::browseShare(const QString& shareId, const QString& path) {
    if (!startConnection()) {
        emit pullError(tr("Failed to connect to remote device"));
        return;
    }

    QJsonObject params;
    params["share_id"] = shareId;
    params["path"] = path;
    sendCommand(CommandType::BROWSE, params);
    mCurrentCommand = CommandType::BROWSE;
}

void PullReceiver::pullFile(const QString& shareId,
                           const QString& remotePath,
                           const QString& localPath) {
    if (!startConnection()) {
        emit pullError(tr("Failed to connect to remote device"));
        return;
    }

    mRemoteShareId = shareId;
    mRemotePath = remotePath;
    mLocalPath = localPath;

    QJsonObject params;
    params["share_id"] = shareId;
    params["path"] = remotePath;
    params["type"] = "file";

    sendCommand(CommandType::GET, params);
    mCurrentCommand = CommandType::GET;

    QString fileName = QFileInfo(remotePath).fileName();
    mInfo->setState(TransferState::Waiting);
    emit pullStarted(fileName);
}

void PullReceiver::pullFolder(const QString& shareId,
                             const QString& remotePath,
                             const QString& localPath) {
    if (!startConnection()) {
        emit pullError(tr("Failed to connect to remote device"));
        return;
    }

    mRemoteShareId = shareId;
    mRemotePath = remotePath;
    mLocalPath = localPath;

    QJsonObject params;
    params["share_id"] = shareId;
    params["path"] = remotePath;
    params["type"] = "folder";

    sendCommand(CommandType::GET, params);
    mCurrentCommand = CommandType::GET;

    QString folderName = QFileInfo(remotePath).fileName();
    if (folderName.isEmpty()) {
        folderName = tr("Shared Folder");
    }
    mInfo->setState(TransferState::Waiting);
    emit pullStarted(folderName);
}

void PullReceiver::sendCommand(CommandType cmd, const QJsonObject& params) {
    QJsonObject command;
    command["command"] = static_cast<int>(cmd);
    command["params"] = params;
    command["device_id"] = Settings::instance()->getDeviceId();
    command["device_name"] = Settings::instance()->getDeviceName();

    QByteArray data = QJsonDocument(command).toJson(QJsonDocument::Compact);
    writePacket(data.size(), PacketType::Command, data);
}

void PullReceiver::processPacket(QByteArray& data, PacketType type) {
    if (type == PacketType::Response) {
        processCommandResponse(data);
    } else {
        Transfer::processPacket(data, type);
    }
}

void PullReceiver::processCommandResponse(QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject response = doc.object();

    if (response.contains("error")) {
        emit pullError(response["error"].toString());
        return;
    }

    switch (mCurrentCommand) {
        case CommandType::LIST:
            handleShareListResponse(response);
            break;

        case CommandType::BROWSE:
            handleBrowseResponse(response);
            break;

        case CommandType::GET:
            handlePullResponse(response);
            break;

        default:
            break;
    }
}

void PullReceiver::handleShareListResponse(const QJsonObject& response) {
    emit shareListReceived(response);
}

void PullReceiver::handleBrowseResponse(const QJsonObject& response) {
    emit browseResultReceived(response);
}

void PullReceiver::handlePullResponse(const QJsonObject& response) {
    // GET命令的响应会跟随Header和Data包
    // 这里只处理错误情况
    if (response.contains("error")) {
        emit pullError(response["error"].toString());
        mInfo->setState(TransferState::Cancelled);
    }
}

void PullReceiver::processHeaderPacket(QByteArray& data) {
    if (mCurrentCommand == CommandType::GET) {
        // 处理文件头，准备接收文件
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject header = doc.object();

        mTotalFileSize = header["size"].toVariant().toLongLong();
        QString fileName = header["name"].toString();

        // 确保本地目录存在
        QDir localDir(mLocalPath);
        if (!localDir.exists()) {
            localDir.mkpath(".");
        }

        // 创建本地文件
        QString fullPath = QDir(mLocalPath).absoluteFilePath(fileName);

        // 处理文件名冲突
        if (QFile::exists(fullPath)) {
            if (Settings::instance()->getReplaceExistingFile()) {
                QFile::remove(fullPath);
            } else {
                // 生成唯一文件名
                QFileInfo fileInfo(fullPath);
                QString baseName = fileInfo.completeBaseName();
                QString extension = fileInfo.suffix();
                int counter = 1;

                while (QFile::exists(fullPath)) {
                    QString newName = QString("%1 (%2)").arg(baseName).arg(counter);
                    if (!extension.isEmpty()) {
                        newName += "." + extension;
                    }
                    fullPath = QDir(mLocalPath).absoluteFilePath(newName);
                    counter++;
                }
            }
        }

        mFile = new QFile(fullPath, this);

        if (!mFile->open(QIODevice::WriteOnly)) {
            emit pullError(tr("Cannot create file: %1").arg(fullPath));
            cancel();
            return;
        }

        mBytesReceived = 0;
        mInfo->setFilePath(fullPath);
        mInfo->setDataSize(mTotalFileSize);
        mInfo->setState(TransferState::Transfering);
    } else {
        Transfer::processHeaderPacket(data);
    }
}

void PullReceiver::processDataPacket(QByteArray& data) {
    if (mCurrentCommand == CommandType::GET && mFile) {
        qint64 written = mFile->write(data);
        if (written != data.size()) {
            emit pullError(tr("Write error"));
            cancel();
            return;
        }

        mBytesReceived += written;

        // 更新进度
        if (mTotalFileSize > 0) {
            int progress = (mBytesReceived * 100) / mTotalFileSize;
            mInfo->setProgress(progress);
            emit pullProgress(progress);
        }
    }
}

void PullReceiver::processFinishPacket(QByteArray& data) {
    if (mCurrentCommand == CommandType::GET) {
        if (mFile) {
            mFile->close();
            QString fileName = QFileInfo(mFile->fileName()).fileName();
            delete mFile;
            mFile = nullptr;

            mInfo->setState(TransferState::Finish);
            emit pullCompleted(fileName);
        }
    }
}
