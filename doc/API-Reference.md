# PDF Widgets API 参考文档

## 目录
1. [文件检测模块 (FileDetector)](#filedetector)
2. [PDF操作模块 (PdfOperations)](#pdfoperations)  
3. [水印处理模块 (WatermarkProcessor)](#watermarkprocessor)
4. [格式转换模块 (FormatConverter)](#formatconverter)
5. [几何计算工具 (GeometryUtils)](#geometryutils)
6. [字符串转换工具 (StringConverter)](#stringconverter)
7. [文件系统工具 (FileSystemUtils)](#filesystemutils)
8. [多线程类参考](#threading)

---

## FileDetector

文件类型检测和验证模块，提供基于扩展名和文件头的检测功能。

### 函数列表

#### isPDFByExtension
```cpp
bool FileDetector::isPDFByExtension(const QString& fileName)
```
**功能描述**：通过文件扩展名判断是否为PDF文件

**参数**：
- `fileName` - 文件名或完整路径

**返回值**：
- `true` - 扩展名为.pdf（忽略大小写）
- `false` - 非PDF扩展名

**示例**：
```cpp
bool isPdf = FileDetector::isPDFByExtension("document.pdf"); // 返回 true
bool isNotPdf = FileDetector::isPDFByExtension("image.jpg"); // 返回 false
```

#### isPDFByMagicNumber
```cpp
bool FileDetector::isPDFByMagicNumber(const QString& filePath)
```
**功能描述**：通过文件头魔数判断是否为PDF文件

**参数**：
- `filePath` - 文件完整路径

**返回值**：
- `true` - 文件头包含"%PDF"标识
- `false` - 非PDF文件或读取失败

**注意事项**：
- 需要文件具有读取权限
- 比扩展名检测更可靠

#### isImageByExtension
```cpp
bool FileDetector::isImageByExtension(const QString& fileName)
```
**功能描述**：通过扩展名判断是否为图片文件

**支持的格式**：jpg, jpeg, png, bmp, gif, tiff, tif

**参数**：
- `fileName` - 文件名或完整路径

**返回值**：
- `true` - 支持的图片格式
- `false` - 非图片格式

#### isImageByMagicNumber
```cpp
bool FileDetector::isImageByMagicNumber(const QString& filePath)
```
**功能描述**：通过文件头魔数判断图片类型

**支持的检测**：
- JPEG：文件头 `FF D8 FF`
- PNG：文件头 `89 50 4E 47` (\x89PNG)
- BMP：文件头 `42 4D` (BM)

#### isNumeric
```cpp
bool FileDetector::isNumeric(const QString& str)
```
**功能描述**：验证字符串是否为纯数字

**参数**：
- `str` - 待验证字符串

**返回值**：
- `true` - 字符串全为数字字符
- `false` - 包含非数字字符

---

## PdfOperations

PDF文档核心操作模块，提供页面信息获取、拆分、合并等功能。

### 函数列表

#### getPages
```cpp
int PdfOperations::getPages(string pdfFile)
```
**功能描述**：获取PDF文档的页面数量

**参数**：
- `pdfFile` - PDF文件路径（std::string）

**返回值**：
- `>0` - PDF文档页面数
- `0` - 文件打开失败或非PDF文件

**实现细节**：使用MuPDF库进行页面计数

#### getPageheight
```cpp
int PdfOperations::getPageheight(wstring filename)
```
**功能描述**：获取PDF文档第一页的高度

**参数**：
- `filename` - PDF文件路径（std::wstring）

**返回值**：
- `>0` - 页面高度（点为单位）
- `0` - 获取失败

#### splitPdf (按页数拆分)
```cpp
int PdfOperations::splitPdf(string in, string out, int subpages)
```
**功能描述**：将PDF文档按指定页数平均拆分

**参数**：
- `in` - 输入PDF文件路径
- `out` - 输出目录路径
- `subpages` - 每个拆分文件包含的页数

**返回值**：
- `0` - 拆分成功
- `非0` - 错误代码

**输出文件命名**：`原文件名_1.pdf`, `原文件名_2.pdf`, ...

**示例**：
```cpp
// 将10页PDF按每3页拆分，产生4个文件（3,3,3,1页）
int result = PdfOperations::splitPdf("input.pdf", "output/", 3);
```

#### splitPdf (按范围拆分)
```cpp
int PdfOperations::splitPdf(string in, string out, int start, int end)
```
**功能描述**：提取PDF文档的指定页面范围

**参数**：
- `in` - 输入PDF文件路径
- `out` - 输出文件路径
- `start` - 起始页码（从1开始）
- `end` - 结束页码（包含）

**返回值**：
- `0` - 提取成功
- `非0` - 错误代码

#### mergePdf
```cpp
int PdfOperations::mergePdf(std::list<string> fileList, string outFile)
```
**功能描述**：合并多个PDF文件为一个

**参数**：
- `fileList` - PDF文件路径列表
- `outFile` - 输出文件路径

**返回值**：
- `0` - 合并成功
- `非0` - 错误代码

**特性**：
- 保持原有页面格式
- 自动为每个文件创建书签
- 支持大量文件合并

---

## WatermarkProcessor

PDF水印处理模块，支持多行文本水印和自定义样式。

### 函数列表

#### addWatermark_multiline
```cpp
int WatermarkProcessor::addWatermark_multiline(
    string inFile,      // 输入PDF文件
    string outFile,     // 输出PDF文件  
    QString mark_txt,   // 水印文本（支持\n分行）
    QString fontName,   // 字体名称
    int fontSize,       // 字体大小
    QString color,      // 颜色（#RRGGBB格式）
    qreal angle,        // 旋转角度（度）
    qreal opacity       // 透明度（0-100）
)
```
**功能描述**：为PDF添加多行文本水印

**参数详解**：
- `mark_txt` - 支持换行符(\n)分隔的多行文本
- `fontName` - 支持的字体：新宋体、楷体、仿宋、黑体
- `fontSize` - 字体大小，推荐范围8-50
- `color` - 十六进制颜色，如"#FF0000"（红色）
- `angle` - 旋转角度，0-180度
- `opacity` - 透明度百分比，0完全透明，100完全不透明

**返回值**：
- `0` - 水印添加成功
- `非0` - 错误代码

**实现原理**：
1. 将多行文本转换为SVG图形
2. 使用PDFlib将SVG嵌入PDF页面
3. 应用指定的样式参数

**示例**：
```cpp
int result = WatermarkProcessor::addWatermark_multiline(
    "input.pdf", 
    "output.pdf",
    "机密文档\n内部使用",
    "新宋体",
    25,
    "#FF0000",
    45.0,
    30.0
);
```

#### getSVGDimensions
```cpp
void WatermarkProcessor::getSVGDimensions(const char* filename, int& width, int& height)
```
**功能描述**：获取SVG文件的尺寸信息

**参数**：
- `filename` - SVG文件路径
- `width` - 输出宽度（引用）
- `height` - 输出高度（引用）

**用途**：用于水印定位计算

---

## FormatConverter

PDF与图片格式转换模块，支持双向转换和批量处理。

### 函数列表

#### pdf2image
```cpp
int FormatConverter::pdf2image(string pdfFile, string imagePath, int resolution = 72)
```
**功能描述**：将PDF文档转换为PNG图片

**参数**：
- `pdfFile` - PDF文件路径
- `imagePath` - 输出图片路径（不含扩展名）
- `resolution` - 分辨率DPI，默认72

**返回值**：
- `转换的页面数` - 成功
- `0` - 失败

**输出文件**：`imagePath_1.png`, `imagePath_2.png`, ...

**分辨率建议**：
- 72 DPI：屏幕显示
- 150 DPI：一般打印
- 300 DPI：高质量打印

#### image2pdf (单图片)
```cpp
int FormatConverter::image2pdf(std::string imageFile, std::string pdfFile)
```
**功能描述**：将单张图片转换为PDF

**参数**：
- `imageFile` - 图片文件路径
- `pdfFile` - 输出PDF路径

**返回值**：
- `0` - 转换成功
- `非0` - 错误代码

**特性**：
- 自动适配页面大小到图片尺寸
- 保持图片原始比例

#### images2pdf (图片列表)
```cpp
int FormatConverter::images2pdf(QStringList& images, std::string pdfFile)
```
**功能描述**：将多张图片合并为单个PDF

**参数**：
- `images` - 图片文件路径列表
- `pdfFile` - 输出PDF路径

**返回值**：
- `0` - 转换成功
- `非0` - 错误代码

#### images2pdf (目录批量)
```cpp
int FormatConverter::images2pdf(std::string imagesDir, std::string pdfFile, int num)
```
**功能描述**：将目录中的图片批量转换为PDF

**参数**：
- `imagesDir` - 图片目录路径
- `pdfFile` - 输出PDF路径
- `num` - 预期图片数量（用于验证）

**命名规则**：图片必须按`1.jpg`, `2.jpg`, ...`格式命名

---

## GeometryUtils

几何计算和SVG生成工具模块。

### 数据结构

#### Rectangle
```cpp
struct Rectangle {
    int x, y;          // 中心点坐标
    int width, height; // 矩形尺寸
};
```

### 函数列表

#### getTextWidth
```cpp
SIZE GeometryUtils::getTextWidth(QString text, QString fontName, int fontSize)
```
**功能描述**：测量文本在指定字体下的显示尺寸

**参数**：
- `text` - 文本内容
- `fontName` - 字体名称
- `fontSize` - 字体大小

**返回值**：Windows SIZE结构体
- `cx` - 文本宽度（像素）
- `cy` - 文本高度（像素）

#### toRadians
```cpp
double GeometryUtils::toRadians(double degrees)
```
**功能描述**：角度转弧度

**参数**：
- `degrees` - 角度值

**返回值**：对应的弧度值

#### rotateRectangle
```cpp
void GeometryUtils::rotateRectangle(Rectangle& rect, double angle, 
                                   double& offsetX, double& offsetY)
```
**功能描述**：计算矩形旋转后的位置偏移

**参数**：
- `rect` - 矩形对象（输入输出）
- `angle` - 旋转角度（弧度）
- `offsetX` - X轴偏移（输出）
- `offsetY` - Y轴偏移（输出）

#### createSVG
```cpp
void GeometryUtils::createSVG(QString svgName, QString text, int pageWidth, int pageHeight,
                              QString fontName, int fontSize, QString color, 
                              qreal angle, qreal opacity)
```
**功能描述**：创建水印用的SVG文件

**参数**：
- `svgName` - SVG文件输出路径
- `text` - 文本内容（支持多行）
- `pageWidth` - 页面宽度
- `pageHeight` - 页面高度
- `fontName` - 字体名称
- `fontSize` - 字体大小
- `color` - 文本颜色
- `angle` - 旋转角度
- `opacity` - 透明度

**生成的SVG特性**：
- 自动居中定位
- 支持多行文本布局
- 应用所有样式参数

---

## StringConverter

字符串编码转换工具模块。

### 函数列表

#### String2WString
```cpp
std::wstring StringConverter::String2WString(const string& str)
```
**功能描述**：将std::string转换为std::wstring

**参数**：
- `str` - 输入字符串（UTF-8编码）

**返回值**：对应的宽字符串（UTF-16）

#### QString2WString
```cpp
std::wstring StringConverter::QString2WString(const QString& str)
```
**功能描述**：将QString转换为std::wstring

**参数**：
- `str` - Qt字符串对象

**返回值**：对应的宽字符串

**用途**：为PDFlib等需要宽字符的库提供适配

---

## FileSystemUtils

文件系统操作工具模块。

### 函数列表

#### traverseDirectory
```cpp
void FileSystemUtils::traverseDirectory(const QDir& dir, QStringList& resultList,
                                        QString fileType, QString exclude)
```
**功能描述**：遍历目录查找指定类型的文件

**参数**：
- `dir` - 目录对象
- `resultList` - 结果列表（输出）
- `fileType` - 文件类型过滤器（如"*.pdf"）
- `exclude` - 排除的文件名模式

#### pathChange
```cpp
QString FileSystemUtils::pathChange(QString rootInput, QString rootOutput, 
                                   QString file, QString insert)
```
**功能描述**：生成对应的输出文件路径

**参数**：
- `rootInput` - 输入根目录
- `rootOutput` - 输出根目录
- `file` - 当前文件路径
- `insert` - 插入的字符串（如文件名后缀）

**返回值**：转换后的输出路径

#### createDirectory
```cpp
bool FileSystemUtils::createDirectory(const QString& path)
```
**功能描述**：创建目录（支持多级）

**参数**：
- `path` - 目录路径

**返回值**：
- `true` - 创建成功或已存在
- `false` - 创建失败

---

## Threading

多线程类参考，用于异步执行耗时操作。

### watermarkThreadSingle

单文件水印处理线程类。

#### 主要方法
```cpp
class watermarkThreadSingle : public QObject, public QRunnable {
public:
    // 参数设置
    void setInputFilename(QString input);
    void setOutputFilename(QString output);
    void setText(QString text);
    void setOpacity(QString opacity);
    void setColor(QString color);
    void setFont(QString font);
    void setRotate(QString rotate);
    void setFontsize(QString fontsize);
    
    // 执行接口
    void run() override;
    
signals:
    void addFinish(const wMap& map);  // 完成信号
};
```

#### 使用示例
```cpp
watermarkThreadSingle* thread = new watermarkThreadSingle();
thread->setInputFilename("input.pdf");
thread->setOutputFilename("output.pdf");
thread->setText("水印文本");
thread->setColor("#FF0000");
thread->setFont("新宋体");

// 连接完成信号
connect(thread, &watermarkThreadSingle::addFinish, 
        this, &MainWindow::onWatermarkFinished);

// 提交到线程池
QThreadPool::globalInstance()->start(thread);
```

### pdf2imageThreadSingle

PDF转图片处理线程类。

#### 主要方法
```cpp
class pdf2imageThreadSingle : public QObject, public QRunnable {
public:
    void setSourceFile(QString sourceFile);
    void setImagePath(QString imagePath);
    void setTargetFile(QString targetFile);
    void setResolution(int resolution);
    void setIs2pdf(bool is2pdf);
    
    void run() override;
    
signals:
    void pdf2imageFinish(const QString& message);
};
```

---

## 错误代码参考

### 通用错误代码
- `0` - 操作成功
- `-1` - 文件不存在
- `-2` - 文件读取失败
- `-3` - 文件写入失败
- `-4` - 参数无效
- `-5` - 内存分配失败

### PDF操作错误代码
- `100` - PDF文档损坏
- `101` - 页面索引超出范围
- `102` - 不支持的PDF版本
- `103` - PDF文档加密受保护

### 图片转换错误代码
- `200` - 不支持的图片格式
- `201` - 图片文件损坏
- `202` - 分辨率设置无效

---

## 使用注意事项

1. **字符编码**：所有字符串参数请使用UTF-8编码
2. **文件路径**：建议使用绝对路径，避免相对路径问题
3. **线程安全**：多线程类已实现线程安全，可安全并发使用
4. **资源管理**：大文件操作后建议及时清理临时文件
5. **错误处理**：务必检查函数返回值，进行适当的错误处理

---

## 版本信息

- **API版本**：1.0
- **最后更新**：2024年
- **兼容性**：Qt 5.12+, Windows 7+