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
 * @file aboutdialog.h
 * @brief 关于对话框类定义
 *
 * AboutDialog显示应用程序的基本信息：
 * - 程序名称和版本
 * - 程序描述
 * - 开发者信息（Credits）
 * - 许可证信息（GPL-3.0）
 *
 * 用户可以通过按钮切换查看开发者信息和许可证全文
 */

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

/**
 * @class AboutDialog
 * @brief 关于对话框类
 *
 * 对话框显示内容：
 * - 程序名称、版本号、描述
 * - 两个可切换按钮：Credits（开发者信息）和License（许可证）
 * - 点击按钮后在文本框中显示对应内容
 *
 * 数据来源：
 * - Credits内容从资源文件 :/text/credits.html 加载
 * - License内容从资源文件 :/text/gpl-3.0.txt 加载
 * - 程序信息从settings.h中的宏定义获取
 *
 * 用户交互：
 * - 点击Credits按钮：显示开发者信息，隐藏默认内容
 * - 点击License按钮：显示GPL-3.0许可证全文，隐藏默认内容
 * - 再次点击按钮可切换回默认显示
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

private Q_SLOTS:
    /**
     * @brief Credits按钮点击
     * @param checked 按钮是否被选中
     *
     * 选中时显示开发者信息，取消选中时恢复默认显示
     */
    void onCreditsClicked(bool checked);

    /**
     * @brief License按钮点击
     * @param checked 按钮是否被选中
     *
     * 选中时显示GPL-3.0许可证全文，取消选中时恢复默认显示
     */
    void onLicenseClicked(bool checked);

private:

    Ui::AboutDialog *ui;       ///< UI界面对象

    QString mCredits;          ///< 缓存的开发者信息（从资源文件加载）
    QString mLicense;          ///< 缓存的许可证文本（从资源文件加载）
};

#endif // ABOUTDIALOG_H
