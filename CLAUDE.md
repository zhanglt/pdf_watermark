# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

PDF Widgets 是一款基于 Qt 框架的 PDF 文档处理工具集，提供 PDF 查看、水印添加、格式转换、文档拆分合并等功能。最近在 `syncFile` 分支添加了局域网文件传输和共享功能。

## 构建系统

项目使用 qmake 构建系统，采用子项目结构。

### 构建命令

```bash
# 在项目根目录下生成 Makefile
qmake pdfwidgets.pro

# Windows (MSVC)
nmake           # 编译 Release 版本
nmake clean     # 清理编译文件

# Linux/macOS
make
make clean
```

### 构建配置

```bash
# Debug 版本
qmake CONFIG+=debug

# Release 版本（默认，启用深度优化 -O2）
qmake CONFIG+=release

# 在 Qt Creator 中
# 项目已配置多个构建配置：Debug、Release、Profile、Debug2
# 当前活动构建：Release（构建目录：E:/test/qt/pdfwidgets）
```

### 运行应用程序

```bash
# Windows
pdfviewer\release\pdfviewer.exe

# Linux/macOS
./pdfviewer/pdfviewer
```

## 项目架构

### 目录结构

```
pdfwidgets/
├── pdfwidgets.pro          # 顶层项目文件（TEMPLATE = subdirs）
└── pdfviewer/              # 主应用程序子项目
    ├── pdfviewer.pro       # 应用程序配置文件
    ├── main.cpp            # 程序入口
    ├── mainwindow.cpp/h    # 主窗口（已废弃，迁移到 src/ui/mainwindow.*）
    ├── function.h          # 功能模块聚合头文件
    ├── include/            # 头文件目录
    │   ├── function/       # 核心功能模块头文件
    │   ├── mark/           # 水印处理头文件
    │   ├── pdf2image/      # PDF转图片头文件
    │   ├── search/         # 搜索功能头文件
    │   └── ...
    ├── src/                # 源代码实现
    │   ├── controllers/    # MVC 控制器（新架构）
    │   ├── function/       # 核心功能实现
    │   ├── transfer/       # 文件传输模块
    │   ├── share/          # 共享文件管理
    │   ├── model/          # 数据模型
    │   ├── ui/             # UI 对话框（新架构）
    │   └── ...
    └── lib/                # 第三方库
        ├── mupdf/          # MuPDF PDF渲染引擎
        ├── pdflib.dll      # PDFlib 商业库
        ├── libxml2/        # XML 处理
        ├── iconv/          # 字符编码转换
        └── qtxlsx/         # Excel 文件处理
```

### 核心模块

#### 1. Controllers（控制器层，MVC 模式）

位于 `pdfviewer/src/controllers/`，负责业务逻辑和 UI 交互：

- **PdfViewerController**: PDF 查看器控制器，管理文档打开、缩放、页面导航
- **WatermarkController**: 水印控制器，处理水印添加、预览、参数设置
- **PdfSplitMergeController**: PDF 拆分合并控制器
- **PdfConverterController**: 格式转换控制器（PDF ↔ 图片）
- **ExcelSearchController**: Excel 搜索控制器

#### 2. Function Modules（功能模块）

位于 `pdfviewer/src/function/` 和 `pdfviewer/include/function/`，提供核心算法：

- **FileDetector**: 文件类型检测（通过扩展名和魔数）
- **StringConverter**: 字符串编码转换（QString ↔ wstring ↔ string）
- **FileSystemUtils**: 文件系统操作（目录遍历、路径转换）
- **FormatConverter**: 格式转换（pdf2image, image2pdf, images2pdf）
- **PdfOperations**: PDF 基础操作（获取页数、拆分、合并）
- **WatermarkProcessor**: 水印处理（单行/多行水印、SVG 生成）
- **GeometryUtils**: 几何计算（文本宽度、旋转矩形、角度转换）

所有模块函数通过 `function.h` 聚合，提供全局函数别名以保持向后兼容。

#### 3. Transfer Module（文件传输模块）

位于 `pdfviewer/src/transfer/`，实现局域网文件传输功能（基于 LANShare 项目）：

**核心类层次结构:**

