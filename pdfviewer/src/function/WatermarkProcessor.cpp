/**
 * @file WatermarkProcessor.cpp
 * @brief 水印处理模块实现
 * 
 * 该模块实现了PDF文档的水印处理功能，支持多行文本水印的添加，
 * 使用SVG生成技术和PDFlib库提供专业的水印效果。
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma execution_character_set("utf-8") // 设置编译器使用UTF-8字符集
#include "include/function/WatermarkProcessor.h"
#include "include/function/GeometryUtils.h" // 需要几何计算工具
#include "include/mark/mark.h" // 包含GetFontsFolder函数声明
#include <QFile>

namespace WatermarkProcessor {

/**
 * @brief 获取SVG文件的尺寸信息
 * 使用libxml2库解析SVG文件并提取宽度和高度属性
 * @param filename SVG文件路径
 * @param width 输出参数，SVG宽度
 * @param height 输出参数，SVG高度
 */
void getSVGDimensions(const char* filename, int& width, int& height) {
    xmlDocPtr doc;
    xmlNodePtr rootNode;

    // 解析SVG文件
    doc = xmlReadFile(filename, nullptr, 0);
    if (doc == nullptr) {
        std::cerr << "Error: Unable to parse SVG file." << std::endl;
        return;
    }

    // 获取根节点
    rootNode = xmlDocGetRootElement(doc);
    if (rootNode == nullptr || xmlStrcmp(rootNode->name, (const xmlChar*)"svg")) {
        std::cerr << "Error: Invalid SVG file format." << std::endl;
        xmlFreeDoc(doc);
        return;
    }

    // 从<svg>元素中提取width和height属性
    const xmlChar* widthStr = xmlGetProp(rootNode, (const xmlChar*)"width");
    const xmlChar* heightStr = xmlGetProp(rootNode, (const xmlChar*)"height");

    if (widthStr == nullptr || heightStr == nullptr) {
        std::cerr << "Error: SVG file missing width or height attribute."
                  << std::endl;
        xmlFreeDoc(doc);
        return;
    }

    // 将宽度和高度字符串转换为整数
    width = atoi((const char*)widthStr);
    height = atoi((const char*)heightStr);

    // 释放内存
    xmlFreeDoc(doc);
}

/**
 * @brief 为PDF文件添加多行文本水印
 * 支持自定义字体、字号、颜色、旋转角度和透明度
 * 使用SVG生成水印图形，然后作为图章添加到PDF页面
 * @param inFile 输入PDF文件路径
 * @param outFile 输出PDF文件路径
 * @param mark_txt 水印文本内容（支持换行符分隔的多行文本）
 * @param fontName 水印字体名称
 * @param fontSize 水印字体大小
 * @param color 水印颜色
 * @param angle 水印旋转角度
 * @param opacity 水印透明度（0.0-1.0）
 * @return 0表示成功，其他值表示失败
 */
