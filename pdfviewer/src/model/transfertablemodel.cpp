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
 * @file transfertablemodel.cpp
 * @brief 传输表格模型类实现文件
 *
 * 本文件实现了 TransferTableModel 类的所有方法，包括 Qt 模型接口、
 * 传输任务管理和状态/颜色转换。核心功能是管理传输任务列表，
 * 并通过信号机制自动更新视图显示。
 */

#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8，支持中文
#include "transfertablemodel.h"
#include "../ui/util.h"

/**
 * @brief 构造函数实现
 *
 * 创建一个空的传输表格模型
 */
TransferTableModel::TransferTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

/**
 * @brief 析构函数实现
 *
 * 删除所有 Transfer 对象，释放内存
 */
TransferTableModel::~TransferTableModel()
{
    for (Transfer* t : mTransfers) {
        delete t;
    }
}

/**
 * @brief 获取行数（Qt 模型接口实现）
 *
 * 返回传输任务的数量
 */
int TransferTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mTransfers.size();
}

/**
 * @brief 获取列数（Qt 模型接口实现）
 *
 * 返回固定的列数（5列）
 */
int TransferTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (int) Column::Count;
}

/**
 * @brief 获取单元格数据（Qt 模型接口实现）
 *
 * 根据索引和角色返回相应的数据。视图在渲染表格时会为每个单元格调用此方法。
 *
 * DisplayRole：返回显示文本
 * - Peer 列：对方设备名称
 * - FileName 列：文件路径
 * - FileSize 列：格式化的文件大小（如 "10.5 MB"）
 * - State 列：状态文本
 * - Progress 列：进度百分比数字
 *
 * ForegroundRole：返回文字颜色（仅 State 列）
 * - 根据不同状态显示不同颜色
 */
QVariant TransferTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        TransferInfo* info = mTransfers.at(index.row())->getTransferInfo();

        if (info) {
            Column col = (Column) index.column();

            if (role == Qt::DisplayRole) {
                // 返回显示文本
                switch (col) {
                case Column::Peer : return info->getPeer().getName();
                case Column::FileName : return info->getFilePath();
                case Column::FileSize : return Util::sizeToString(info->getDataSize());
                case Column::State : return getStateString(info->getState());
                case Column::Progress : return info->getProgress();
                default : break;
                }
            }
            else if (role == Qt::ForegroundRole && col == Column::State) {
                // 返回状态列的文字颜色
                return getStateColor(info->getState());
            }
        }
    }

    return QVariant();
}

/**
 * @brief 获取表头数据（Qt 模型接口实现）
 *
 * 返回表格的列标题文本（中文）
 */
QVariant TransferTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        Column col = (Column) section;
        switch (col) {
        case Column::Peer : return tr("节点");
        case Column::FileName : return tr("文件路径");
        case Column::FileSize : return tr("大小");
        case Column::State : return tr("状态");
        case Column::Progress : return tr("进度");
        default : break;
        }
    }

    return QVariant();
}

/**
 * @brief 插入传输任务
 *
 * 将新的传输任务添加到列表开头（prepend），使最新的任务显示在最上面。
 * 自动连接 TransferInfo 的信号，监听状态和进度变化：
 *
 * - fileOpened 信号：文件打开时更新文件名和大小列
 * - stateChanged 信号：状态改变时更新状态列
 *
 * 使用 lambda 表达式捕获 info 指针，当信号触发时：
 * 1. 通过 info->getOwner() 找到对应的 Transfer 对象
 * 2. 在列表中查找索引
 * 3. 发出 dataChanged 信号通知视图更新指定单元格
 */
void TransferTableModel::insertTransfer(Transfer *t)
{
    if (!t) {
        return;
    }

    // 在列表开头插入新任务
    beginInsertRows(QModelIndex(), 0, 0);
    mTransfers.prepend(t);
    endInsertRows();
    emit dataChanged(index(1, 0), index(mTransfers.size()-1, (int) Column::Count));

    TransferInfo* info = t->getTransferInfo();

    // 连接文件打开信号：更新文件名和大小列
    connect(info, &TransferInfo::fileOpened, [=]() {
        int idx = mTransfers.indexOf(info->getOwner());
        QModelIndex fNameIdx = index(idx, (int) Column::FileName);
        QModelIndex fSizeIdx = index(idx, (int) Column::FileSize);
        emit dataChanged(fNameIdx, fSizeIdx);
    });

    // 连接状态变化信号：更新状态列
    connect(info, &TransferInfo::stateChanged, [=](TransferState state) {
        Q_UNUSED(state);

        int idx = mTransfers.indexOf(info->getOwner());
        QModelIndex stateIdx = index(idx, (int) Column::State);
        emit dataChanged(stateIdx, stateIdx);
    });
}

