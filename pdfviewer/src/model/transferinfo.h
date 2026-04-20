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
 * @file transferinfo.h
 * @brief 传输信息类头文件
 *
 * 本文件定义了 TransferInfo 类及相关的枚举类型，用于表示文件传输任务的状态和信息。
 * TransferInfo 封装了一次文件传输的所有元数据，包括对方设备、传输进度、文件信息等，
 * 并通过信号机制通知状态变化。
 *
 * 主要内容：
 * - TransferState：传输状态枚举，定义传输生命周期的各个阶段
 * - TransferType：传输类型枚举，区分上传和下载
 * - TransferInfo：传输信息类，管理单个传输任务的所有信息和状态
 */

#ifndef TRANSFERINFO_H
#define TRANSFERINFO_H

#include <QObject>

#include "device.h"

/**
 * @enum TransferState
 * @brief 传输状态枚举
 *
 * 定义文件传输过程中的所有可能状态。状态按传输生命周期排列，
 * 通过状态机模式管理状态转换，确保状态切换的合法性。
 *
 * 状态转换规则：
 * - Idle -> Waiting：开始传输
 * - Waiting -> Transfering：连接建立，开始数据传输
 * - Waiting -> Cancelled：用户取消
 * - Waiting -> Paused：用户暂停
 * - Transfering -> Finish：传输完成
 * - Transfering -> Disconnected：连接断开
 * - Transfering -> Cancelled：用户取消
 * - Transfering -> Paused：用户暂停
 * - Paused -> Waiting/Transfering：恢复传输
 * - Paused -> Cancelled：从暂停状态取消
 */
enum class TransferState {
    Idle,         ///< 空闲状态：传输对象创建但未开始
    Waiting,      ///< 等待状态：等待连接建立或等待对方确认
    Disconnected, ///< 断开状态：网络连接异常断开
    Paused,       ///< 暂停状态：用户主动暂停传输
    Cancelled,    ///< 取消状态：用户主动取消传输
    Transfering,  ///< 传输状态：正在进行数据传输
    Finish        ///< 完成状态：传输成功完成
};

/**
 * @enum TransferType
 * @brief 传输类型枚举
 *
 * 区分文件传输的方向，从本机视角定义。
 */
enum class TransferType {
    None,     ///< 无类型：未初始化或未定义
    Download, ///< 下载：从对方设备接收文件到本机
    Upload    ///< 上传：从本机发送文件到对方设备
};

class Transfer;

/**
 * @class TransferInfo
 * @brief 传输信息类
 *
 * TransferInfo 类封装单个文件传输任务的所有信息和状态。它作为传输层(Transfer)
 * 和表现层(UI/Model)之间的数据接口，提供以下功能：
 *
 * 1. **信息存储**：保存对方设备、文件路径、文件大小、传输进度等元数据
 * 2. **状态管理**：通过状态机模式管理传输状态转换，确保状态切换合法
 * 3. **事件通知**：通过 Qt 信号机制通知状态变化、进度更新等事件
 * 4. **操作控制**：提供暂停、恢复、取消等操作的能力判断接口
 *
 * TransferInfo 对象通常与 Transfer 对象一对一关联：
 * - Transfer：负责实际的网络通信和数据传输
 * - TransferInfo：负责信息存储和状态管理
 *
 * 典型使用场景：
 * - 在 UI 层显示传输列表（通过 TransferTableModel）
 * - 监听传输进度更新进度条
 * - 根据状态启用/禁用操作按钮（暂停、恢复、取消）
 */
