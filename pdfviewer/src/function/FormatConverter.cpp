/**
 * @file FormatConverter.cpp
 * @brief 格式转换模块实现
 * 
 * 该模块实现了PDF与图片格式之间的相互转换功能，
 * 使用MuPDF和PDFlib库提供高质量的格式转换服务。
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma execution_character_set("utf-8") // 设置编译器使用UTF-8字符集
#include "include/function/FormatConverter.h"
#include "include/mark/mark.h" // 包含GetFontsFolder函数声明

namespace FormatConverter {

/**
 * @brief 将PDF文件转换为图片文件
 * 使用MuPDF库将PDF的每一页转换为PNG格式的图片
 * @param pdfFile PDF文件路径
 * @param imagePath 图片输出目录路径
 * @param resolution 输出图片的分辨率（DPI），默认72
 * @return 转换的页面数量，出错时返回1
 */
int pdf2image(string pdfFile, string imagePath, int resolution) {
    // 创建MuPDF上下文
    fz_context* ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    fz_register_document_handlers(ctx);  // 注册文档处理器
    
    // 打开PDF文档
    fz_document* doc = fz_open_document(ctx, pdfFile.c_str());
    if (!doc) {
        qDebug() << "文件处理错误：" << QString::fromStdString(pdfFile);
        return 1;
    }
    
    int num = fz_count_pages(ctx, doc);  // 获取PDF页面数
    float zoom = (float)resolution / (float)72;  // 计算缩放比例（基于72DPI）
    fz_matrix ctm = fz_scale(zoom, zoom);  // 创建变换矩阵

    // 逐页转换为图片
    for (int i = 0; i < num; i++) {
        string fileName = imagePath + "/" + to_string(i) + ".png";
        // 创建页面的像素图
        fz_pixmap* pix =
            fz_new_pixmap_from_page_number(ctx, doc, i, ctm, fz_device_rgb(ctx), 0);
        // 保存为PNG文件
        fz_save_pixmap_as_png(ctx, pix, fileName.c_str());
        fz_drop_pixmap(ctx, pix);  // 释放像素图内存
    }

    // 清理资源
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
    return num;  // 返回转换的页面数
}

/**
 * @brief 将单个图片文件转换为PDF文件
 * 使用PDFlib库创建包含一页图片的PDF文档
 * @param imageFile 输入图片文件路径
 * @param pdfFile 输出PDF文件路径
 * @return 0表示成功，2表示失败
 */
int image2pdf(std::string imageFile, std::string pdfFile) {
    PDFlib p;
    
    // 设置PDFlib搜索路径，包括字体映射文件和系统字体目录
    const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
    wostringstream optlist;
    optlist << L"searchpath={{" << searchpath << L"}";
    optlist << L" {" << GetFontsFolder() << L"}}";
    p.set_option(optlist.str());

    // 设置PDF文档信息
    p.set_info(L"Creator", L"泛生态业务工具集");
    p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
    
    int image;
    double imagewidth, imageheight;
    try {
        // 开始创建PDF文档
        if (p.begin_document(StringConverter::String2WString(pdfFile), L"") == -1) {
            wcerr << L"Error: " << p.get_errmsg() << endl;
            return 2;
        }
        
        // 加载图片并获取尺寸信息
        image = p.load_image(L"auto", StringConverter::String2WString(imageFile), L"");
        imagewidth = p.info_image(image, L"width", L"");   // 获取图片宽度
        imageheight = p.info_image(image, L"height", L""); // 获取图片高度
        
        if (image == 0) {
            // 图片加载失败的错误处理
        }

        // 创建与图片尺寸相同的PDF页面
        p.begin_page_ext(imagewidth, imageheight, L"");
        p.fit_image(image, 0, 0, L"");  // 将图片放置在页面左下角
        p.close_image(image);           // 关闭图片资源
        p.end_page_ext(L"");           // 结束页面

        p.end_document(L"");  // 结束文档创建
        return 0;
    } catch (PDFlib::Exception& ex) {
        // PDFlib异常处理
        wcerr << L"PDFlib 发生异常: " << endl
              << L"[" << ex.get_errnum() << L"] " << ex.get_apiname() << L": "
              << ex.get_errmsg() << endl
              << L": " << L"错误的参数选项请使用 - h参数查看帮助" << endl;
        return 2;
    }
}

