/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file browsesharedialog.h
 * @brief 浏览共享对话框类定义
 *
 * BrowseShareDialog用于浏览局域网内其他设备的共享文件和文件夹，
 * 并支持下载共享内容。
 *
 * 主要功能：
 * - 选择局域网内的设备
 * - 查看设备的共享列表
 * - 浏览共享文件夹的内容
 * - 下载共享文件
 * - 搜索共享内容（待实现）
 *
 * 与PullReceiver协作，通过Pull协议获取共享列表和文件
 */

#ifndef BROWSESHAREDIALOG_H
#define BROWSESHAREDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QTableWidget>
#include <QJsonObject>
#include "../transfer/pullreceiver.h"
#include "../model/devicelistmodel.h"

namespace Ui {
class BrowseShareDialog;
}

/**
 * @class BrowseShareDialog
 * @brief 浏览共享对话框类
 *
 * 对话框界面布局：
 * - 顶部：设备选择下拉框、刷新按钮、搜索框
 * - 左侧：共享列表树形控件（显示设备及其共享项）
 * - 右侧：文件列表表格（显示选中共享的内容）
 * - 底部：下载按钮、进度条、状态标签
 *
 * 用户交互流程：
 * 1. 从下拉框选择一个设备
 * 2. 自动连接到设备并获取共享列表
 * 3. 在树形控件中点击共享项查看详情
 * 4. 对于文件夹类型的共享，显示其内容
 * 5. 选择要下载的文件并点击下载按钮
 * 6. 选择保存位置并开始下载
 * 7. 进度条显示下载进度
 *
 * 与后端交互：
 * - 通过PullReceiver与远程设备通信
 * - 每个设备对应一个PullReceiver实例（缓存复用）
 * - 支持的操作：获取共享列表、浏览文件夹、下载文件
 */
class BrowseShareDialog : public QDialog {
    Q_OBJECT

private:
    Ui::BrowseShareDialog *ui;                      ///< UI界面对象
    DeviceListModel* mDeviceModel;                  ///< 设备列表模型
    QMap<QString, PullReceiver*> mPullReceivers;    ///< 设备ID到PullReceiver的映射（缓存）
    Device mCurrentDevice;                          ///< 当前选中的设备
    QString mCurrentShareId;                        ///< 当前选中的共享ID
    QString mCurrentPath;                           ///< 当前浏览的路径
    QJsonObject mCurrentShareList;                  ///< 当前设备的共享列表（缓存）

public:
    explicit BrowseShareDialog(DeviceListModel* deviceModel,
                              QWidget *parent = nullptr);
    ~BrowseShareDialog();

private Q_SLOTS:
    /** @brief 设备选择变化，连接到新设备并获取共享列表 */
    void onDeviceSelectionChanged(int index);

    /** @brief 共享列表项点击，显示共享详情或浏览文件夹内容 */
    void onShareItemClicked(QTreeWidgetItem* item, int column);

    /** @brief 下载按钮点击，下载选中的文件 */
    void onDownloadClicked();

    /** @brief 刷新按钮点击，重新加载设备列表和共享列表 */
    void onRefreshClicked();

    /** @brief 搜索框文本变化（功能待实现） */
    void onSearchTextChanged(const QString& text);

    /** @brief 处理接收到的共享列表 */
    void handleShareListReceived(const QJsonObject& shares);

    /** @brief 处理接收到的文件夹浏览结果 */
    void handleBrowseResultReceived(const QJsonObject& content);

    /** @brief 处理下载进度更新 */
    void handlePullProgress(int percentage);

    /** @brief 处理下载完成 */
    void handlePullCompleted(const QString& fileName);

    /** @brief 处理下载错误 */
    void handlePullError(const QString& error);

private:
    /** @brief 初始化UI组件（设置表格、树形控件等） */
    void setupUI();

    /** @brief 连接信号和槽 */
    void connectSignals();

    /** @brief 更新设备下拉框列表 */
    void updateDeviceList();

    /** @brief 连接到指定设备并请求共享列表 */
    void connectToDevice(const Device& device);

    /** @brief 在树形控件中显示共享列表 */
    void displayShareList(const QJsonObject& shares);

    /** @brief 在表格中显示文件夹内容 */
    void displayFolderContent(const QJsonObject& content);

    /** @brief 清空所有显示内容 */
    void clearContent();

    /**
     * @brief 获取指定设备的PullReceiver（复用已创建的实例）
     * @param device 设备对象
     * @return PullReceiver指针
     */
    PullReceiver* getPullReceiver(const Device& device);

    /** @brief 选择下载保存路径 */
    QString selectDownloadPath();
};

#endif // BROWSESHAREDIALOG_H