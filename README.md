# PDF Widgets - 专业的 PDF 工具集

<div align="center">
  <img src="doc/logo.png" alt="PDF Widgets Logo" width="200"/>
  
  [![Qt Version](https://img.shields.io/badge/Qt-5.x-green.svg)](https://www.qt.io/)
  [![C++](https://img.shields.io/badge/C++-11-blue.svg)](https://isocpp.org/)
  [![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)
  [![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
</div>

## 📖 项目简介

PDF Widgets 是一款基于 Qt 框架开发的功能强大的 PDF 文档处理工具集。它提供了从基础的 PDF 查看到高级的批量处理等全方位的 PDF 操作功能，旨在为用户提供一站式的 PDF 解决方案。

### ✨ 核心特性

- 🔍 **PDF 查看器** - 流畅的文档浏览体验，支持缩放、翻页、连续显示
- 💧 **批量水印处理** - 高效的水印添加功能，支持自定义样式和批量操作
- 🔄 **格式转换** - PDF 与图片格式互转，保持高质量输出
- ✂️ **文档拆分** - 灵活的拆分选项，支持按页数或范围拆分
- 🔗 **文档合并** - 多个 PDF 文件合并为一个，保持原有格式
- 🔎 **内容搜索** - 在 PDF 和 Excel 文件中快速搜索关键内容

## 📋 系统要求

### 运行环境
- **操作系统**: Windows 7 及以上版本
- **处理器**: 双核 2.0GHz 或更高
- **内存**: 4GB RAM（推荐 8GB）
- **硬盘空间**: 至少 500MB 可用空间

### 开发依赖
- **Qt Framework**: 5.12 或更高版本
  - Qt Core
  - Qt Gui
  - Qt Widgets
  - Qt PrintSupport
  - Qt Svg
  - Qt PDFWidgets
- **第三方库**:
  - MuPDF - PDF 渲染引擎
  - PDFlib - 高级 PDF 操作
  - libxml2 - XML/SVG 处理
  - libiconv - 字符编码转换
  - Qt5Xlsx - Excel 文件处理

## 🛠️ 安装说明

### 从源码构建

1. **克隆仓库**
```bash
git clone https://github.com/yourusername/pdfwidgets.git
cd pdfwidgets
```

2. **配置环境**
确保已安装 Qt 5.12+ 和所需的第三方库。

3. **构建项目**
```bash
# 生成 Makefile
qmake pdfwidgets.pro

# 编译项目
make  # Linux/macOS
# 或
nmake  # Windows with MSVC
```

4. **运行程序**
```bash
./pdfviewer/pdfviewer  # Linux/macOS
# 或
pdfviewer\release\pdfviewer.exe  # Windows
```

### 构建选项
```bash
# 调试版本
qmake CONFIG+=debug

# 发布版本（优化）
qmake CONFIG+=release
```

## 📚 使用指南

### 1. PDF 查看功能
- **打开文档**: 文件 → 打开 或 Ctrl+O
- **缩放控制**: 
  - 放大: Ctrl + =
  - 缩小: Ctrl + -
- **页面导航**:
  - 上一页: Page Up
  - 下一页: Page Down
- **显示模式**: 视图 → 滚动显示（连续页面模式）

### 2. 水印添加功能
1. 切换到"水印操作"标签页
2. 设置水印参数：
   - **水印文本**: 支持多行文本（使用换行符分隔）
   - **字体设置**: 新宋体、楷体、仿宋、黑体
   - **字体大小**: 8-50pt 可调
   - **颜色选择**: 点击"字体颜色"按钮选择
   - **旋转角度**: 0-180度
   - **透明度**: 0-100%
3. 选择输入/输出目录
4. 点击"增加水印"开始处理
5. 可选：点击"导出PDF"将结果导出为图片PDF

### 3. 格式转换功能

#### 图片转 PDF
- **单张转换**: 选择单个图片文件 → 转换
- **批量转换**: 选择包含图片的目录 → 批量转换

#### PDF 转图片
- 选择 PDF 文件
- 设置输出分辨率（72-300 DPI）
- 点击"转换"

### 4. 文件拆分功能
- **平均拆分**: 指定每个文件包含的页数
- **范围拆分**: 输入页面范围（如: 1-7,4-12,10-20）
- 选择输出目录
- 点击"开始拆分"

### 5. 文件合并功能
1. 点击"增加文件"或拖放 PDF 文件到表格
2. 调整文件顺序（拖动排序）
3. 设置输出文件名
4. 选择输出目录
5. 点击"开始合并文件"

### 6. 内容搜索功能
1. 选择包含 Excel 文件的目录
2. 输入搜索关键字
3. 点击"搜索"
4. 查看搜索结果树形列表
5. 可选：点击"导出"保存搜索结果

## 🏗️ 项目架构

### 目录结构
```
pdfwidgets/
├── pdfviewer/              # 主应用程序
│   ├── main.cpp           # 程序入口
│   ├── mainwindow.cpp/h   # 主窗口实现
│   ├── function.h         # 功能函数聚合
│   ├── include/           # 头文件目录
│   │   ├── function/      # 功能模块头文件
│   │   ├── mark/          # 水印处理组件
│   │   ├── pdf2image/     # PDF转图片组件
│   │   └── search/        # 搜索功能组件
│   ├── src/               # 源代码实现
│   │   ├── controllers/   # MVC控制器
│   │   └── function/      # 功能模块实现
│   └── lib/               # 第三方库封装
└── doc/                   # 文档和资源
```

### 功能模块

项目采用模块化设计，主要功能模块包括：

1. **FileDetector** - 文件类型检测
2. **StringConverter** - 字符串编码转换
3. **FileSystemUtils** - 文件系统操作
4. **FormatConverter** - 格式转换处理
5. **PdfOperations** - PDF 基础操作
6. **WatermarkProcessor** - 水印处理
7. **GeometryUtils** - 几何计算工具

### 多线程架构
- 使用 `QThreadPool` 管理线程池
- 采用 `QRunnable` 模式实现并发任务
- 通过 Qt 信号槽机制进行线程间通信

## 💻 开发指南

### 代码规范
- 使用 UTF-8 编码
- 遵循 Qt 编码规范
- 类名使用 PascalCase
- 函数名使用 camelCase

### 扩展新功能
1. 在 `include/function/` 创建新的头文件
2. 在 `src/function/` 实现功能
3. 在 `function.h` 中添加接口
4. 更新 UI 和控制逻辑

### 构建配置
项目使用 qmake 构建系统，主要配置文件：
- `pdfwidgets.pro` - 顶层项目文件
- `pdfviewer/pdfviewer.pro` - 应用程序配置

## 🤝 贡献指南

欢迎提交 Pull Request 或创建 Issue！

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 🙏 致谢

- Qt 开源社区
- MuPDF 开发团队
- 所有贡献者和用户

---

<div align="center">
  <p>如有问题或建议，欢迎创建 <a href="https://github.com/zhanglt/pdfwidgets/issues">Issue</a></p>
  <p>⭐ 如果这个项目对您有帮助，请给我们一个 Star！</p>
</div>