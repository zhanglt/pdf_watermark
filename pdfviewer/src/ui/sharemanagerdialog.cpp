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
 * @file sharemanagerdialog.cpp
 * @brief 共享管理对话框实现
 *
 * 本文件实现了共享管理对话框的所有功能：
 * - 添加新的文件/文件夹共享
 * - 查看和管理现有共享列表
 * - 编辑共享配置（名称、描述、密码、权限）
 * - 激活/停用共享
 * - 删除共享
 * - 查看共享详细信息（路径、创建时间、访问次数等）
 *
 * 对话框布局：
 * - 左侧：共享列表表格，显示所有共享的概要信息
 * - 右侧：详情面板，显示选中共享的详细信息
 * - 顶部：工具栏，包含添加、删除、编辑、刷新等操作按钮
 *
 * 与SharedFileManager单例类协作，持久化管理共享配置
 */

#include "sharemanagerdialog.h"
#include "ui_sharemanagerdialog.h"
#include "../ui/util.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileInfo>

/**
 * @brief ShareManagerDialog 构造函数
 * @param parent 父窗口指针
 *
 * 初始化共享管理对话框，设置UI，连接信号槽，并更新共享列表
 */
ShareManagerDialog::ShareManagerDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ShareManagerDialog) {
    ui->setupUi(this);                                  // 设置UI界面
    mShareManager = SharedFileManager::getInstance();   // 获取共享文件管理器的单例实例
    setupUI();                                          // 初始化UI组件
    connectSignals();                                   // 连接信号和槽
    updateShareList();                                  // 更新共享列表显示
}

/**
 * @brief 析构函数
 *
 * 清理UI资源
 */
ShareManagerDialog::~ShareManagerDialog() {
    delete ui;
}

/**
 * @brief 设置UI界面
 *
 * 初始化对话框的用户界面元素：
 * - 设置窗口标题和大小
 * - 配置工具栏操作按钮的图标和信号连接
 * - 配置共享列表表格的显示属性
 * - 设置分割器的比例
 */
void ShareManagerDialog::setupUI() {
    setWindowTitle(tr("我的共享"));      // 设置窗口标题
    resize(900, 600);                    // 设置窗口默认大小

    // 设置工具栏图标（使用UI文件中定义的action）
    ui->actionAddShare->setIcon(QIcon(":/img/add.png"));
    ui->actionRemoveShare->setIcon(QIcon(":/img/remove.png"));
    ui->actionEditShare->setIcon(QIcon(":/img/edit.png"));
    ui->actionRefresh->setIcon(QIcon(":/img/refresh.png"));

    // 连接工具栏按钮的点击信号到相应的槽函数
    connect(ui->actionAddShare, &QAction::triggered, this, &ShareManagerDialog::onAddShareClicked);
    connect(ui->actionRemoveShare, &QAction::triggered, this, &ShareManagerDialog::onRemoveShareClicked);
    connect(ui->actionEditShare, &QAction::triggered, this, &ShareManagerDialog::onEditShareClicked);
    connect(ui->actionRefresh, &QAction::triggered, this, &ShareManagerDialog::onRefreshClicked);

    // 配置共享列表表格的显示属性
    ui->shareTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);   // 选择整行
    ui->shareTableWidget->setAlternatingRowColors(true);                        // 交替行颜色
    ui->shareTableWidget->horizontalHeader()->setStretchLastSection(true);      // 最后一列自动拉伸

    // 设置左右分割器的比例（左边表格：右边详情 = 2:1）
    ui->splitter->setStretchFactor(0, 2);
    ui->splitter->setStretchFactor(1, 1);
}

/**
 * @brief 连接信号和槽
 *
 * 建立UI控件和功能函数之间的信号槽连接：
 * - 共享列表表格的选择变化事件
 * - 详情面板中的复选框状态变化
 * - 共享管理器的数据变更通知
 */