```
Transfer (抽象基类)
├── Sender (Push模式发送端)
├── Receiver (Push模式接收端)
├── PullSender (Pull模式发送端)
└── PullReceiver (Pull模式接收端)
```

**传输协议:**

数据包格式：`[包大小(4字节)] + [包类型(1字节)] + [包数据(N字节)]`

数据包类型（PacketType 枚举）:
- `Header (0x01)`: 文件头信息包（JSON 格式，包含文件名、大小等元数据）
- `Data (0x02)`: 文件数据包
- `Finish (0x03)`: 传输完成包
- `Cancel (0x04)`: 取消传输包
- `Pause (0x05)`: 暂停传输包
- `Resume (0x06)`: 恢复传输包
- `Command (0x10)`: 命令请求包（用于共享浏览）
- `Response (0x11)`: 命令响应包

**协议识别机制:**

TransferServer 使用 ProtocolIdentifier 读取第一个数据包，根据类型判断:
- `Header` 包 → Push 协议（普通文件传输）→ 创建 Receiver
- `Command` 包 → Pull 协议（共享浏览）→ 创建 PullSender

**关键类:**

- **DeviceBroadcaster**: UDP 广播发现局域网内的设备
- **TransferServer**: TCP 服务器，监听传输端口，接受连接
- **Sender/Receiver**: Push 模式（主动发送文件）
- **PullSender/PullReceiver**: Pull 模式（浏览共享并拉取文件）

#### 4. Share Module（共享文件管理）

位于 `pdfviewer/src/share/`，管理文件/文件夹共享：

- **SharedFileManager**: 单例模式，核心管理器
  - 数据库：SQLite (`shares.db`)，包含 shares、file_index、access_control、access_log 表
  - 功能：添加/删除/更新共享、文件索引、访问控制（密码、白名单/黑名单）、访问日志
  - 线程安全：使用 QReadWriteLock 保护共享数据
  - 定时更新：每 5 分钟自动更新文件索引

- **SharedItem**: 共享项数据结构（ID、路径、名称、类型、权限等）
- **ShareConfig**: 共享配置结构（密码、权限、描述等）

#### 5. Model Module（数据模型）

位于 `pdfviewer/src/model/`：

- **Device**: 设备信息（ID、名称、IP、操作系统）
- **DeviceListModel**: 设备列表模型（QAbstractListModel）
- **TransferInfo**: 传输信息（状态、进度、速度）
- **TransferTableModel**: 传输列表表格模型（QAbstractTableModel）

#### 6. UI Module（用户界面）

位于 `pdfviewer/src/ui/`，新架构的对话框：

- **mainwindow.cpp/h**: 新的主窗口实现（替代根目录的旧版本）
- **settingsdialog**: 设置对话框（传输端口、广播端口、设备名称）
- **sharemanagerdialog**: 共享管理对话框
- **browsesharedialog**: 浏览共享对话框
- **receiverselectordialog**: 接收者选择对话框
- **aboutdialog**: 关于对话框

### 多线程架构

- 使用 `QThreadPool` 管理线程池
- 任务类继承 `QRunnable` 并实现 `run()` 方法
- 通过 `QThreadPool::globalInstance()->start(task)` 提交任务
- 使用 Qt 信号槽机制进行线程间通信（自动跨线程）
- 共享数据使用 `QMutex` 或 `QReadWriteLock` 保护

**示例:**
```cpp
// 水印处理使用线程池
watermarkThreadSingle* task = new watermarkThreadSingle(...);
m_threadPool.start(task);

// SharedFileManager 使用读写锁
QReadLocker locker(&mLock);  // 读操作
QWriteLocker locker(&mLock); // 写操作
```

## 第三方库依赖

### 必需库（已包含在项目中）

- **Qt Framework 5.12+**
  - Qt Core, Gui, Widgets, PrintSupport, Svg
  - Qt PDFWidgets (Qt 5.14+)
  - Qt Network, Sql

- **MuPDF** - PDF 渲染引擎
  - 链接库：libmupdf, libpkcs7, libmuthreads, libthirdparty
  - 包含路径：`pdfviewer/lib/mupdf/include/`

