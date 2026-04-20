/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/
#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8，支持中文

/**
 * @file browsesharedialog.cpp
 * @brief 浏览共享对话框实现
 *
 * 本文件实现了浏览共享对话框的所有功能：
 * - 连接到局域网内的设备
 * - 获取和显示设备的共享列表
 * - 浏览共享文件夹的内容
 * - 下载共享文件
 * - 显示下载进度
 *
 * 与PullReceiver协作，实现Pull协议通信
 */

#include "browsesharedialog.h"
#include "ui_browsesharedialog.h"
#include "util.h"
#include "settings.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QtDebug>

/**
 * @brief BrowseShareDialog构造函数
 * @param deviceModel 设备列表模型
 * @param parent 父窗口指针
 *
 * 初始化对话框：
 * 1. 设置UI
 * 2. 连接信号和槽
 * 3. 触发设备发现
 * 4. 更新设备下拉框列表
 */
BrowseShareDialog::BrowseShareDialog(DeviceListModel* deviceModel, QWidget *parent)
    : QDialog(parent), ui(new Ui::BrowseShareDialog), mDeviceModel(deviceModel) {
    ui->setupUi(this);
    setupUI();              // 初始化UI组件
    connectSignals();       // 连接信号和槽

    // 对话框打开时立即触发设备发现
    if (mDeviceModel) {
        qDebug() << "BrowseShareDialog: Triggering initial device discovery...";
        mDeviceModel->triggerBroadcast();

        // 稍微延迟后更新列表，给其他设备时间响应
        QTimer::singleShot(500, this, [this]() {
            updateDeviceList();
        });
    } else {
        updateDeviceList();     // 加载设备列表
    }
}

/**
 * @brief 析构函数
 *
 * 清理资源：
 * 1. 取消所有PullReceiver的传输
 * 2. 删除所有PullReceiver对象
 * 3. 清理UI资源
 */
BrowseShareDialog::~BrowseShareDialog() {
    // 清理所有PullReceiver
    for (PullReceiver* receiver : mPullReceivers) {
        receiver->cancel();      // 取消正在进行的传输
        receiver->deleteLater(); // 延迟删除对象
    }
    delete ui;
}

/**
 * @brief 初始化UI组件
 *
 * 设置界面元素的属性：
 * - 窗口标题和大小
 * - 树形控件和表格的显示属性
 * - 分割器比例
 * - 初始状态（禁用下载按钮，隐藏进度条）
 */
void BrowseShareDialog::setupUI() {
    setWindowTitle(tr("浏览网络共享"));
    resize(900, 600);

    // 设置树形控件
    ui->shareTreeWidget->setHeaderLabel(tr("共享列表"));
    ui->shareTreeWidget->setRootIsDecorated(true);

    // 设置文件表格
    ui->fileTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->fileTableWidget->setAlternatingRowColors(true);
    ui->fileTableWidget->horizontalHeader()->setStretchLastSection(true);

    // 设置分割器比例
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);

    // 初始状态
    ui->downloadButton->setEnabled(false);
    ui->progressBar->setVisible(false);
}

/**
 * @brief 连接信号和槽
 *
 * 建立UI控件和处理函数之间的信号槽连接：
 * - 设备选择变化
 * - 共享列表项点击
 * - 按钮点击（刷新、下载）
 * - 搜索框文本变化
 * - 文件表格选择变化（控制下载按钮状态）
 */
void BrowseShareDialog::connectSignals() {
    // 设备选择变化 → 连接到新设备
    connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BrowseShareDialog::onDeviceSelectionChanged);

    // 共享列表点击
    connect(ui->shareTreeWidget, &QTreeWidget::itemClicked,
            this, &BrowseShareDialog::onShareItemClicked);

    // 按钮
    connect(ui->refreshButton, &QPushButton::clicked,
            this, &BrowseShareDialog::onRefreshClicked);
    connect(ui->downloadButton, &QPushButton::clicked,
            this, &BrowseShareDialog::onDownloadClicked);

    // 搜索
    connect(ui->searchLineEdit, &QLineEdit::textChanged,
            this, &BrowseShareDialog::onSearchTextChanged);

    // 文件表格选择
    connect(ui->fileTableWidget, &QTableWidget::itemSelectionChanged, [this]() {
        ui->downloadButton->setEnabled(ui->fileTableWidget->selectedItems().size() > 0);
    });

    // 监听设备列表模型的变化，自动更新设备列表
    if (mDeviceModel) {
        // 当有新设备加入时更新列表
        connect(mDeviceModel, &QAbstractItemModel::rowsInserted,
                this, &BrowseShareDialog::updateDeviceList);
        // 当设备移除时更新列表
        connect(mDeviceModel, &QAbstractItemModel::rowsRemoved,
                this, &BrowseShareDialog::updateDeviceList);
        // 当数据变化时更新列表（如设备名称变化）
        connect(mDeviceModel, &QAbstractItemModel::dataChanged,
                this, &BrowseShareDialog::updateDeviceList);
    }
}