class TransferInfo : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param owner 拥有此信息对象的 Transfer 对象指针
     * @param parent Qt 父对象，用于对象树管理
     *
     * 创建一个传输信息对象，初始化所有字段为默认值：
     * - 状态：Idle
     * - 类型：None
     * - 进度：0
     * - 数据大小：0
     */
    explicit TransferInfo(Transfer* owner, QObject *parent = nullptr);

    // === Getter 方法 ===

    /**
     * @brief 获取对方设备信息
     * @return Device 对象，表示文件传输的对方设备
     */
    inline Device getPeer() const { return mPeer; }

    /**
     * @brief 获取传输进度
     * @return 进度百分比 (0-100)
     */
    inline int getProgress() const { return mProgress; }

    /**
     * @brief 获取当前传输状态
     * @return TransferState 枚举值
     */
    inline TransferState getState() const { return mState; }

    /**
     * @brief 获取上一次的传输状态
     * @return TransferState 枚举值
     *
     * 用于暂停后恢复到之前的状态（Waiting 或 Transfering）
     */
    inline TransferState getLastState() const { return mLastState; }

    /**
     * @brief 获取传输类型
     * @return TransferType 枚举值（Upload 或 Download）
     */
    inline TransferType getTransferType() const { return mType; }

    /**
     * @brief 获取数据总大小
     * @return 文件大小（字节数）
     */
    inline qint64 getDataSize() const { return mDataSize; }

    /**
     * @brief 获取文件路径
     * @return 文件完整路径字符串
     */
    inline QString getFilePath() const { return mFilePath; }

    /**
     * @brief 获取拥有者 Transfer 对象
     * @return Transfer 对象指针
     *
     * 返回与此信息对象关联的 Transfer 对象，用于执行实际的传输操作
     */
    inline Transfer* getOwner() const { return mOwner; }

    // === 操作能力判断 ===

    /**
     * @brief 判断是否可以恢复传输
     * @return true 表示当前可以恢复（状态为 Paused）
     */
    bool canResume() const;

    /**
     * @brief 判断是否可以暂停传输
     * @return true 表示当前可以暂停（状态为 Waiting 或 Transfering）
     */
    bool canPause() const;

    /**
     * @brief 判断是否可以取消传输
     * @return true 表示当前可以取消（状态为 Waiting、Transfering 或 Paused）
     */
    bool canCancel() const;

    // === Setter 方法 ===

    /**
     * @brief 设置对方设备信息
     * @param peer Device 对象
     */
    void setPeer(Device peer);

    /**
     * @brief 设置传输状态
     * @param state 新的传输状态
     *
     * 使用状态机模式验证状态转换的合法性，只有符合转换规则的状态切换才会生效。
     * 状态改变时会发出 stateChanged 信号。
     */
    void setState(TransferState state);

    /**
     * @brief 设置传输类型
     * @param type 传输类型（Upload 或 Download）
     */
    void setTransferType(TransferType type);

    /**
     * @brief 设置传输进度
     * @param progress 新的进度值 (0-100)
     *
     * 进度改变时会发出 progressChanged 信号
     */
    void setProgress(int progress);

    /**
     * @brief 设置数据总大小
     * @param size 文件大小（字节数）
     */
    void setDataSize(qint64 size);

    /**
     * @brief 设置文件路径
     * @param fileName 文件完整路径
     */
    void setFilePath(const QString& fileName);

Q_SIGNALS:
    /**
     * @brief 传输完成信号
     *
     * 当传输成功完成时发出
     */
    void done();

    /**
     * @brief 错误发生信号
     * @param errStr 错误描述字符串
     *
     * 当传输过程中发生错误时发出
     */
    void errorOcurred(const QString& errStr);

    /**
     * @brief 进度变化信号
     * @param progress 新的进度值 (0-100)
     *
     * 当传输进度更新时发出，UI 层可监听此信号更新进度条
     */
    void progressChanged(int progress);

    /**
     * @brief 文件打开信号
     *
     * 当文件成功打开（发送或接收）时发出
     */
    void fileOpened();

    /**
     * @brief 状态变化信号
     * @param state 新的传输状态
     *
     * 当传输状态改变时发出，UI 层可监听此信号更新界面显示
     */
    void stateChanged(TransferState state);

private:
    Device mPeer;                ///< 对方设备信息
    TransferState mState;        ///< 当前传输状态
    TransferState mLastState;    ///< 上一次的传输状态（用于暂停恢复）
    TransferType mType;          ///< 传输类型（上传或下载）
    int mProgress;               ///< 传输进度 (0-100)
    qint64 mDataSize;            ///< 数据总大小（字节）
    QString mFilePath;           ///< 文件完整路径

    Transfer* mOwner;            ///< 拥有此信息对象的 Transfer 对象指针
};

#endif // TRANSFERINFO_H