/**
 * @brief 将图片列表合并到一个PDF文件
 * 每个图片占一页，页面大小自适应图片尺寸
 * @param images 图片文件路径列表
 * @param pdfFile 输出PDF文件路径
 * @return 0表示成功，2表示失败
 */
int images2pdf(QStringList& images, std::string pdfFile) {
    PDFlib p;
    const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
    wostringstream optlist;
    optlist << L"searchpath={{" << searchpath << L"}";
    optlist << L" {" << GetFontsFolder() << L"}}";
    p.set_option(optlist.str());
    
    // 设置PDF文档信息
    p.set_info(L"Creator", L"泛生态业务工具集");
    p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
    
    int image;
    double imagewidth, imageheight;
    
    try {
        if (p.begin_document(StringConverter::String2WString(pdfFile), L"") == -1) {
            wcerr << L"Error: " << p.get_errmsg() << endl;
            return 2;
        }
        
        // 遍历图片列表，为每个图片创建一页PDF
        for (const QString& file : images) {
            image = p.load_image(L"auto", StringConverter::QString2WString(file), L"");  // 加载图片
            imagewidth = p.info_image(image, L"width", L"");           // 获取图片宽度
            imageheight = p.info_image(image, L"height", L"");         // 获取图片高度
            
            if (image == 0) {
                wcerr << L"Error: " << p.get_errmsg();  // 图片加载失败
            }
            
            p.begin_page_ext(imagewidth, imageheight, L"");  // 创建与图片尺寸相同的页面
            p.fit_image(image, 0, 0, L"");                   // 将图片放置在页面上
            p.close_image(image);                            // 关闭图片资源
            std::remove(file.toUtf8().data());              // 删除原始图片文件
            p.end_page_ext(L"");                            // 结束页面
        }
        p.end_document(L"");
        return 0;
    } catch (PDFlib::Exception& ex) {
        wcerr << L"PDFlib 发生异常: " << endl
              << L"[" << ex.get_errnum() << L"] " << ex.get_apiname() << L": "
              << ex.get_errmsg() << endl
              << L": " << L"错误的参数选项请使用 - h参数查看帮助" << endl;
        return 2;
    }
}

/**
 * @brief 将指定目录中的图片文件合并为PDF
 * 特殊函数，图片文件名必须为0.png, 1.png, ..., (num-1).png格式
 * @param imagesDir 图片目录路径
 * @param pdfFile 输出PDF文件路径
 * @param num 图片文件数量
 * @return 0表示成功，2表示失败
 */
int images2pdf(std::string imagesDir, std::string pdfFile, int num) {
    PDFlib p;
    const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
    wostringstream optlist;
    optlist << L"searchpath={{" << searchpath << L"}";
    optlist << L" {" << GetFontsFolder() << L"}}";
    p.set_option(optlist.str());
    
    // 设置PDF文档信息
    p.set_info(L"Creator", L"泛生态业务工具集");
    p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
    
    int image;
    double imagewidth, imageheight;
    
    try {
        if (p.begin_document(StringConverter::String2WString(pdfFile), L"") == -1) {
            wcerr << L"Error: " << p.get_errmsg() << endl;
            return 2;
        }
        
        // 按照编号顺序处理图片文件
        for (int i = 0; i < num; i++) {
            // 载入图片文件
            image = p.load_image(
                L"auto", StringConverter::String2WString(imagesDir + "/" + to_string(i) + ".png"),
                L"");
            // 获取图片的宽、高
            imagewidth = p.info_image(image, L"width", L"");
            imageheight = p.info_image(image, L"height", L"");
            
            // 以图片的尺寸生成PDF页面
            p.begin_page_ext(imagewidth, imageheight, L"");
            p.fit_image(image, 0, 0, L"");
            p.close_image(image);
            
            // 删除处理过的图片文件
            std::remove((imagesDir + "/" + to_string(i) + ".png").c_str());
            p.end_page_ext(L"");
        }
        p.end_document(L"");
        return 0;
    } catch (PDFlib::Exception& ex) {
        wcerr << L"PDFlib 发生异常: " << endl
              << L"[" << ex.get_errnum() << L"] " << ex.get_apiname() << L": "
              << ex.get_errmsg() << endl
              << L": " << L"错误的参数选项请使用 - h参数查看帮助" << endl;
        return 2;
    }
}

} // namespace FormatConverter