int addWatermark_multiline(string inFile, string outFile, QString mark_txt,
                           QString fontName, int fontSize, QString color,
                           qreal angle, qreal opacity) {
    const wstring outfile = StringConverter::String2WString(outFile);
    wstring pdffile = StringConverter::String2WString(inFile);
    
    try {
        PDFlib p;
        // 设置错误处理策略为返回错误值
        p.set_option(L"errorpolicy=return");
        
        // 设置PDFlib搜索路径
        const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
        wostringstream optlist;
        optlist << L"searchpath={{" << searchpath << L"}";
        optlist << L" {" << GetFontsFolder() << L"}}";
        p.set_option(optlist.str());
        p.set_option(L"errorpolicy=return");

        // 创建输出PDF文档
        if (p.begin_document(outfile, L"") == 0) {
            qDebug() << L"Error: " << p.get_errmsg() << endl;
            return 2;
        }
        
        // 打开输入PDF文档
        int indoc = p.open_pdi_document(pdffile, L"");
        if (indoc != 0) {
            qDebug() << "error----------------indoc == 0";
        }

        int endpage = (int)p.pcos_get_number(indoc, L"length:pages");
        wstring graphicsfile = pdffile + L".svg";
        int w = 0, h = 0;
        
        // 设置字体和文本
        QFont font(fontName, fontSize);
        QStringList textLines = mark_txt.split("\n");
        
        // 创建一个QFontMetrics对象，用于获取文本尺寸
        QFontMetrics fontMetrics(font);
        // 获取字体的高度
        int lineHeight = fontMetrics.height();
        int maxWidth = 0;
        
        // 遍历文本各行，获取最长宽度
        foreach (const QString& line, textLines) {
            int lineWidth = GeometryUtils::getTextWidth(line, fontName, fontSize).cx;
            if (lineWidth > maxWidth) {
                maxWidth = lineWidth;
            }
        }

        int maxHeight = lineHeight * textLines.size();  // 单行高度 × 行数
        
        // 遍历PDF文档的所有页面，为每一页添加水印
        for (int pageno = 1; pageno <= endpage; pageno++) {
            int page = p.open_pdi_page(indoc, pageno, L"");  // 打开页面
            if (page != 0) {
                qDebug() << "error----------------page == 0";  // 页面打开失败
            }
            
            // 获取当前页面的尺寸信息
            int pageWidth =
                p.pcos_get_number(indoc,
                                  L"pages[" + StringConverter::String2WString(to_string(pageno - 1)) +
                                      L"]/width");   // 页面宽度（如A4为595点）
            int pageHeigth =
                p.pcos_get_number(indoc,
                                  L"pages[" + StringConverter::String2WString(to_string(pageno - 1)) +
                                      L"]/height");  // 页面高度（如A4为842点）

            // 创建水印矩形区域，位于页面中心
            GeometryUtils::Rectangle rect = {pageWidth / 2 + 50,           // X坐标：页面中心 + 补偿值
                                           pageHeigth / 2 + maxHeight / 2, // Y坐标：页面中心 + 文字高度的一半
                                           maxWidth,                       // 矩形宽度
                                           maxHeight};                     // 矩形高度
            
            double offsetX, offsetY;  // 旋转后的XY坐标补偿值
            GeometryUtils::rotateRectangle(rect, angle, offsetX, offsetY);  // 计算旋转补偿
            
            // 如果页面尺寸发生变化，重新生成SVG水印文件
            // 这样可以确保水印在不同尺寸的页面上都能正确显示
            if ((pageWidth != w) || (pageHeigth != h)) {
                GeometryUtils::createSVG(QString::fromStdString(inFile) + ".svg", mark_txt, pageWidth,
                          pageHeigth, fontName, fontSize, color, angle, opacity);
                w = pageWidth;   // 记录当前页面宽度
                h = pageHeigth;  // 记录当前页面高度
            }

            p.begin_page_ext(0, 0, L"width=a4.width height=a4.height");

            // 将导入的页面放置在输出页面上，并调整页面大小
            p.fit_pdi_page(page, 0, 0, L"adjustpage");
            
            // 创建用作注释图标的模板
            p.begin_template_ext(pageWidth, pageHeigth, L"topdown=true");
            
            // 加载SVG图形
            int gs = p.load_graphics(L"auto", graphicsfile, L"");
            
            // 将SVG放置在模板的指定位置
            p.fit_graphics(gs, pageWidth / 2 - offsetX / 2,
                           pageHeigth / 2 - offsetY / 2,
                           L" scale=1 position={center} "
                           L"fitmethod=auto showborder");
            p.close_graphics(gs);
            p.end_template_ext(0, 0);
            
            // 使用模板创建"正常"外观的注释
            p.create_annotation(0, 0, pageWidth, pageHeigth, L"Stamp",
                              L"template={normal=1} ");
            p.close_pdi_page(page);

            p.end_page_ext(L"");
        }

        p.end_document(L"");
        
        // 删除临时SVG文件
        string svg = inFile + ".svg";
        QFile::remove(QString::fromStdString(svg));
        
    } catch (PDFlib::Exception& ex) {
        wcerr << L"PDFlib exception occurred:" << endl
              << L"[" << ex.get_errnum() << L"] " << ex.get_apiname() << L": "
              << ex.get_errmsg() << endl;
        return 99;
    } catch (exception& e) {
        cerr << "C++ exception occurred: " << e.what() << endl;
        return 99;
    } catch (...) {
        cerr << "Generic C++ exception occurred!" << endl;
        return 99;
    }

    return 0;
}

} // namespace WatermarkProcessor
