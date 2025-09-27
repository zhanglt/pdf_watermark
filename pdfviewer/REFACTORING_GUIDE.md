# MainWindow 重构指南

## 概述
本文档说明了如何将 `mainwindow.cpp` 文件从1530行拆分成多个更小、更易维护的控制器类。

## 文件结构变化

### 原始结构
```
pdfviewer/
├── mainwindow.h        (141行)
└── mainwindow.cpp      (1530行)
```

### 重构后结构
```
pdfviewer/
├── mainwindow_refactored.h      (~100行)
├── mainwindow_refactored.cpp     (~300行)
└── src/
    └── controllers/
        ├── PdfViewerController.h     (~100行)
        ├── PdfViewerController.cpp   (~200行)
        ├── WatermarkController.h     (~100行)
        ├── WatermarkController.cpp   (~300行)
        ├── PdfConverterController.h  (~100行)
        ├── PdfConverterController.cpp (~300行)
        ├── PdfSplitMergeController.h (~100行)
        ├── PdfSplitMergeController.cpp (~300行)
        ├── ExcelSearchController.h   (~100行)
        └── ExcelSearchController.cpp (~250行)
```

## 迁移步骤

### 1. 更新项目文件 (pdfviewer.pro)
在 SOURCES 部分添加：
```
SOURCES += src/controllers/PdfViewerController.cpp \
           src/controllers/WatermarkController.cpp \
           src/controllers/PdfConverterController.cpp \
           src/controllers/PdfSplitMergeController.cpp \
           src/controllers/ExcelSearchController.cpp

HEADERS += src/controllers/PdfViewerController.h \
           src/controllers/WatermarkController.h \
           src/controllers/PdfConverterController.h \
           src/controllers/PdfSplitMergeController.h \
           src/controllers/ExcelSearchController.h
```

### 2. 替换主窗口文件
将 `mainwindow.cpp` 和 `mainwindow.h` 替换为：
- `mainwindow_refactored.cpp` → `mainwindow.cpp`
- `mainwindow_refactored.h` → `mainwindow.h`

### 3. 编译注意事项
- 确保所有控制器的头文件路径正确
- 某些函数可能需要从 `function.h` 中导出为公共函数
- 检查信号槽连接是否正确

## 功能分布

### PdfViewerController
- PDF文档打开、查看
- 缩放控制
- 页面导航
- 书签处理

### WatermarkController
- 水印添加和预览
- 颜色、透明度、旋转角度设置
- 字体选择
- 批量水印处理

### PdfConverterController
- 图片转PDF
- PDF转图片
- 批量转换
- 导出PDF（文字PDF转图片PDF）

### PdfSplitMergeController
- PDF拆分（按页数、按范围）
- PDF合并
- 文件列表管理

### ExcelSearchController
- Excel文件搜索
- 搜索结果显示
- 结果导出

## 优势

1. **代码组织更清晰**：每个控制器专注于一个功能域
2. **更易维护**：每个文件都在300-400行以内
3. **便于团队协作**：不同开发者可以同时工作在不同控制器上
4. **更好的可测试性**：可以独立测试每个控制器
5. **降低耦合度**：控制器之间通过信号槽通信

## 注意事项

1. 控制器之间的通信通过信号槽机制
2. UI指针在控制器构造时传入，但控制器不拥有UI
3. 线程池由MainWindow管理，控制器只获得引用
4. 所有日志输出通过信号发送到MainWindow统一处理