/**
 * @brief 更新设备下拉框列表
 *
 * 从DeviceListModel获取局域网内发现的设备，并添加到下拉框：
 * - 第一项为提示文本"选择设备..."
 * - 每个设备显示为"设备名 (IP地址)"
 * - 设备对象存储在ItemData中，供后续使用
 */
void BrowseShareDialog::updateDeviceList() {
    ui->deviceComboBox->clear();
    ui->deviceComboBox->addItem(tr("选择设备..."));  // 添加提示项

    int count = mDeviceModel->rowCount();
    qDebug() << "BrowseShareDialog: Found" << count << "devices in model";

    for (int i = 0; i < count; ++i) {
        Device device = mDeviceModel->device(i);
        QString text = QString("%1 (%2)").arg(device.getName())
                                         .arg(device.getAddress().toString());
        ui->deviceComboBox->addItem(text, QVariant::fromValue(device));  // 存储设备对象
        qDebug() << "  Device:" << text;
    }

    // 如果没有设备，显示提示
    if (count == 0) {
        ui->statusLabel->setText(tr("未发现其他设备，请检查网络连接"));
    }
}

/**
 * @brief 设备选择变化处理
 * @param index 下拉框选中的索引
 *
 * 当用户选择新设备时：
 * 1. 验证选择是否有效（不是提示项）
 * 2. 从ItemData获取设备对象
 * 3. 调用connectToDevice连接并获取共享列表
 * 4. 如果没有选择，清空显示内容
 */
void BrowseShareDialog::onDeviceSelectionChanged(int index) {
    if (index <= 0) {
        clearContent();
        return;
    }

    Device device = ui->deviceComboBox->itemData(index).value<Device>();
    if (device.isValid()) {
        mCurrentDevice = device;
        connectToDevice(device);
    }
}

void BrowseShareDialog::connectToDevice(const Device& device) {
    ui->statusLabel->setText(tr("连接到 %1...").arg(device.getName()));
    ui->progressBar->setVisible(true);
    ui->progressBar->setRange(0, 0); // 显示忙碌状态

    PullReceiver* receiver = getPullReceiver(device);
    receiver->requestShareList();
}

PullReceiver* BrowseShareDialog::getPullReceiver(const Device& device) {
    QString deviceId = device.getId();

    if (!mPullReceivers.contains(deviceId)) {
        PullReceiver* receiver = new PullReceiver(device, this);

        connect(receiver, &PullReceiver::shareListReceived,
                this, &BrowseShareDialog::handleShareListReceived);
        connect(receiver, &PullReceiver::browseResultReceived,
                this, &BrowseShareDialog::handleBrowseResultReceived);
        connect(receiver, &PullReceiver::pullProgress,
                this, &BrowseShareDialog::handlePullProgress);
        connect(receiver, &PullReceiver::pullCompleted,
                this, &BrowseShareDialog::handlePullCompleted);
        connect(receiver, &PullReceiver::pullError,
                this, &BrowseShareDialog::handlePullError);

        mPullReceivers[deviceId] = receiver;
    }

    return mPullReceivers[deviceId];
}

void BrowseShareDialog::handleShareListReceived(const QJsonObject& shares) {
    ui->progressBar->setVisible(false);
    ui->statusLabel->setText(tr("已连接"));

    mCurrentShareList = shares;
    displayShareList(shares);
}

