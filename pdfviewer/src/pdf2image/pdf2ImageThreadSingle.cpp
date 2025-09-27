/**
 * @file pdf2ImageThreadSingle.cpp
 * @brief PDF转图片单线程处理类实现文件
 * 
 * 实现了PDF文档到图片格式的转换功能，支持以下特性：
 * - 单个PDF文件转换为多个图片文件（每页一个图片）
 * - 可选择的分辨率设置
 * - 支持PNG格式输出
 * - 异常安全的资源管理
 * - 可选的图片转PDF功能（转换后再合并回PDF）
 * - 多线程支持（继承QRunnable）
 * 
 * 技术实现：
 * - 使用MuPDF库进行PDF解析和渲染
 * - 基于Qt的多线程框架（QThreadPool）
 * - 支持自定义输出路径和分辨率
 * - 完整的错误处理和资源清理
 * 
 * 依赖库：
 * - MuPDF (PDF处理)
 * - Qt Core (多线程、信号槽)
 */

#pragma execution_character_set("utf-8")  // 防止中文乱码
#include "include/pdf2image/pdf2ImageThreadSingle.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QThread>

#include "function.h"

/**
 * @brief PDF转图片线程构造函数
 * @param parent 父对象指针
 * 
 * 初始化PDF转图片处理线程：
 * - 继承QObject支持信号槽机制
 * - 继承QRunnable支持线程池执行
 * - 设置自动删除，任务完成后自动清理内存
 * - 初始化默认参数（分辨率、转换模式等）
 * 
 * @note 注释掉的构造函数是早期版本，支持互斥锁和计数器
 */
/*
// 早期版本的构造函数，支持互斥锁和计数器
pdf2imageThreadSingle::pdf2imageThreadSingle(QMutex *mutex,int *count)
:m_mutex(mutex)
{
    this->count=count;
    setAutoDelete(true);
}
*/
pdf2imageThreadSingle::pdf2imageThreadSingle(QObject* parent)
    : QObject(parent),      // 初始化Qt对象基类
      QRunnable()           // 初始化线程任务基类
{
  setAutoDelete(true);    // 设置任务完成后自动删除对象
}

/**
 * @brief 设置源PDF文件路径
 * @param sourceFile 要转换的PDF文件完整路径
 */
void pdf2imageThreadSingle::setSourceFile(QString sourceFile) {
  m_sourceFile = sourceFile;
}

/**
 * @brief 设置目标PDF文件路径（用于图片转PDF模式）
 * @param targetFile 转换后PDF文件的保存路径
 * @note 仅在m_is2pdf为true时使用此路径
 */
void pdf2imageThreadSingle::setTargetFile(QString targetFile) {
  m_targetFile = targetFile;
}

/**
 * @brief 设置图片输出目录路径
 * @param imagePath 生成图片文件的保存目录
 * @note 目录必须存在，生成的图片文件名格式为：0.png, 1.png, 2.png...
 */
void pdf2imageThreadSingle::setImagePath(QString imagePath) {
  m_imagePath = imagePath;
}

/**
 * @brief 设置图片输出分辨率
 * @param resolution 输出图片的DPI分辨率
 * @note 默认72 DPI，值越大图片越清晰但文件也越大
 */
void pdf2imageThreadSingle::setResolution(int resolution) {
  m_resolution = resolution;
}
/**
 * @brief 设置转换模式标志
 * @param is2pdf 转换模式选择
 *               - true: PDF→图片→PDF模式（先转图片再合并回PDF）
 *               - false: PDF→图片模式（仅转换为图片文件）
 * 
 * @note PDF→图片→PDF模式常用于PDF压缩或格式标准化
 */
void pdf2imageThreadSingle::setIs2pdf(bool is2pdf) { m_is2pdf = is2pdf; }
/**
 * @brief 线程主执行函数
 * 
 * 执行实际的PDF转换任务流程：
 * 1. 调用pdf2image()函数将PDF转换为图片文件
 * 2. 如果是PDF→图片→PDF模式，则调用images2pdf()将图片合并回PDF
 * 3. 发送完成信号通知上层组件
 * 4. 处理所有可能的异常情况并确保信号发出
 * 
 * @note 该函数在线程池中异步执行，不会阻塞主线程
 */