/**
 * @brief 清理已完成的传输任务
 *
 * 遍历列表，移除所有处于终止状态的任务：
 * - Idle：空闲（未开始）
 * - Finish：成功完成
 * - Disconnected：连接断开
 * - Cancelled：用户取消
 *
 * 使用 i-- 技巧：删除元素后列表长度减1，索引也要减1以避免跳过元素
 */
void TransferTableModel::clearCompleted()
{
    for (int i = 0; i < mTransfers.size(); i++) {
        Transfer* t = mTransfers.at(i);
        TransferState state = t->getTransferInfo()->getState();

        // 检查是否为终止状态
        if (state == TransferState::Idle ||
                state == TransferState::Finish ||
                state == TransferState::Disconnected ||
                state == TransferState::Cancelled) {

            beginRemoveRows(QModelIndex(), i, i);
            mTransfers.remove(i);
            endRemoveRows();
            t->deleteLater();
            i--;  // 删除后索引回退
        }
    }
}

/**
 * @brief 根据索引获取 Transfer 对象
 *
 * @param index 任务索引
 * @return Transfer 指针，索引无效时返回 nullptr
 */
Transfer* TransferTableModel::getTransfer(int index) const
{
    if (index < 0 || index >= mTransfers.size()) {
        return nullptr;
    }

    return mTransfers.at(index);
}

/**
 * @brief 根据索引获取 TransferInfo 对象
 *
 * 这是 getTransfer(index)->getTransferInfo() 的快捷方法
 *
 * @param index 任务索引
 * @return TransferInfo 指针
 */
TransferInfo* TransferTableModel::getTransferInfo(int index) const
{
    return getTransfer(index)->getTransferInfo();
}

/**
 * @brief 移除指定索引的传输任务
 *
 * 从列表中移除任务并调用 deleteLater 延迟删除 Transfer 对象
 *
 * @param index 任务索引
 */
void TransferTableModel::removeTransfer(int index)
{
    if (index < 0 || index >= mTransfers.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    mTransfers.at(index)->deleteLater();
    mTransfers.remove(index);
    endRemoveRows();
}

/**
 * @brief 将传输状态转换为中文文本
 *
 * 用于在 UI 中显示易读的状态文本
 *
 * @param state 传输状态枚举值
 * @return 中文状态文本
 */
QString TransferTableModel::getStateString(TransferState state) const
{
    switch (state) {
    case TransferState::Idle : return tr("空闲");
    case TransferState::Waiting : return tr("等待");
    case TransferState::Disconnected : return tr("断开");
    case TransferState::Paused : return tr("暂停");
    case TransferState::Cancelled : return tr("取消");
    case TransferState::Transfering : return tr("传输");
    case TransferState::Finish : return tr("完成");
    }

    return QString();
}

/**
 * @brief 获取传输状态对应的颜色
 *
 * 为不同状态返回不同的颜色，用于在状态列中以颜色区分状态：
 * - 黑色：空闲
 * - 橙色：等待、暂停（需要注意的状态）
 * - 蓝色：传输中（正常进行）
 * - 绿色：完成（成功）
 * - 红色：断开、取消（错误或失败）
 *
 * @param state 传输状态枚举值
 * @return QColor 对象
 */
QColor TransferTableModel::getStateColor(TransferState state) const
{
    switch (state) {
    case TransferState::Idle : return QColor("black");
    case TransferState::Waiting : return QColor("orange");
    case TransferState::Disconnected : return QColor("red");
    case TransferState::Paused : return QColor("orange");
    case TransferState::Cancelled : return QColor("red");
    case TransferState::Transfering : return QColor("blue");
    case TransferState::Finish : return QColor("green");
    }

    return QColor();
}
