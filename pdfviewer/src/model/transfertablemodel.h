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
 * @file transfertablemodel.h
 * @brief 传输表格模型类头文件
 *
 * 本文件定义了 TransferTableModel 类，这是一个基于 QAbstractTableModel 的 Qt 模型类，
 * 用于在 UI 层以表格形式展示文件传输任务列表。
 *
 * TransferTableModel 遵循 Qt 的模型-视图(Model-View)架构：
 * - 模型(Model)：TransferTableModel，管理传输任务数据
 * - 视图(View)：QTableView，以表格形式显示传输列表
 * - 数据源：Transfer 对象列表，每个对象代表一个传输任务
 *
 * 主要功能：
 * - 管理所有传输任务的 Transfer 对象
 * - 提供 Qt 标准表格模型接口（rowCount、columnCount、data、headerData）
 * - 监听每个传输任务的状态变化和进度更新，自动刷新视图
 * - 支持清理已完成的任务、移除指定任务
 * - 为不同传输状态提供颜色标识
 */

#ifndef TRANSFERTABLEMODEL_H
#define TRANSFERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QColor>

#include "../transfer/transfer.h"
#include "transferinfo.h"

/**
 * @class TransferTableModel
 * @brief 传输表格模型类
 *
 * TransferTableModel 是一个基于 QAbstractTableModel 的 Qt 模型类，用于管理和展示
 * 文件传输任务列表。它实现了 Qt 模型-视图架构中的模型角色，以表格形式呈现传输信息。
 *
 * 表格结构（列定义）：
 * - Peer：对方设备名称
 * - FileName：文件路径
 * - FileSize：文件大小（格式化后的字符串，如 "10.5 MB"）
 * - State：传输状态（空闲/等待/传输/暂停/完成等）
 * - Progress：传输进度（百分比，0-100）
 *
 * 工作原理：
 * 1. 通过 insertTransfer 方法添加 Transfer 对象到模型
 * 2. 自动连接每个 Transfer 的 TransferInfo 信号（状态变化、进度更新）
 * 3. 当传输状态或进度改变时，发出 dataChanged 信号通知视图刷新
 * 4. 视图调用 data() 方法获取显示内容
 *
 * 数据角色支持：
 * - Qt::DisplayRole：显示文本（设备名、文件名、大小、状态、进度）
 * - Qt::ForegroundRole：状态列的文字颜色（根据状态不同显示不同颜色）
 *
 * 典型使用场景：
 * @code
 * // 创建模型并绑定到视图
 * TransferTableModel* model = new TransferTableModel();
 * QTableView* view = new QTableView();
 * view->setModel(model);
 *
 * // 开始新的传输任务
 * Transfer* transfer = new Sender(...);
 * model->insertTransfer(transfer);
 *
 * // 清理已完成的任务
 * model->clearCompleted();
 * @endcode
 *
 * 内存管理：
 * - 模型拥有所有 Transfer 对象的所有权
 * - 析构时会自动删除所有 Transfer 对象
 * - 移除任务时会调用 deleteLater 延迟删除
 */
class TransferTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent Qt 父对象，用于对象树管理
     *
     * 创建一个空的传输表格模型
     */
    explicit TransferTableModel(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     *
     * 删除所有 Transfer 对象，释放资源
     */
    ~TransferTableModel() override;

    // === Qt 模型接口实现 ===

    /**
     * @brief 获取行数（Qt 模型接口）
     * @param parent 父索引（表格模型中未使用）
     * @return 传输任务的数量
     *
     * 视图会调用此方法确定显示多少行
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief 获取列数（Qt 模型接口）
     * @param parent 父索引（表格模型中未使用）
     * @return 固定为 5 列（Peer、FileName、FileSize、State、Progress）
     *
     * 视图会调用此方法确定显示多少列
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief 获取指定单元格的数据（Qt 模型接口）
     * @param index 模型索引（包含行号和列号）
     * @param role 数据角色
     * @return QVariant 包装的数据
     *
     * 根据不同的角色和列返回不同的数据：
     * - DisplayRole：显示文本（设备名/文件名/大小/状态/进度）
     * - ForegroundRole：状态列的文字颜色
     *
     * 视图在渲染每个单元格时会调用此方法
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief 获取表头数据（Qt 模型接口）
     * @param section 列号或行号
     * @param orientation 方向（水平或垂直）
     * @param role 数据角色
     * @return QVariant 包装的表头文本
     *
     * 返回表头显示的文本：节点、文件路径、大小、状态、进度
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // === 传输任务管理 ===

    /**
     * @brief 插入传输任务到模型
     * @param t Transfer 对象指针
     *
     * 将新的传输任务添加到列表开头（最新的任务显示在最上面）。
     * 自动连接 TransferInfo 的信号，监听状态变化和进度更新。
     *
     * 连接的信号：
     * - fileOpened：文件打开时更新文件名和大小列
     * - stateChanged：状态改变时更新状态列
     *
     * 模型获得 Transfer 对象的所有权，会在析构或移除时删除
     */
    void insertTransfer(Transfer* t);

    /**
     * @brief 清理已完成的传输任务
     *
     * 移除所有处于终止状态的任务：
     * - Idle：空闲
     * - Finish：完成
     * - Disconnected：断开
     * - Cancelled：取消
     *
     * 用于清理传输列表，只保留正在进行的任务
     */
    void clearCompleted();

    // === 任务查询 ===

    /**
     * @brief 根据索引获取 Transfer 对象
     * @param index 任务在列表中的索引（0-based）
     * @return Transfer 对象指针，索引无效时返回 nullptr
     */
    Transfer* getTransfer(int index) const;

    /**
     * @brief 根据索引获取 TransferInfo 对象
     * @param index 任务在列表中的索引（0-based）
     * @return TransferInfo 对象指针
     *
     * 这是 getTransfer(index)->getTransferInfo() 的快捷方法
     */
    TransferInfo* getTransferInfo(int index) const;

    /**
     * @brief 移除指定索引的传输任务
     * @param index 任务在列表中的索引（0-based）
     *
     * 从列表中移除任务并调用 deleteLater 延迟删除 Transfer 对象
     */
    void removeTransfer(int index);

    /**
     * @enum Column
     * @brief 表格列枚举
     *
     * 定义表格的列顺序和数量，Count 表示总列数
     */
    enum class Column : int {
        Peer = 0,   ///< 对方设备名称列
        FileName,   ///< 文件路径列
        FileSize,   ///< 文件大小列
        State,      ///< 传输状态列
        Progress,   ///< 传输进度列
        Count       ///< 列数（值为 5）
    };

private:
    /**
     * @brief 将传输状态转换为显示文本
     * @param state 传输状态
     * @return 状态的中文文本（空闲/等待/传输/暂停/完成等）
     */
    QString getStateString(TransferState state) const;

    /**
     * @brief 获取传输状态对应的颜色
     * @param state 传输状态
     * @return QColor 对象
     *
     * 颜色映射：
     * - Idle：黑色
     * - Waiting：橙色
     * - Transfering：蓝色
     * - Paused：橙色
     * - Finish：绿色
     * - Disconnected：红色
     * - Cancelled：红色
     */
    QColor getStateColor(TransferState state) const;

    QVector<Transfer*> mTransfers;    ///< 传输任务列表，存储所有 Transfer 对象指针

};

#endif // TRANSFERTABLEMODEL_H