void ShareManagerDialog::connectSignals() {
    // 当用户在表格中选择不同的共享项时，更新右侧的详情面板
    // 使用itemClicked而不是itemSelectionChanged，避免在表格更新时触发
    connect(ui->shareTableWidget, &QTableWidget::itemClicked,
            [this](QTableWidgetItem* item) {
                if (item) {
                    onShareSelectionChanged();
                }
            });

    // 详情面板的控件事件
    // 激活/停用共享的复选框
    connect(ui->activeCheckBox, &QCheckBox::toggled,
            this, &ShareManagerDialog::onActiveCheckBoxToggled);
    // 密码保护的复选框
    connect(ui->passwordCheckBox, &QCheckBox::toggled,
            this, &ShareManagerDialog::onPasswordCheckBoxToggled);

    // 监听共享管理器的数据变更信号，自动刷新列表显示
    // 使用Qt::QueuedConnection确保信号在事件循环中异步处理，避免嵌套调用
    connect(mShareManager, &SharedFileManager::shareAdded,
            this, &ShareManagerDialog::updateShareList, Qt::QueuedConnection);    // 新增共享时刷新
    connect(mShareManager, &SharedFileManager::shareRemoved,
            this, &ShareManagerDialog::updateShareList, Qt::QueuedConnection);    // 删除共享时刷新
    connect(mShareManager, &SharedFileManager::shareUpdated,
            this, &ShareManagerDialog::updateShareList, Qt::QueuedConnection);    // 更新共享时刷新
}

/**
 * @brief 处理"添加共享"按钮点击事件
 *
 * 完整的添加共享流程：
 * 1. 让用户选择要共享的文件或文件夹
 * 2. 获取共享配置（名称、描述、密码、权限）
 * 3. 调用共享管理器添加共享
 * 4. 更新UI显示
 */
void ShareManagerDialog::onAddShareClicked() {
    // 步骤1：选择要共享的文件或文件夹
    QString caption = tr("选择要共享的文件或文件夹");
    QString path = QFileDialog::getExistingDirectory(this, caption);  // 首先尝试选择文件夹

    if (path.isEmpty()) {
        // 如果没有选择文件夹，尝试选择单个文件
        path = QFileDialog::getOpenFileName(this, caption);
    }

    if (path.isEmpty()) {
        return;  // 用户取消了选择
    }

    // 步骤2：获取共享配置
    bool ok;
    QString defaultName = QFileInfo(path).fileName();  // 使用文件/文件夹名作为默认共享名

    ShareConfig config;

    // 2.1 输入共享名称（必需）
    config.name = QInputDialog::getText(this, tr("共享名称"),
                                       tr("请输入共享名称:"),
                                       QLineEdit::Normal,
                                       defaultName,
                                       &ok);
    if (!ok || config.name.isEmpty()) {
        return;  // 用户取消或未输入名称
    }

    // 2.2 输入共享描述（可选）
    config.description = QInputDialog::getText(this, tr("共享描述"),
                                              tr("请输入描述(可选):"),
                                              QLineEdit::Normal, "", &ok);

    // 2.3 设置访问密码（可选）
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                             tr("密码保护"),
                                                             tr("是否设置访问密码？"));
    if (reply == QMessageBox::Yes) {
        config.password = QInputDialog::getText(this, tr("设置密码"),
                                               tr("请输入访问密码:"),
                                               QLineEdit::Password, "", &ok);
        if (!ok) {
            config.password.clear();  // 用户取消，不设置密码
        }
    }

    // 2.4 设置访问权限
    QStringList items;
    items << tr("只读") << tr("读写");
    QString item = QInputDialog::getItem(this, tr("权限设置"),
                                        tr("请选择权限:"), items, 0, false, &ok);
    if (ok) {
        config.permissions = (item == tr("只读")) ? "r" : "rw";  // r=只读，rw=读写
    } else {
        config.permissions = "r";  // 默认为只读权限
    }

    // 步骤3：添加共享到管理器
    QString shareId = mShareManager->addShare(path, config);

    // 步骤4：根据结果更新UI并提示用户
    if (!shareId.isEmpty()) {
        updateShareList();  // 刷新共享列表显示
        QMessageBox::information(this, tr("成功"),
                               tr("共享添加成功"));
    } else {
        QMessageBox::warning(this, tr("失败"),
                           tr("无法添加共享"));
    }
}

