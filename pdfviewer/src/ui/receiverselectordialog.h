/*
    LANShare - LAN file transfer.
    Copyright (C) 2016 Abdul Aris R. <abdularisrahmanudin10@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file receiverselectordialog.h
 * @brief 接收方选择对话框类定义
 *
 * ReceiverSelectorDialog用于选择文件传输的接收方，
 * 显示局域网内通过UDP广播发现的所有设备。
 *
 * 用户可以单选或多选接收方，支持刷新设备列表。
 */

#ifndef RECEIVERSELECTORDIALOG_H
#define RECEIVERSELECTORDIALOG_H

#include <QDialog>

class DeviceListModel;
class Device;

namespace Ui {
class ReceiverSelectorDialog;
}

/**
 * @class ReceiverSelectorDialog
 * @brief 接收方选择对话框类
 *
 * 对话框显示局域网内发现的所有设备，用户可以：
 * - 单选或多选接收方（支持Ctrl/Shift多选）
 * - 点击"发送"按钮确认选择
 * - 点击"刷新"按钮重新扫描设备
 *
 * 设备信息由DeviceListModel提供，该模型从DeviceBroadcaster
 * 获取通过UDP广播发现的设备。
 *
 * 用户交互流程：
 * 1. 对话框打开时自动刷新设备列表
 * 2. 用户从列表中选择一个或多个设备
 * 3. 点击"发送"确认选择（如果没有选择，会提示用户）
 * 4. 主窗口获取选中的设备并开始发送
 */
class ReceiverSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReceiverSelectorDialog(DeviceListModel* model, QWidget *parent = nullptr);
    ~ReceiverSelectorDialog() override;

    /** @brief 获取当前选中的设备（单选模式） */
    Device getSelectedDevice() const;

    /** @brief 获取所有选中的设备（多选模式） */
    QVector<Device> getSelectedDevices() const;

private Q_SLOTS:
    /**
     * @brief 发送按钮点击
     *
     * 验证是否选择了接收方，如果选择了则接受对话框，
     * 否则提示用户选择
     */
    void onSendClicked();

    /** @brief 刷新按钮点击，重新扫描并显示设备列表 */
    void onRefreshClicked();

private:
    Ui::ReceiverSelectorDialog *ui;    ///< UI界面对象

    DeviceListModel* mModel;           ///< 设备列表数据模型
};

#endif // RECEIVERSELECTORDIALOG_H
