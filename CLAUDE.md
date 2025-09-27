# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概览

这是一个基于 Qt 框架的 PDF 工具集项目（pdfwidgets），提供了 PDF 文档的查看、编辑、水印添加、格式转换等功能。项目主要使用 C++ 开发，集成了 MuPDF、PDFlib、libxml2 等第三方库。

## 构建系统

项目使用 Qt 的 qmake 构建系统：

```bash
# 在项目根目录下构建
qmake pdfwidgets.pro
make  # 或 nmake (Windows)

# 调试版本
qmake CONFIG+=debug
make

# 发布版本  
qmake CONFIG+=release
make
```

注意：由于用户要求只进行代码分析，不进行构建，以上命令仅供参考。

## 架构与核心组件

### 项目结构
- **pdfwidgets.pro**: 顶层项目文件，使用 SUBDIRS 模板
- **pdfviewer/**: 主应用程序目录
  - **mainwindow.cpp/h**: 主窗口类，协调所有功能模块
  - **function.cpp/h**: 核心 PDF 处理功能函数集合
  - **lib/pdflib.cpp/h**: PDF 库的 C++ 封装层

### 功能模块
1. **PDF 查看**: 基于 Qt PDF 模块 (`QPdfDocument`, `QPdfView`)
2. **水印处理**: 
   - `watermarkThread.cpp`: 批量水印处理线程
   - `watermarkThreadSingle.cpp`: 单文件水印处理
   - `multiWatermarkThreadSingle.cpp`: 多行文本水印处理
3. **PDF 转图片**: `pdf2ImageThreadSingle.cpp` 
4. **文本搜索**: `SearchThread.cpp`
5. **PDF 操作**: 拆分、合并、图片转PDF等功能在 `function.cpp` 中实现

### 依赖库
- **Qt5**: Core, Gui, Widgets, PrintSupport, Svg, PDFWidgets
- **MuPDF**: PDF 渲染和处理 (`libmupdf`, `libthirdparty`, `libmuthreads`)
- **PDFlib**: 高级 PDF 操作
- **libxml2**: XML 处理
- **libiconv**: 字符编码转换
- **Qt5Xlsx**: Excel 文件处理

### 多线程设计
项目使用 `QThreadPool` 管理多个工作线程：
- 水印添加、PDF转图片等耗时操作在独立线程中执行
- 使用 `QRunnable` 模式实现任务并发处理
- 通过信号槽机制与主线程通信

### UI 组件
- 自定义控件位于 `include/` 和 `src/` 目录下
- 使用 Qt Designer 设计的 `mainwindow.ui`
- 样式通过 `qss/pdf.css` 定义

## 开发注意事项

1. **字符编码**: 项目使用 UTF-8 编码，通过 `#pragma execution_character_set("utf-8")` 设置
2. **平台相关**: 主要针对 Windows 平台（使用 Windows API，如 `windows.h`）
3. **资源管理**: PDF 文档、图片等资源通过 `resources.qrc` 管理
4. **国际化**: 支持中文界面，翻译文件位于 `languages/` 目录