/**
 * @brief 处理"删除共享"按钮点击事件
 *
 * 删除选中的共享项：
 * 1. 检查是否有选中的共享
 * 2. 弹出确认对话框
 * 3. 执行删除操作
 * 4. 清理UI并刷新列表
 */
void ShareManagerDialog::onRemoveShareClicked() {
    // 检查是否选择了共享项
    if (mCurrentShareId.isEmpty()) {
        QMessageBox::warning(this, tr("提示"),
                           tr("请先选择要删除的共享"));
        return;
    }

    // 弹出确认对话框，防止误删
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                             tr("确认删除"),
                                                             tr("确定要删除选中的共享吗？"));
    if (reply == QMessageBox::Yes) {
        // 保存要删除的ID
        QString idToDelete = mCurrentShareId;

        // 在删除前先清理UI状态，避免updateShareList尝试恢复已删除的项
        mCurrentShareId.clear();        // 清空当前选中的共享ID
        clearDetails();                  // 清空详情面板

        // 执行删除操作
        if (mShareManager->removeShare(idToDelete)) {
            // 删除成功，shareRemoved信号会自动触发updateShareList
            // 不需要手动调用updateShareList
            QMessageBox::information(this, tr("成功"),
                                   tr("共享已删除"));
        } else {
            // 删除失败，恢复选中状态
            mCurrentShareId = idToDelete;
            QMessageBox::warning(this, tr("失败"),
                               tr("无法删除共享"));
        }
    }
}

/**
 * @brief 处理"编辑共享"按钮点击事件
 *
 * 编辑选中共享的配置信息：
 * 1. 检查是否有选中的共享
 * 2. 获取当前共享的配置
 * 3. 通过对话框让用户修改配置
 * 4. 保存更新后的配置
 */
void ShareManagerDialog::onEditShareClicked() {
    // 检查是否选择了共享项
    if (mCurrentShareId.isEmpty()) {
        QMessageBox::warning(this, tr("提示"),
                           tr("请先选择要编辑的共享"));
        return;
    }

    // 获取当前共享的信息和配置
    SharedItem item = mShareManager->getSharedItem(mCurrentShareId);
    ShareConfig config = item.config;

    bool ok;
    // 编辑共享名称
    config.name = QInputDialog::getText(this, tr("编辑名称"),
                                       tr("共享名称:"),
                                       QLineEdit::Normal,
                                       config.name,          // 显示当前名称
                                       &ok);
    if (!ok) return;  // 用户取消编辑

    // 编辑共享描述（多行文本）
    config.description = QInputDialog::getMultiLineText(this, tr("编辑描述"),
                                                       tr("共享描述:"),
                                                       config.description);  // 显示当前描述

    // 更新共享配置
    if (mShareManager->updateShare(mCurrentShareId, config)) {
        updateShareList();                      // 刷新列表
        showShareDetails(mCurrentShareId);      // 刷新详情面板
        QMessageBox::information(this, tr("成功"),
                               tr("共享信息已更新"));
    } else {
        QMessageBox::warning(this, tr("失败"),
                           tr("无法更新共享信息"));
    }
}

/**
 * @brief 处理"刷新"按钮点击事件
 *
 * 重新加载并显示共享列表
 */
void ShareManagerDialog::onRefreshClicked() {
    updateShareList();
}

/**
 * @brief 更新共享列表显示
 *
 * 从共享管理器获取所有共享项，并在表格中显示：
 * - 名称、类型、大小、权限、访问次数
 * - 非活动共享以灰色显示
 * - 在第一列的UserRole中存储共享ID，用于后续操作
 */