void BrowseShareDialog::displayShareList(const QJsonObject& shares) {
    ui->shareTreeWidget->clear();

    QJsonObject deviceInfo = shares["device"].toObject();
    QString deviceName = deviceInfo["name"].toString();

    QTreeWidgetItem* deviceItem = new QTreeWidgetItem(ui->shareTreeWidget);
    deviceItem->setText(0, deviceName);
    deviceItem->setIcon(0, QIcon(":/img/computer.png"));
    deviceItem->setExpanded(true);

    QJsonArray shareArray = shares["shares"].toArray();
    for (const QJsonValue& value : shareArray) {
        QJsonObject share = value.toObject();

        QTreeWidgetItem* shareItem = new QTreeWidgetItem(deviceItem);
        shareItem->setText(0, share["name"].toString());

        // 存储共享信息
        shareItem->setData(0, Qt::UserRole, share["id"].toString());
        shareItem->setData(0, Qt::UserRole + 1, share);

        // 设置图标
        QString type = share["type"].toString();
        if (type == "folder") {
            shareItem->setIcon(0, QIcon(":/img/folder.png"));
        } else {
            shareItem->setIcon(0, QIcon(":/img/file.png"));
        }

        // 添加工具提示
        QString tooltip = tr("类型: %1\n大小: %2\n权限: %3")
            .arg(type)
            .arg(Util::sizeToString(share["size"].toVariant().toLongLong()))
            .arg(share["permissions"].toString());

        if (share["hasPassword"].toBool()) {
            tooltip += tr("\n需要密码");
        }

        shareItem->setToolTip(0, tooltip);
    }
}

void BrowseShareDialog::onShareItemClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);

    if (!item || !item->parent()) {
        return; // 顶级项（设备节点）
    }

    QString shareId = item->data(0, Qt::UserRole).toString();
    if (shareId.isEmpty()) {
        return;
    }

    mCurrentShareId = shareId;
    mCurrentPath = "";

    // 获取共享信息
    QJsonObject shareInfo = item->data(0, Qt::UserRole + 1).toJsonObject();
    QString type = shareInfo["type"].toString();

    if (type == "folder") {
        // 浏览文件夹内容
        ui->statusLabel->setText(tr("浏览文件夹..."));
        ui->progressBar->setVisible(true);
        ui->progressBar->setRange(0, 0);

        PullReceiver* receiver = getPullReceiver(mCurrentDevice);
        receiver->browseShare(shareId, "");
    } else {
        // 单个文件，显示在表格中
        ui->fileTableWidget->clearContents();
        ui->fileTableWidget->setRowCount(1);

        ui->fileTableWidget->setItem(0, 0, new QTableWidgetItem(shareInfo["name"].toString()));
        ui->fileTableWidget->setItem(0, 1, new QTableWidgetItem(tr("文件")));
        ui->fileTableWidget->setItem(0, 2, new QTableWidgetItem(
            Util::sizeToString(shareInfo["size"].toVariant().toLongLong())));
        ui->fileTableWidget->setItem(0, 3, new QTableWidgetItem("-"));
    }
}

void BrowseShareDialog::handleBrowseResultReceived(const QJsonObject& content) {
    ui->progressBar->setVisible(false);
    ui->statusLabel->setText(tr("就绪"));

    if (content.contains("error")) {
        QMessageBox::warning(this, tr("错误"), content["error"].toString());
        return;
    }

    displayFolderContent(content);
}

void BrowseShareDialog::displayFolderContent(const QJsonObject& content) {
    ui->fileTableWidget->clearContents();

    QJsonArray items = content["items"].toArray();
    ui->fileTableWidget->setRowCount(items.size());

    int row = 0;
    for (const QJsonValue& value : items) {
        QJsonObject item = value.toObject();

        QString name = item["name"].toString();
        QString type = item["type"].toString();
        qint64 size = item["size"].toVariant().toLongLong();
        QString modified = item["modified"].toString();

        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setData(Qt::UserRole, item);

        if (type == "folder") {
            nameItem->setIcon(QIcon(":/img/folder.png"));
            ui->fileTableWidget->setItem(row, 1, new QTableWidgetItem(tr("文件夹")));
        } else {
            nameItem->setIcon(QIcon(":/img/file.png"));
            ui->fileTableWidget->setItem(row, 1, new QTableWidgetItem(tr("文件")));
        }

        ui->fileTableWidget->setItem(row, 0, nameItem);
        ui->fileTableWidget->setItem(row, 2, new QTableWidgetItem(
            type == "folder" ? "-" : Util::sizeToString(size)));
        ui->fileTableWidget->setItem(row, 3, new QTableWidgetItem(modified));

        row++;
    }
}

