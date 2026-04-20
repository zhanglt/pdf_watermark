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
 * @file transferinfo.cpp
 * @brief 传输信息类实现文件
 *
 * 本文件实现了 TransferInfo 类的所有方法，包括状态管理、属性设置和操作能力判断。
 * 核心功能是通过状态机模式管理传输状态转换，确保状态切换的合法性和一致性。
 */

#pragma execution_character_set("utf-8")  // 设置源码字符编码为UTF-8，支持中文

#include "transferinfo.h"

/**
 * @brief 构造函数实现
 *
 * 初始化所有成员变量为默认值，传输开始时处于 Idle 状态
 */
TransferInfo::TransferInfo(Transfer* owner, QObject *parent) :
    QObject(parent),
    mState(TransferState::Idle), mLastState(TransferState::Idle),
    mType(TransferType::None), mProgress(0), mDataSize(0),
    mOwner(owner)
{
}

/**
 * @brief 判断是否可以恢复传输
 *
 * 只有当前状态为 Paused（暂停）时才能恢复
 */
bool TransferInfo::canResume() const
{
    return mState == TransferState::Paused;
}

/**
 * @brief 判断是否可以暂停传输
 *
 * 只有在 Waiting（等待）或 Transfering（传输中）状态时才能暂停
 */
bool TransferInfo::canPause() const
{
    return mState == TransferState::Waiting ||
            mState == TransferState::Transfering;
}

/**
 * @brief 判断是否可以取消传输
 *
 * 在 Waiting（等待）、Transfering（传输中）或 Paused（暂停）状态时可以取消
 */
bool TransferInfo::canCancel() const
{
    return mState == TransferState::Waiting ||
            mState == TransferState::Transfering ||
            mState == TransferState::Paused;
}

/**
 * @brief 设置对方设备信息
 *
 * 使用移动语义优化性能
 */
void TransferInfo::setPeer(Device peer)
{
    mPeer = std::move(peer);
}

/**
 * @brief 设置传输状态
 *
 * 使用状态机模式管理状态转换。根据当前状态和目标状态，验证转换是否合法。
 * 只有符合状态转换规则的切换才会生效，并发出 stateChanged 信号。
 *
 * 状态转换规则：
 * - Idle：只能转换到 Waiting
 * - Waiting：可以转换到 Transfering、Cancelled、Paused
 * - Transfering：可以转换到 Disconnected、Finish、Cancelled、Paused
 * - Paused：可以恢复到 Waiting/Transfering，或转换到 Cancelled、Disconnected
 *
 * 特殊处理：
 * - 从 Paused 恢复时，会恢复到 mLastState 记录的之前状态
 * - 每次状态改变前，会保存当前状态到 mLastState
 *
 * @param newState 目标状态
 */
void TransferInfo::setState(TransferState newState)
{
    if (newState != mState) {
        TransferState tmp = mState;

        switch (mState) {
        case TransferState::Idle : {
            // Idle 状态只能转换到 Waiting
            if (newState == TransferState::Waiting) {
                mState = newState;
                emit stateChanged(mState);
            }
            break;
        }
        case TransferState::Waiting : {
            // Waiting 状态可以开始传输、被取消或暂停
            if (newState == TransferState::Transfering ||
                    newState == TransferState::Cancelled ||
                    newState == TransferState::Paused) {
                mState = newState;
                emit stateChanged(mState);
            }
            break;
        }
        case TransferState::Transfering : {
            // Transfering 状态可以完成、断开、被取消或暂停
            if (newState == TransferState::Disconnected ||
                    newState == TransferState::Finish ||
                    newState == TransferState::Cancelled ||
                    newState == TransferState::Paused) {
                mState = newState;
                emit stateChanged(mState);
            }
            break;
        }
        case TransferState::Paused : {
            // Paused 状态可以恢复或被取消/断开
            if (newState == TransferState::Waiting ||
                    newState == TransferState::Transfering) {
                // 恢复时，返回到暂停前的状态
                mState = mLastState;
                emit stateChanged(mState);
            }
            else if (newState == TransferState::Cancelled ||
                     newState == TransferState::Disconnected) {
                mState = newState;
                emit stateChanged(mState);
            }
            break;
        }
        default:
            break;
        }

        // 保存当前状态，用于暂停恢复
        mLastState = tmp;
    }
}

/**
 * @brief 设置传输进度
 *
 * 进度值发生变化时会发出 progressChanged 信号，UI 层可监听此信号更新进度条
 *
 * @param newProgress 新的进度值 (0-100)
 */
void TransferInfo::setProgress(int newProgress)
{
    if (newProgress != mProgress) {
        mProgress = newProgress;
        emit progressChanged(mProgress);
    }
}

/**
 * @brief 设置传输类型
 *
 * @param type 传输类型（Upload 或 Download）
 */
void TransferInfo::setTransferType(TransferType type)
{
    mType = type;
}

/**
 * @brief 设置数据总大小
 *
 * @param size 文件大小（字节数）
 */
void TransferInfo::setDataSize(qint64 size)
{
    mDataSize = size;
}

/**
 * @brief 设置文件路径
 *
 * @param fileName 文件完整路径
 */
void TransferInfo::setFilePath(const QString &fileName)
{
    mFilePath = fileName;
}