void ShareManagerDialog::updateShareList() {
    // 在更新表格前阻止信号发射，防止在表格不一致状态时触发选择变化事件
    ui->shareTableWidget->blockSignals(true);

    // 保存当前选中的共享ID
    QString currentId = mCurrentShareId;

    // 清空表格内容
    ui->shareTableWidget->clearContents();

    // 从共享管理器获取所有共享项
    QList<SharedItem> shares = mShareManager->getSharedItems();
    ui->shareTableWidget->setRowCount(shares.size());  // 设置行数

    int row = 0;
    for (const SharedItem& share : shares) {
        // 第0列：名称（同时存储共享ID在UserRole中）
        QTableWidgetItem* nameItem = new QTableWidgetItem(share.name);
        nameItem->setData(Qt::UserRole, share.id);  // 隐藏存储共享ID
        ui->shareTableWidget->setItem(row, 0, nameItem);

        // 第1列：类型（文件夹/文件）
        ui->shareTableWidget->setItem(row, 1, new QTableWidgetItem(
            share.type == SharedItem::Folder ? tr("文件夹") : tr("文件")));

        // 第2列：大小（格式化显示）
        ui->shareTableWidget->setItem(row, 2, new QTableWidgetItem(
            Util::sizeToString(share.size)));

        // 第3列：权限（只读/读写）
        ui->shareTableWidget->setItem(row, 3, new QTableWidgetItem(
            share.permissions == "rw" ? tr("读写") : tr("只读")));

        // 第4列：访问次数
        ui->shareTableWidget->setItem(row, 4, new QTableWidgetItem(
            QString::number(share.accessCount)));

        // 如果共享未激活，将整行设置为灰色
        if (!share.isActive) {
            for (int col = 0; col < ui->shareTableWidget->columnCount(); ++col) {
                QTableWidgetItem* item = ui->shareTableWidget->item(row, col);
                if (item) {
                    item->setForeground(Qt::gray);  // 设置文字颜色为灰色
                }
            }
        }

        row++;
    }

    // 恢复信号发射
    ui->shareTableWidget->blockSignals(false);

    // 恢复之前选中的共享项（如果存在且未被删除）
    if (!currentId.isEmpty()) {
        bool found = false;
        for (int i = 0; i < ui->shareTableWidget->rowCount(); ++i) {
            QTableWidgetItem* item = ui->shareTableWidget->item(i, 0);
            if (item && item->data(Qt::UserRole).toString() == currentId) {
                ui->shareTableWidget->selectRow(i);
                found = true;
                break;
            }
        }
        // 如果之前选中的项已经不存在（比如被删除了），清空当前选中
        if (!found && currentId == mCurrentShareId) {
            mCurrentShareId.clear();
            clearDetails();
        }
    }
}

/**
 * @brief 处理共享列表选择变化事件
 *
 * 当用户在表格中选择不同的共享项时：
 * - 获取选中项的共享ID
 * - 在右侧详情面板显示该共享的详细信息
 * - 如果没有选中项，清空详情面板
 */
void ShareManagerDialog::onShareSelectionChanged() {
    // 获取当前选中的行
    int currentRow = ui->shareTableWidget->currentRow();

    if (currentRow >= 0 && currentRow < ui->shareTableWidget->rowCount()) {
        // 获取选中行的第一列（名称列），其中存储了共享ID
        QTableWidgetItem* item = ui->shareTableWidget->item(currentRow, 0);
        if (item) {
            QString shareId = item->data(Qt::UserRole).toString();  // 从UserRole中获取共享ID
            if (!shareId.isEmpty()) {
                showShareDetails(shareId);  // 显示该共享的详细信息
            } else {
                clearDetails();
            }
        } else {
            clearDetails();
        }
    } else {
        // 没有选中项，清空详情面板
        clearDetails();
    }
}

/**
 * @brief 在详情面板显示共享的详细信息
 * @param shareId 要显示的共享ID
 *
 * 显示内容包括：
 * - 共享路径
 * - 描述
 * - 创建时间
 * - 修改时间
 * - 激活状态
 * - 是否有密码保护
 */