void pdf2imageThreadSingle::run() {
  try {
    // 执行PDF转图片操作，获取转换页数
    int num = pdf2image(m_sourceFile.toStdString(), m_imagePath.toStdString(),
                        m_resolution);
    
    // 如果转换成功且需要再次转为PDF，则执行图片合并操作
    if (num > 0 && m_is2pdf) {
      images2pdf(m_imagePath.toStdString(), m_targetFile.toStdString(), num);
    }
    
    // 发送任务完成信号
    emit addFinish();
    
  } catch (const std::exception& e) {
    // 捕获标准异常并记录错误信息
    qDebug() << "PDF转换异常:" << e.what();
    emit addFinish();  // 确保信号始终发出，否则可能导致上层组件死锁
    
  } catch (...) {
    // 捕获所有其他类型的异常
    qDebug() << "PDF转换发生未知异常";
    emit addFinish();  // 确保信号始终发出
  }
}


/**
 * @brief 将PDF文件转换为图片文件的核心函数
 * @param pdfFile PDF源文件路径
 * @param imagePath 图片输出目录路径
 * @param resolution 输出图片的DPI分辨率
 * @return 成功转换的页数，失败返回-1
 * 
 * 详细处理流程：
 * 1. 初始化MuPDF上下文和文档对象
 * 2. 注册文档处理器并打开PDF文件
 * 3. 获取PDF页数并验证有效性
 * 4. 计算缩放矩阵（基于分辨率）
 * 5. 逐页转换：渲染→生成Pixmap→保存PNG
 * 6. 清理所有资源并返回结果
 * 
 * @note 使用MuPDF库实现，支持完整的异常处理和资源清理
 */
int pdf2imageThreadSingle::pdf2image(string pdfFile, string imagePath, int resolution) {
  // 初始化MuPDF资源指针
  fz_context* ctx = nullptr;   // MuPDF上下文对象
  fz_document* doc = nullptr;  // PDF文档对象
  
  try {
    // 创建MuPDF上下文（内存管理器、异常处理器、默认缓存大小）
    ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    if (!ctx) {
      qDebug() << "创建MuPDF上下文失败";
      return -1;
    }
    
    // 注册文档处理器（PDF、XPS等格式支持）
    fz_register_document_handlers(ctx);
    
    // 打开PDF文档文件
    doc = fz_open_document(ctx, pdfFile.c_str());
    if (!doc) {
      qDebug() << "文件处理错误：" << QString::fromStdString(pdfFile);
      fz_drop_context(ctx);  // 清理上下文资源
      return -1;
    }
    
    // 获取PDF文档的总页数
    int num = fz_count_pages(ctx, doc);
    if (num <= 0) {
      qDebug() << "PDF页数无效：" << num;
      fz_drop_document(ctx, doc);  // 清理文档资源
      fz_drop_context(ctx);        // 清理上下文资源
      return -1;
    }
    
    // 计算缩放因子：目标分辨率 / 默认DPI(72)
    // 例如：300 DPI / 72 DPI = 4.17倍缩放
    float zoom = static_cast<float>(resolution) / 72.0f;
    
    // 创建缩放变换矩阵（同时对X和Y方向应用相同缩放）
    fz_matrix ctm = fz_scale(zoom, zoom);

    // 遍历PDF的每一页并转换为图片
    for (int i = 0; i < num; i++) {
      // 生成输出图片文件名：0.png, 1.png, 2.png...
      string fileName = imagePath + "/" + to_string(i) + ".png";
      fz_pixmap* pix = nullptr;  // 像素数据对象
      
      try {
        // 从指定页面创建像素数据（渲染过程）
        // 参数：上下文、文档、页码、变换矩阵、颜色空间、透明度
        pix = fz_new_pixmap_from_page_number(ctx, doc, i, ctm, fz_device_rgb(ctx), 0);
        
        if (pix) {
          // 将像素数据保存为PNG格式文件
          fz_save_pixmap_as_png(ctx, pix, fileName.c_str());
          // 释放当前页的像素数据内存
          fz_drop_pixmap(ctx, pix);
        }
        
      } catch (...) {
        // 单页处理失败，清理资源并继续处理下一页
        if (pix) fz_drop_pixmap(ctx, pix);
        qDebug() << "处理第" << i << "页时发生错误";
        continue; // 跳过当前页，继续处理下一页
      }
    }

    // 所有页面处理完成，清理MuPDF资源
    fz_drop_document(ctx, doc);  // 释放文档对象
    fz_drop_context(ctx);        // 释放上下文对象
    
    // 返回成功转换的页数
    return num;
    
  } catch (...) {
    // 发生严重异常，确保资源清理
    if (doc) fz_drop_document(ctx, doc);  // 安全释放文档对象
    if (ctx) fz_drop_context(ctx);        // 安全释放上下文对象
    qDebug() << "pdf2image发生严重异常";
    return -1;  // 返回失败标志
  }
}
