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
#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8，支持中文

/**
 * @file receiverselectordialog.cpp
 * @brief 接收方选择对话框实现
 *
 * 本文件实现了接收方选择对话框的所有功能：
 * - 显示局域网内发现的设备列表
 * - 支持单选和多选设备
 * - 刷新设备列表
 * - 验证选择并返回结果
 */

#include <QMessageBox>

#include "receiverselectordialog.h"
#include "ui_receiverselectordialog.h"

#include "../model/devicelistmodel.h"
#include "../model/device.h"

/**
 * @brief ReceiverSelectorDialog构造函数
 * @param model 设备列表数据模型
 * @param parent 父窗口指针
 *
 * 初始化对话框：
 * 1. 设置UI
 * 2. 将设备列表模型绑定到ListView
 * 3. 清空当前选择
 * 4. 刷新设备列表
 */
ReceiverSelectorDialog::ReceiverSelectorDialog(DeviceListModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReceiverSelectorDialog),
    mModel(model)
{
    ui->setupUi(this);

    // 将设备列表模型绑定到ListView
    ui->listView->setModel(mModel);
    ui->listView->setCurrentIndex(QModelIndex());  // 清空当前选择

    // 刷新设备列表，确保显示最新的设备
    model->refresh();
}

/**
 * @brief 析构函数
 *
 * 清理UI资源
 */
ReceiverSelectorDialog::~ReceiverSelectorDialog()
{
    delete ui;
}

/**
 * @brief 获取当前选中的设备（单选模式）
 * @return 选中的设备对象，如果没有选中则返回无效设备
 */
Device ReceiverSelectorDialog::getSelectedDevice() const
{
    QModelIndex currIndex = ui->listView->currentIndex();
    if (currIndex.isValid()) {
        return mModel->device(currIndex.row());
    }

    return Device();  // 返回无效设备
}

/**
 * @brief 获取所有选中的设备（多选模式）
 * @return 选中的设备列表
 *
 * 遍历所有选中的索引，从模型中获取对应的设备对象
 */
QVector<Device> ReceiverSelectorDialog::getSelectedDevices() const
{
    QVector<Device> devices;
    QItemSelectionModel* selModel = ui->listView->selectionModel();
    if (selModel) {

        // 获取所有选中的索引
        QModelIndexList selected = selModel->selectedIndexes();
        for (auto selectedIndex : selected) {
            if (selectedIndex.isValid()) {
                devices.push_back(mModel->device(selectedIndex.row()));
            }
        }
    }

    return devices;
}

/**
 * @brief 发送按钮点击
 *
 * 验证用户是否选择了接收方：
 * - 如果选择了，接受对话框并返回
 * - 如果没有选择，提示用户选择接收方
 */
void ReceiverSelectorDialog::onSendClicked()
{
    QModelIndex currIndex = ui->listView->currentIndex();
    if (currIndex.isValid())
        accept();  // 确认选择并关闭对话框
    else
        QMessageBox::information(this, tr("Info"), tr("请选择接收方"));
}

/**
 * @brief 刷新按钮点击
 *
 * 重新扫描并显示局域网内的设备列表
 */
void ReceiverSelectorDialog::onRefreshClicked()
{
    mModel->refresh();
}