void ShareManagerDialog::showShareDetails(const QString& shareId) {
    if (shareId.isEmpty()) {
        clearDetails();
        return;
    }

    mCurrentShareId = shareId;  // 记录当前选中的共享ID

    // 从共享管理器获取共享信息
    SharedItem item = mShareManager->getSharedItem(shareId);

    // 如果获取失败，清空详情
    if (item.id.isEmpty()) {
        clearDetails();
        return;
    }

    // 在详情面板显示各项信息（暂时断开信号以防止递归）
    ui->activeCheckBox->blockSignals(true);
    ui->passwordCheckBox->blockSignals(true);

    ui->pathLineEdit->setText(item.path);                                         // 文件路径
    ui->descriptionTextEdit->setPlainText(item.config.description);              // 描述
    ui->createdLineEdit->setText(item.createdAt.toString(Qt::DefaultLocaleShortDate));   // 创建时间
    ui->modifiedLineEdit->setText(item.modifiedAt.toString(Qt::DefaultLocaleShortDate)); // 修改时间
    ui->activeCheckBox->setChecked(item.isActive);                               // 激活状态
    ui->passwordCheckBox->setChecked(!item.config.password.isEmpty());           // 密码保护状态

    // 恢复信号
    ui->activeCheckBox->blockSignals(false);
    ui->passwordCheckBox->blockSignals(false);
}

/**
 * @brief 清空详情面板
 *
 * 清除详情面板中的所有信息，用于：
 * - 没有选中任何共享项时
 * - 删除共享后
 */
void ShareManagerDialog::clearDetails() {
    mCurrentShareId.clear();                    // 清空当前选中的共享ID
    ui->pathLineEdit->clear();                  // 清空路径显示
    ui->descriptionTextEdit->clear();           // 清空描述
    ui->createdLineEdit->clear();               // 清空创建时间
    ui->modifiedLineEdit->clear();              // 清空修改时间
    ui->activeCheckBox->setChecked(false);      // 重置激活复选框
    ui->passwordCheckBox->setChecked(false);    // 重置密码复选框
}

/**
 * @brief 处理激活复选框状态变化
 * @param checked 复选框是否选中
 *
 * 当用户切换激活/停用状态时：
 * - 更新共享管理器中的激活状态
 * - 刷新列表显示（非激活项会显示为灰色）
 */
void ShareManagerDialog::onActiveCheckBoxToggled(bool checked) {
    if (!mCurrentShareId.isEmpty()) {
        // 更新共享的激活状态
        mShareManager->setShareActive(mCurrentShareId, checked);
        // 刷新列表以更新显示（非激活项显示为灰色）
        updateShareList();
    }
}

/**
 * @brief 处理密码保护复选框状态变化
 * @param checked 复选框是否选中
 *
 * 管理共享的密码保护：
 * - 选中时：弹出对话框让用户设置密码
 * - 取消选中时：清除密码保护
 * - 如果用户取消密码输入，复选框恢复未选中状态
 */
void ShareManagerDialog::onPasswordCheckBoxToggled(bool checked) {
    if (!mCurrentShareId.isEmpty()) {
        // 获取当前共享的配置
        SharedItem item = mShareManager->getSharedItem(mCurrentShareId);
        ShareConfig config = item.config;

        if (checked) {
            // 用户想要设置密码保护
            bool ok;
            config.password = QInputDialog::getText(this, tr("设置密码"),
                                                   tr("请输入访问密码:"),
                                                   QLineEdit::Password, "", &ok);
            if (!ok) {
                // 用户取消了密码输入，恢复复选框状态
                ui->passwordCheckBox->setChecked(false);
                return;
            }
        } else {
            // 用户取消密码保护
            config.password.clear();
        }

        // 更新共享配置
        mShareManager->updateShare(mCurrentShareId, config);
    }
}
