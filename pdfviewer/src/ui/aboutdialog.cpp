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
 * @file aboutdialog.cpp
 * @brief 关于对话框实现
 *
 * 本文件实现了关于对话框的所有功能：
 * - 显示程序基本信息
 * - 加载并显示开发者信息
 * - 加载并显示许可证文本
 * - 切换不同内容的显示
 */

#include <QFile>

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "settings.h"
#include "util.h"

/**
 * @brief AboutDialog构造函数
 * @param parent 父窗口指针
 *
 * 初始化对话框：
 * 1. 设置UI
 * 2. 显示程序名称、版本号和描述
 * 3. 隐藏文本编辑框（只在点击按钮后显示）
 */
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog),
    mCredits(""), mLicense("")
{
    ui->setupUi(this);

    // 设置程序信息
    ui->programNameLbl->setText(PROGRAM_NAME);
    ui->programVersionLbl->setText(Util::parseAppVersion(false));
    ui->programDescLbl->setText(PROGRAM_DESC);

    // 初始时隐藏文本编辑框
    ui->textEdit->setVisible(false);
}

/**
 * @brief 析构函数
 *
 * 清理UI资源
 */
AboutDialog::~AboutDialog()
{
    delete ui;
}

/**
 * @brief Credits按钮点击处理
 * @param checked 按钮是否被选中
 *
 * 按钮选中时：
 * 1. 如果尚未加载，从资源文件 :/text/credits.html 加载开发者信息
 * 2. 在文本编辑框中显示开发者信息
 * 3. 隐藏默认内容，显示文本编辑框
 * 4. 取消License按钮的选中状态
 *
 * 按钮取消选中时：
 * - 恢复默认显示
 */
void AboutDialog::onCreditsClicked(bool checked)
{
    if (checked) {
        if (mCredits.isEmpty()) {
            QFile file(":/text/credits.html");
            file.open(QIODevice::ReadOnly);
            mCredits = file.readAll();
        }
        ui->textEdit->setText(mCredits);
    }

    // 切换显示内容
    ui->textContent->setVisible(!checked);     // 隐藏/显示默认内容
    ui->textEdit->setVisible(checked);         // 显示/隐藏文本编辑框
    ui->licenseBtn->setChecked(false);         // 取消License按钮的选中状态
}

/**
 * @brief License按钮点击处理
 * @param checked 按钮是否被选中
 *
 * 按钮选中时：
 * 1. 如果尚未加载，从资源文件 :/text/gpl-3.0.txt 加载许可证文本
 * 2. 在文本编辑框中显示许可证全文
 * 3. 隐藏默认内容，显示文本编辑框
 * 4. 取消Credits按钮的选中状态
 *
 * 按钮取消选中时：
 * - 恢复默认显示
 */
void AboutDialog::onLicenseClicked(bool checked)
{
    if (checked) {
        // 延迟加载许可证文本（只在第一次点击时加载）
        if (mLicense.isEmpty()) {
            QFile file(":/text/gpl-3.0.txt");
            file.open(QIODevice::ReadOnly);
            mLicense = file.readAll();
        }
        ui->textEdit->setText(mLicense);
    }

    // 切换显示内容
    ui->textContent->setVisible(!checked);     // 隐藏/显示默认内容
    ui->textEdit->setVisible(checked);         // 显示/隐藏文本编辑框
    ui->creditBtn->setChecked(false);          // 取消Credits按钮的选中状态
}
