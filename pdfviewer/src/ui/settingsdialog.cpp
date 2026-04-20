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
 * @file settingsdialog.cpp
 * @brief 设置对话框实现
 *
 * 本文件实现了设置对话框的所有功能：
 * - 加载和显示当前配置
 * - 保存用户修改的配置
 * - 重置为默认配置
 * - 选择下载目录
 *
 * 与Settings类协作，实现配置的持久化管理
 */

#include <QFileDialog>
#include <QHostAddress>
#include <QVector>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "settings.h"

/**
 * @brief SettingsDialog构造函数
 * @param parent 父窗口指针
 *
 * 初始化对话框并加载当前配置到UI控件
 */
SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // 设置 IP 地址下拉框的最小宽度，确保能完整显示 IPv4 地址
    ui->ipAddrComboBox->setMinimumWidth(220);
    ui->ipAddrComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    assign();  // 加载当前配置
}

/**
 * @brief 析构函数
 *
 * 清理UI资源
 */
SettingsDialog::~SettingsDialog()
{
    delete ui;
}

/**
 * @brief 取消按钮点击
 *
 * 不保存修改，直接关闭对话框（reject）
 */
void SettingsDialog::onCancelClicked()
{
    reject();
}

/**
 * @brief 保存按钮点击
 *
 * 保存流程：
 * 1. 验证设备名称不为空
 * 2. 读取所有UI控件的值
 * 3. 更新Settings单例中的配置
 * 4. 调用saveSettings()持久化到配置文件
 * 5. 关闭对话框（accept）
 *
 * 注意：端口号、缓冲区大小等通过SpinBox控件自动限制范围
 */
void SettingsDialog::onSaveClicked()
{
    Settings* set = Settings::instance();

    // 获取原传输端口，用于检测是否变化
    int oldTransferPort = set->getTransferPort();

    // 验证并设置设备名称
    QString name = ui->deviceNameLineEdit->text();
    if (!name.isEmpty())
        set->setDeviceName(name);

    // 设置选中的IP地址
    QString selectedIpStr = ui->ipAddrComboBox->currentText();
    if (!selectedIpStr.isEmpty()) {
        QHostAddress selectedIp(selectedIpStr);
        set->setDeviceAddress(selectedIp);
    }

    // 设置网络端口
    set->setBroadcastPort(ui->bcPortSpinBox->value());
    int newTransferPort = ui->transferPortSpinBox->value();
    set->setTransferPort(newTransferPort);

    // 设置文件传输相关参数
    set->setFileBufferSize(ui->buffSizeSpinBox->value() * 1024);  // 从KB转换为字节
    set->setDeviceName(ui->deviceNameLineEdit->text());
    set->setDownloadDir(ui->downDirlineEdit->text());
    set->setBroadcastInterval(ui->bcIntervalSpinBox->value());
    set->setReplaceExistingFile(ui->overwriteCheckBox->isChecked());

    // 持久化配置到文件
    set->saveSettings();

    // 如果传输端口发生变化，发出信号通知MainWindow
    if (oldTransferPort != newTransferPort) {
        emit transferPortChanged(newTransferPort);
    }

    accept();  // 关闭对话框
}

/**
 * @brief 重置按钮点击
 *
 * 将所有配置恢复为默认值，并刷新UI显示
 */
void SettingsDialog::onResetClicked()
{
    Settings::instance()->reset();
    assign();  // 重新加载配置到UI
}

/**
 * @brief 选择下载目录按钮点击
 *
 * 弹出文件夹选择对话框，让用户选择新的下载目录
 */
void SettingsDialog::onSelectDownDirClicked()
{
    QString dirName = Settings::instance()->getDownloadDir();
    QString newDirName = QFileDialog::getExistingDirectory(this, tr("选择路径"), dirName);

    if (!newDirName.isEmpty())
        ui->downDirlineEdit->setText(newDirName);
}

/**
 * @brief 将Settings中的配置赋值到UI控件
 *
 * 从Settings单例读取当前配置并填充到各个控件：
 * - 设备ID、操作系统（只读标签）
 * - IP地址（下拉框，可选择）
 * - 设备名称（输入框）
 * - 下载目录（输入框）
 * - 广播端口、传输端口、缓冲区大小、广播间隔（数字框）
 * - 是否覆盖已存在文件（复选框）
 */
void SettingsDialog::assign()
{
    Settings* sets = Settings::instance();
    Device me = sets->getMyDevice();

    ui->deviceIdLabel->setText(me.getId());
    
    // 填充IP地址下拉框
    ui->ipAddrComboBox->clear();
    QVector<QHostAddress> addresses = sets->getAllAvailableAddresses();
    QHostAddress currentAddr = me.getAddress();
    int currentIndex = -1;
    
    for (int i = 0; i < addresses.size(); ++i) {
        ui->ipAddrComboBox->addItem(addresses[i].toString());
        if (addresses[i] == currentAddr) {
            currentIndex = i;
        }
    }
    
    // 设置当前选中的IP地址
    if (currentIndex >= 0) {
        ui->ipAddrComboBox->setCurrentIndex(currentIndex);
    } else if (!addresses.isEmpty()) {
        // 如果当前地址不在列表中，选择第一个
        ui->ipAddrComboBox->setCurrentIndex(0);
    }
    
    ui->osNameLabel->setText(me.getOSName());
    ui->deviceNameLineEdit->setText(me.getName());
    ui->downDirlineEdit->setText(sets->getDownloadDir());

    ui->bcPortSpinBox->setValue(sets->getBroadcastPort());
    ui->transferPortSpinBox->setValue(sets->getTransferPort());
    ui->buffSizeSpinBox->setValue(sets->getFileBufferSize() / 1024);
    ui->bcIntervalSpinBox->setValue(sets->getBroadcastInterval());
    ui->overwriteCheckBox->setChecked(sets->getReplaceExistingFile());
}