void BrowseShareDialog::onDownloadClicked() {
    QList<QTableWidgetItem*> selectedItems = ui->fileTableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    QString downloadPath = selectDownloadPath();
    if (downloadPath.isEmpty()) {
        return;
    }

    // 获取选中的文件
    QSet<int> rows;
    for (QTableWidgetItem* item : selectedItems) {
        rows.insert(item->row());
    }

    PullReceiver* receiver = getPullReceiver(mCurrentDevice);

    for (int row : rows) {
        QTableWidgetItem* nameItem = ui->fileTableWidget->item(row, 0);
        if (!nameItem) continue;

        QJsonObject itemData = nameItem->data(Qt::UserRole).toJsonObject();
        QString name = itemData["name"].toString();
        QString type = itemData["type"].toString();

        QString remotePath = mCurrentPath.isEmpty() ? name : mCurrentPath + "/" + name;

        if (type == "file") {
            ui->statusLabel->setText(tr("下载 %1...").arg(name));
            ui->progressBar->setVisible(true);
            ui->progressBar->setRange(0, 100);
            ui->progressBar->setValue(0);

            receiver->pullFile(mCurrentShareId, remotePath, downloadPath);
        } else {
            // TODO: 实现文件夹下载
            QMessageBox::information(this, tr("提示"),
                                   tr("文件夹下载功能尚未实现"));
        }
    }
}

QString BrowseShareDialog::selectDownloadPath() {
    QString defaultPath = Settings::instance()->getDownloadDir();
    QString path = QFileDialog::getExistingDirectory(this,
                                                    tr("选择下载位置"),
                                                    defaultPath);
    return path;
}

void BrowseShareDialog::handlePullProgress(int percentage) {
    ui->progressBar->setValue(percentage);
}

void BrowseShareDialog::handlePullCompleted(const QString& fileName) {
    ui->progressBar->setVisible(false);
    ui->statusLabel->setText(tr("下载完成: %1").arg(fileName));
    QMessageBox::information(this, tr("下载完成"),
                           tr("文件 %1 下载完成").arg(fileName));
}

void BrowseShareDialog::handlePullError(const QString& error) {
    ui->progressBar->setVisible(false);
    ui->statusLabel->setText(tr("错误: %1").arg(error));
    QMessageBox::warning(this, tr("下载错误"), error);
}

void BrowseShareDialog::onRefreshClicked() {
    qDebug() << "BrowseShareDialog: Refreshing device list...";

    // 先清空设备列表，然后触发广播重新发现设备
    if (mDeviceModel) {
        qDebug() << "  Clearing existing device list...";
        mDeviceModel->refresh();  // 清空现有设备列表

        qDebug() << "  Triggering broadcast for device discovery...";
        mDeviceModel->triggerBroadcast();  // 触发新的广播发现

        // 广播后稍微延迟更新列表，让其他设备有时间响应
        QTimer::singleShot(500, this, [this]() {
            qDebug() << "  Updating device list after broadcast...";
            updateDeviceList();
        });
    } else {
        updateDeviceList();
    }

    // 如果当前有选中的设备，重新连接
    if (mCurrentDevice.isValid()) {
        qDebug() << "Reconnecting to device:" << mCurrentDevice.getName();
        connectToDevice(mCurrentDevice);
    } else {
        // 清空显示内容
        ui->shareTreeWidget->clear();
        ui->fileTableWidget->clearContents();
        ui->fileTableWidget->setRowCount(0);
        ui->statusLabel->setText(tr("请选择设备"));
    }
}

void BrowseShareDialog::onSearchTextChanged(const QString& text) {
    // TODO: 实现搜索功能
    Q_UNUSED(text);
}

void BrowseShareDialog::clearContent() {
    ui->shareTreeWidget->clear();
    ui->fileTableWidget->clearContents();
    ui->fileTableWidget->setRowCount(0);
    ui->downloadButton->setEnabled(false);
    ui->statusLabel->setText(tr("未连接"));
    mCurrentShareId.clear();
    mCurrentPath.clear();
}
