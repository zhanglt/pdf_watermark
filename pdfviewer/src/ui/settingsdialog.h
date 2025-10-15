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
 * @file settingsdialog.h
 * @brief 设置对话框类定义
 *
 * SettingsDialog是应用程序的配置界面，用户可以在此修改：
 * - 设备信息（设备名称、ID、IP地址、操作系统）
 * - 网络配置（广播端口、传输端口、广播间隔）
 * - 文件传输设置（下载目录、缓冲区大小、是否覆盖已存在文件）
 * - 配置重置功能
 *
 * 配置持久化通过Settings单例类实现
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

/**
 * @class SettingsDialog
 * @brief 设置对话框类
 *
 * 对话框包含以下设置项：
 * - 设备信息：ID（只读）、IP地址（只读）、操作系统（只读）、设备名称（可编辑）
 * - 网络配置：广播端口、传输端口、广播间隔（秒）
 * - 文件传输：下载目录、缓冲区大小（KB）、是否覆盖已存在文件
 *
 * 用户交互：
 * - 打开对话框时自动加载当前配置
 * - 点击"保存"：验证并保存配置到Settings
 * - 点击"取消"：放弃修改并关闭对话框
 * - 点击"重置"：恢复默认配置
 * - 点击下载目录旁的按钮：弹出文件夹选择对话框
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;

Q_SIGNALS:
    /**
     * @brief 传输端口已更改信号
     * @param newPort 新的传输端口号
     *
     * 当用户修改传输端口并保存时发出此信号，
     * 通知MainWindow需要重启TransferServer以使用新端口
     */
    void transferPortChanged(int newPort);

private Q_SLOTS:
    /** @brief 取消按钮点击，关闭对话框不保存 */
    void onCancelClicked();

    /**
     * @brief 保存按钮点击
     *
     * 保存流程：
     * 1. 验证设备名称不为空
     * 2. 从UI控件读取所有配置项
     * 3. 更新Settings单例中的配置
     * 4. 调用saveSettings()持久化到配置文件
     * 5. 关闭对话框
     */
    void onSaveClicked();

    /**
     * @brief 重置按钮点击
     *
     * 将所有配置项恢复为默认值，并刷新UI显示
     */
    void onResetClicked();

    /** @brief 选择下载目录按钮点击，弹出文件夹选择对话框 */
    void onSelectDownDirClicked();

private:
    /**
     * @brief 将Settings中的配置赋值到UI控件
     *
     * 从Settings单例读取当前配置并填充到各个输入框、数字框和复选框
     */
    void assign();

    Ui::SettingsDialog *ui;  ///< UI界面对象
};

#endif // SETTINGSDIALOG_H
