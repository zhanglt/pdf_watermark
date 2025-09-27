/**
 * @file FileSystemUtils.cpp
 * @brief 文件系统操作工具模块实现
 * 
 * 该模块实现了文件和目录操作的各种实用工具函数，
 * 为PDF处理应用提供基础的文件系统操作支持。
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma execution_character_set("utf-8") // 设置编译器使用UTF-8字符集
#include "include/function/FileSystemUtils.h"

namespace FileSystemUtils {

/**
 * @brief 递归遍历目录及其子目录，收集指定类型的文件
 * 将匹配条件的文件路径添加到结果列表中
 * @param dir 要遍历的目录
 * @param resultList 存放结果文件路径的列表
 * @param fileType 要查找的文件类型（扩展名），如"pdf"
 * @param exclude 要排除的文件名关键字，包含此字符串的文件将被忽略
 */
void traverseDirectory(const QDir& dir, QStringList& resultList,
                       QString fileType, QString exclude) {
    // 获取当前目录下的所有文件
    QStringList fileList = dir.entryList(QDir::Files);
    
    foreach (const QString& file, fileList) {
        QFileInfo fileInfo(file);
        QString extension = fileInfo.suffix();  // 获取文件扩展名
        
        // 检查文件是否符合条件：
        // 1. 扩展名匹配指定类型
        // 2. 不包含排除关键字
        // 3. 不包含"_pdf_"标记（避免重复处理）
        if ((extension.toLower() == fileType) &&
            (!dir.filePath(file).contains(exclude)) &&
            (!dir.filePath(file).contains("_pdf_"))) {
            resultList.append(dir.filePath(file));  // 添加符合条件的文件路径
        }
    }
    
    // 获取子目录列表（排除"." 和 ".."）
    QStringList dirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    // 递归遍历每个子目录
    foreach (const QString& subdir, dirList) {
        QDir subdirectory = dir;
        subdirectory.cd(subdir);  // 进入子目录
        traverseDirectory(subdirectory, resultList, fileType, exclude);
    }
}

/**
 * @brief 遍历一个目录及其子目录中的所有文件，并将其中的指定类型文件路径添加到结果列表中
 * @param dir 要遍历的目录
 * @param resultList 存放结果文件路径的列表
 * @param fileTypes 要查找的文件类型列表
 */
void tDirectory(const QDir& dir, QStringList& resultList,
                const QVariantList& fileTypes) {
    QStringList fileList = dir.entryList(QDir::Files);  // 获取当前目录下的文件列表
    
    foreach (const QString& file, fileList) {
        QFileInfo fileInfo(file);
        QString extension = fileInfo.suffix();  // 获取文件扩展名
        
        // 检查文件扩展名是否匹配任何一种指定的文件类型
        for (const QVariant& fileType : fileTypes) {
            QString Value = fileType.toString().toLower();
            if (extension.toLower() == Value) {  // 如果文件类型匹配
                resultList.append(dir.filePath(file));  // 将文件路径添加到结果列表中
            }
        }
    }
    
    // 获取子目录列表（排除"." 和 ".."）
    QStringList dirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    // 递归遍历每个子目录
    foreach (const QString& subdir, dirList) {
        QDir subdirectory = dir;  // 创建子目录的QDir对象
        subdirectory.cd(subdir);  // 切换到子目录
        tDirectory(subdirectory, resultList, fileTypes);  // 递归遍历子目录
    }
}

/**
 * @brief 根据输入路径生成新的输出路径
 * 将文件路径中的根目录替换为新的输出目录，并可选地插入一个标识字符串
 * @param rootInput 原始根目录路径
 * @param rootOutput 新的输出根目录路径
 * @param file 完整的文件路径
 * @param insert 要插入的标识字符串（如"_watermarked"）
 * @return 生成的新文件路径
 */
QString pathChange(QString rootInput, QString rootOutput, QString file,
                   QString insert) {
    QFileInfo fileInfo(file);
    QString directory = fileInfo.path();    // 获取文件所在目录
    QString fileName = fileInfo.fileName();  // 获取文件名部分
    
    // 计算相对于根目录的子路径
    QString suffix = directory.right(directory.length() - rootInput.length());
    
    // 构造新的输出路径：输出根目录 + 插入字符串 + 子路径
    QString newPath = rootOutput + "/" + insert + suffix + "/";

    createDirectory(newPath);  // 创建目录（如果不存在）
    return newPath + fileName; // 返回完整的新文件路径
}

/**
 * @brief 从完整文件路径中提取文件名部分
 * 支持Windows和Unix风格的路径分隔符
 * @param fullPath 完整的文件路径
 * @return 文件名部分（不包含路径）
 */
std::string extractFileName(const std::string& fullPath) {
    // 查找最后一个路径分隔符（'/' 或 '\\'）的位置
    size_t pos = fullPath.find_last_of("/\\");

    if (pos != std::string::npos) {
        return fullPath.substr(pos + 1);  // 返回分隔符之后的部分
    } else {
        return fullPath;  // 没有路径分隔符，返回整个字符串
    }
}

/**
 * @brief 从完整文件路径中提取目录路径部分
 * @param fullPath 完整的文件路径
 * @return 目录路径部分（不包含文件名）
 */
std::string extractDirectory(const std::string& fullPath) {
    // 查找最后一个路径分隔符的位置
    size_t pos = fullPath.find_last_of("/\\");

    if (pos != std::string::npos) {
        return fullPath.substr(0, pos);  // 返回分隔符之前的部分
    } else {
        return "";  // 没有路径分隔符，返回空字符串
    }
}

/**
 * @brief 创建目录（包括中间目录）
 * 如果目录不存在，则递归创建所有必要的父目录
 * @param path 要创建的目录路径
 * @return true表示创建成功或目录已存在，false表示创建失败
 */
bool createDirectory(const QString& path) {
    QDir dir(path);
    if (!dir.exists()) {
        if (dir.mkpath(path)) {  // mkpath会递归创建所有必要的父目录
            return true;
        } else {
            qWarning() << "Failed to create directory: " << path;
            return false;
        }
    } else {
        return true;  // 目录已存在
    }
}

} // namespace FileSystemUtils
