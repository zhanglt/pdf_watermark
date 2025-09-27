/**
 * @file FileDetector.cpp
 * @brief 文件类型检测模块实现
 * 
 * 该模块提供各种文件类型的检测功能，通过扩展名和文件头魔数
 * 来准确识别PDF文件、图片文件等，并提供字符串验证功能。
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma execution_character_set("utf-8") // 设置编译器使用UTF-8字符集
#include "include/function/FileDetector.h"

namespace FileDetector {

/**
 * @brief 通过文件扩展名判断是否为PDF文件
 * @param fileName 文件名（包含扩展名）
 * @return true表示是PDF文件，false表示不是
 */
bool isPDFByExtension(const QString& fileName) {
    return fileName.toLower().endsWith(".pdf");
}

/**
 * @brief 通过文件头魔数判断是否为PDF文件
 * PDF文件的前4个字节是"%PDF"
 * @param filePath 文件路径
 * @return true表示是PDF文件，false表示不是
 */
bool isPDFByMagicNumber(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray magicNumber = file.read(4);  // 读取前4个字节
        file.close();
        return magicNumber.startsWith("%PDF");  // 检查是否以"%PDF"开头
    }
    return false;
}

/**
 * @brief 通过文件扩展名判断是否为图片文件
 * 支持的图片格式：jpg、jpeg、png、bmp
 * @param fileName 文件名
 * @return true表示是图片文件，false表示不是
 */
bool isImageByExtension(const QString& fileName) {
    QString ext = fileName.toLower().right(4);  // 获取后缀（最后4个字符）
    return ext == ".jpg" || ext == "jpeg" || ext == ".png" || ext == ".bmp";
}

/**
 * @brief 通过文件头魔数判断是否为图片文件
 * 检查JPEG、PNG、BMP格式的魔数
 * @param filePath 文件路径
 * @return true表示是图片文件，false表示不是
 */
bool isImageByMagicNumber(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray magicNumber = file.read(4);  // 读取前4个字节
        file.close();
        return magicNumber.startsWith(QByteArray("\xFF\xD8\xFF", 3))         // JPEG魔数
               || magicNumber.startsWith(QByteArray("\x89\x50\x4E\x47", 4))  // PNG魔数
               || magicNumber.startsWith(QByteArray("BM", 2));               // BMP魔数
    }
    return false;
}

/**
 * @brief 判断字符串是否为纯数字
 * @param str 要检查的字符串
 * @return true表示是纯数字，false表示不是
 */
bool isNumeric(const QString& str) {
    QRegularExpression re("\\d+");  // 正则表达式：匹配一个或多个数字
    QRegularExpressionMatch match = re.match(str);
    // 检查是否匹配且匹配长度等于字符串长度（即整个字符串都是数字）
    return match.hasMatch() && match.capturedLength() == str.length();
}

} // namespace FileDetector
