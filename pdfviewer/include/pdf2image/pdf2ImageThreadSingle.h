/**
 * @file pdf2ImageThreadSingle.h
 * @brief PDF转图片单线程处理类头文件
 * @author PDF工具集项目组
 * @date 2024
 * 
 * 本文件定义了PDF转图片功能的单线程处理类。
 * 继承自QObject和QRunnable，支持在线程池中异步执行PDF转图片操作。
 * 
 * 主要功能：
 * - PDF文档转换为图片格式
 * - 支持自定义分辨率设置
 * - 双向转换：PDF转图片 / 图片转PDF
 * - 多线程异步处理
 * - 处理完成信号通知
 * 
 * 依赖库：Qt5框架、MuPDF或其他PDF处理库
 */

#ifndef PDF2IMAGETHREADSINGLE_H
#define PDF2IMAGETHREADSINGLE_H
#include <qmap.h>

#include <QMutex>         // 互斥锁支持
#include <QObject>        // Qt对象基类
#include <QRunnable>      // 可运行任务基类
#include <QVector>        // Qt动态数组

#include "qthreadpool.h"  // Qt线程池
using namespace std;

/**
 * @typedef wMap
 * @brief 水印映射表类型定义
 * 
 * 为QMap<QString, int>类型定义别名，用于存储文件名到处理状态的映射。
 */
typedef QMap<QString, int> wMap;  // typedef操作符为QMap起一别名

/**
 * @class pdf2imageThreadSingle
 * @brief PDF转图片单线程处理类
 * 
 * 继承自QObject和QRunnable，实现在独立线程中执行PDF转图片操作。
 * 支持设置转换参数、处理进度通知和双向转换功能。
 */
class pdf2imageThreadSingle : public QObject, public QRunnable {
  Q_OBJECT
 public:
  //   pdf2imageThreadSingle(QMutex *mutex=nullptr,int *count=0);
  // explicit  pdf2imageThreadSingle(QObject *parent = nullptr) :
  // QObject(parent), m_value(0) {}
  
  /**
   * @brief 构造函数
   * @param parent 父对象指针
   * 
   * 初始化PDF转图片处理线程对象，设置默认参数。
   */
  explicit pdf2imageThreadSingle(QObject* parent = nullptr);

  /**
   * @brief 设置源PDF文件路径
   * @param sourceFile 源PDF文件的完整路径
   */
  void setSourceFile(QString sourceFile);
  
  /**
   * @brief 设置输出图片路径
   * @param imagePath 图片输出目录路径
   */
  void setImagePath(QString imagePath);
  
  /**
   * @brief 设置目标文件路径
   * @param targetFile 目标文件的完整路径（用于图片转PDF时）
   */
  void setTargetFile(QString targetFile);
  
  /**
   * @brief PDF转图片核心处理函数
   * @param pdfFile PDF文件路径
   * @param imagePath 图片输出路径
   * @param resolution 图片分辨率（DPI）
   * @return 处理结果，0表示成功，其他值表示失败
   * 
   * 将PDF文档的所有页面转换为指定分辨率的图片文件。
   */
  int pdf2image(string pdfFile, string imagePath, int resolution);
  
  /**
   * @brief 设置图片输出分辨率
   * @param resolution 分辨率值（DPI，如72、150、300等）
   * 
   * 设置PDF转图片时的输出分辨率，影响图片质量和文件大小。
   */
  void setResolution(int resolution);
  
  /**
   * @brief 设置是否启用图片转PDF功能
   * @param is2pdf true表示启用图片转PDF，false表示PDF转图片
   * 
   * 控制转换方向：true为图片转PDF模式，false为PDF转图片模式。
   */
  void setIs2pdf(bool is2pdf);
  
  /**
   * @brief 线程主执行函数
   * 
   * QRunnable接口的实现，在线程池中执行的主要逻辑。
   * 根据设置的参数执行PDF转图片或图片转PDF操作。
   */
  void run() override;
  
 public slots:

 signals:
  /**
   * @brief 处理完成信号
   * 
   * 当PDF转图片或图片转PDF操作完成时发出此信号，
   * 通知主线程或其他组件处理已完成。
   */
  void addFinish();

 private:
  /**
   * @brief 图片输出分辨率
   * 
   * 存储PDF转图片时的输出分辨率设置，单位为DPI。
   */
  int m_resolution;
  
  /**
   * @brief 转换方向标志
   * 
   * true表示图片转PDF模式，false表示PDF转图片模式。
   * 默认值为true（图片转PDF）。
   */
  bool m_is2pdf = true;
  
  /**
   * @brief 文件路径变量
   * 
   * m_sourceFile: 源文件路径（PDF或图片）
   * m_imagePath: 图片文件路径或输出目录
   * m_targetFile: 目标文件路径（转换后的文件）
   */
  QString m_sourceFile, m_imagePath, m_targetFile;
};

#endif  // PDF2IMAGETHREADSINGLE_H