- **PDFlib** - 高级 PDF 操作（商业库）
  - 文件：`pdfviewer/lib/pdflib.dll`, `pdflib.lib`
  - 注意：系统不提供商业许可证，需自行解决

- **libxml2** - XML/SVG 处理
  - 链接库：libxml2 (Release), libxml2d (Debug)

- **libiconv** - 字符编码转换
  - 链接库：libiconv

- **Qt5Xlsx** - Excel 文件处理
  - 链接库：Qt5Xlsx (Release), Qt5Xlsxd (Debug)
  - 包含路径：`pdfviewer/lib/qtxlsx/include/`

### 库链接配置

所有库都根据构建配置（Debug/Release）自动链接正确的版本，配置在 `pdfviewer/pdfviewer.pro` 中。

## 编码规范

- **字符编码**: 所有源文件使用 UTF-8 编码
  - `.pro` 文件中设置：`QMAKE_CXXFLAGS += /utf-8` 和 `CONFIG += utf8_source`
  - `main.cpp` 中使用：`#pragma execution_character_set("utf-8")`

- **命名约定**:
  - 类名：PascalCase（如 `WatermarkController`）
  - 函数名：camelCase（如 `addWatermark`）
  - 成员变量：m_ 前缀（如 `m_document`）
  - 私有成员：使用 `m` 前缀

- **Qt 编码风格**: 遵循 Qt 官方编码规范

## 重要约定

### 1. 控制器-UI 分离

新代码采用 MVC 模式，控制器位于 `src/controllers/`，UI 位于 `src/ui/`。旧的 `mainwindow.cpp/h` 在根目录已废弃。

### 2. 功能模块封装

所有核心算法封装在 `src/function/` 的命名空间中，通过 `function.h` 提供统一接口。新功能应继续遵循此模式。

### 3. 文件传输协议

修改传输协议时必须保持向后兼容，或更新协议版本号。数据包解析在 `Transfer::onReadyRead()` 中处理，支持粘包。

### 4. 共享文件管理

SharedFileManager 是单例，全局唯一。修改共享数据必须加锁。数据库操作应在主线程进行。

### 5. 路径安全

所有文件路径操作（特别是共享浏览）必须防止目录遍历攻击，使用 `QDir::cleanPath()` 和 `QString::contains("..")` 检查。

### 6. 资源清理

文件传输和共享操作涉及文件句柄和网络连接，确保在异常情况下正确释放资源（使用 RAII 或智能指针）。

## 常见任务

### 添加新的 PDF 处理功能

1. 在 `include/function/` 创建头文件，定义命名空间和函数
2. 在 `src/function/` 实现功能
3. 在 `function.h` 中添加全局函数别名
4. 在 `pdfviewer.pro` 的 HEADERS 和 SOURCES 中注册文件
5. 创建或更新相应的 Controller

### 修改传输协议

1. 更新 `src/transfer/transfer.h` 中的枚举定义
2. 在 `Transfer::processPacket()` 中添加新的分发逻辑
3. 在派生类中实现 `processXxxPacket()` 方法
4. 更新协议文档注释

### 添加新的共享功能

1. 修改 `SharedFileManager` 添加新方法
2. 如需数据库支持，在 `initDatabase()` 中添加表结构
3. 发射适当的信号通知 UI 更新
4. 在 `ShareManagerDialog` 中添加 UI 控件

## 调试和测试

### 调试配置

- 项目包含 4 个构建配置：Debug、Release、Profile、Debug2
- Debug 版本链接调试版库（如 libxml2d, Qt5Xlsxd）
- 使用 Qt Creator 的调试器或 Visual Studio 调试器

### 局域网功能测试

需要至少两台设备在同一局域网：
1. 在设置中配置不同的设备名称
2. 启动 DeviceBroadcaster 广播设备信息
3. 测试文件发送/接收功能
4. 测试共享浏览和拉取功能

### 单元测试

当前项目没有单元测试框架。建议为核心功能模块添加 Qt Test 框架测试。

## 当前分支信息

- **主分支**: `master`
- **当前分支**: `syncFile`（局域网传输和共享功能开发分支）
- 合并到主分支前，确保所有新功能经过充分测试，不影响原有 PDF 功能
