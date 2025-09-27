/**
 * @file PdfOperations.cpp
 * @brief PDF文档操作模块实现
 * 
 * 该模块实现了PDF文档的各种基础操作功能，
 * 使用MuPDF和PDFlib库提供专业的PDF处理服务。
 * 
 * @author Qt PDF工具集项目组
 * @date 2023
 */

#pragma execution_character_set("utf-8") // 设置编译器使用UTF-8字符集
#include "include/function/PdfOperations.h"
#include "include/mark/mark.h" // 包含GetFontsFolder函数声明

namespace PdfOperations {

/**
 * @brief 获取PDF文件的页面数量
 * 使用MuPDF库解析PDF文件并返回页面数
 * @param pdfFile PDF文件路径
 * @return PDF文件的页面数，出错时返回1
 */
int getPages(string pdfFile) {
    // 创建MuPDF上下文和文档处理器
    fz_context* ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    fz_register_document_handlers(ctx);
    
    // 打开PDF文档
    fz_document* doc = fz_open_document(ctx, pdfFile.c_str());
    if (!doc) {
        qDebug() << "文件处理错误：" << QString::fromStdString(pdfFile);
        fz_drop_context(ctx);
        return 1;  // 文件打开失败
    }
    
    int num = fz_count_pages(ctx, doc);  // 获取页面数
    
    // 清理资源
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
    return num;
}

/**
 * @brief 获取PDF文件页面高度
 * 使用PDFlib获取PDF第一页的高度信息
 * @param filename PDF文件路径（宽字符串）
 * @return 页面高度（点为单位），出错时返回0
 */
int getPageheight(wstring filename) {
    PDFlib p;
    try {
        // 打开源PDF文件获取实例id
        int doc = p.open_pdi_document(filename, L"");
        if (doc == -1) {
            cout << "打开源pdf文件失败!" << endl;
            return 0;
        }
        // 获取第一页的高度信息
        auto fHeight = p.pcos_get_number(doc, L"pages[0]/height");  // 高度842（A4纸张）

        p.close_pdi_document(doc);
        return fHeight;
    } catch (PDFlib::Exception& e) {
        // 输出PDFlib异常信息
        wprintf(p.get_errmsg().c_str());
        return 0;
    } catch (...) {
        // 其他异常
        return 0;
    }
}

/**
 * @brief 将PDF拆分为多个文件，每个文件包含指定页数
 * @param in 输入PDF文件路径
 * @param out 输出文件名前缀
 * @param subpages 每个拆分文件的页数
 * @return 0表示成功，2表示失败
 */
int splitPdf(string in, string out, int subpages) {
    int indoc, page_count;
    wstring infile = StringConverter::String2WString(in);
    wstring outfile_basename = StringConverter::String2WString(out);
    
    try {
        PDFlib p;
        const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
        wostringstream optlist;
        optlist << L"searchpath={{" << searchpath << L"}";
        optlist << L" {" << GetFontsFolder() << L"}}";
        p.set_option(optlist.str());
        
        // 设置PDF文档信息
        p.set_info(L"Creator", L"泛生态业务工具集");
        p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
        
        // 打开输入PDF文档
        indoc = p.open_pdi_document(infile, L"");
        if (indoc == -1) {
            wcerr << L"打开文档错误: " << p.get_errmsg() << endl;
        }
        
        page_count = (int)p.pcos_get_number(indoc, L"length:pages");
        // 计算需要拆分成的文件数
        int outdoc_count = page_count / subpages + (page_count % subpages > 0 ? 1 : 0);
        
        // 逐个创建拆分的PDF文件
        for (int outdoc_counter = 0, page = 0; outdoc_counter < outdoc_count; outdoc_counter++) {
            std::wstringstream s1, s0;
            s0 << outdoc_counter;
            s1 << outdoc_counter + 1;
            wstring outfile = outfile_basename + L"_" + s1.str() + L".pdf";

            // 打开新的子文档
            if (p.begin_document(outfile, L"") == -1) {
            }
            p.set_info(L"Creator", L"泛生态业务工具集");
            p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
            p.set_info(L"Subject", L"Sub-document " + s1.str() + L" of " + s0.str() +
                                       L" of input document '" + infile + L"'");
            
            // 复制指定数量的页面到子文档
            for (int i = 0; page < page_count && i < subpages; page++, i++) {
                // 页面大小可能会被fit_pdi_page()调整
                p.begin_page_ext(0, 0, L"width=a4.width height=a4.height");
                int pagehdl = p.open_pdi_page(indoc, page + 1, L"");
                if (pagehdl == -1) {
                }
                // 将导入的页面放置在输出页面上，并调整页面大小
                p.fit_pdi_page(pagehdl, 0, 0, L"adjustpage");
                p.close_pdi_page(pagehdl);
                p.end_page_ext(L"");
            }
            
            // 关闭子文档
            p.end_document(L"");
        }
        p.close_pdi_document(indoc);

    } catch (PDFlib::Exception& ex) {
        wcerr << L"PDFlib 发生异常: " << endl
              << L"[" << ex.get_errnum() << L"] " << ex.get_apiname() << L": "
              << ex.get_errmsg() << endl
              << L": " << L"错误的参数选项请使用 - h参数查看帮助" << endl;
        return 2;
    }
    return 0;
}

/**
 * @brief 从PDF总提取start页到end页，为一个新的PDF文件
 * @param in 输入文件路径
 * @param out 输出文件路径前缀
 * @param start 截取的起始页（从0开始）
 * @param end 截取的终止页（不包含此页）
 * @return 0表示成功，2表示失败
 */
int splitPdf(string in, string out, int start, int end) {
    int indoc;
    wstring infile = StringConverter::String2WString(in);
    wstring outfile_basename = StringConverter::String2WString(out);
    
    try {
        PDFlib p;
        const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
        wostringstream optlist;
        optlist << L"searchpath={{" << searchpath << L"}";
        optlist << L" {" << GetFontsFolder() << L"}}";
        p.set_option(optlist.str());
        
        // 设置PDF文档信息
        p.set_info(L"Creator", L"泛生态业务工具集");
        p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
        
        // 打开输入PDF文档
        indoc = p.open_pdi_document(infile, L"");
        if (indoc == -1) {
            wcerr << L"打开文档错误: " << p.get_errmsg() << endl;
        }
        
        wstring outfile = outfile_basename + L"_split.pdf";
        
        // 打开子文档
        if (p.begin_document(outfile, L"") == -1) {
        }
        p.set_info(L"Creator", L"泛生态业务工具集");
        p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
        
        // 复制指定范围的页面
        for (int page = start; page < end; page++) {
            // 页面大小可能会被fit_pdi_page()调整
            p.begin_page_ext(0, 0, L"width=a4.width height=a4.height");
            int pagehdl = p.open_pdi_page(indoc, page + 1, L"");
            if (pagehdl == -1) {
            }
            // 将导入的页面放置在输出页面上，并调整页面大小
            p.fit_pdi_page(pagehdl, 0, 0, L"adjustpage");
            p.close_pdi_page(pagehdl);
            p.end_page_ext(L"");
        }
        
        // 关闭子文档
        p.end_document(L"");
        p.close_pdi_document(indoc);

    } catch (PDFlib::Exception& ex) {
        wcerr << L"PDFlib 发生异常: " << endl
              << L"[" << ex.get_errnum() << L"] " << ex.get_apiname() << L": "
              << ex.get_errmsg() << endl
              << L": " << L"错误的参数选项请使用 - h参数查看帮助" << endl;
        return 2;
    }
    return 0;
}

/**
 * @brief 将文件列表fileList中的文件合并成一个PDF文件
 * @param fileList 文件路径列表
 * @param outFile 合并后的输出文件路径
 * @return 0表示成功，2表示失败
 */
int mergePdf(std::list<std::string> fileList, string outFile) {
    wstring outfile = StringConverter::String2WString(outFile);
    PDFlib p;

    try {
        const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
        wostringstream optlist;
        optlist << L"searchpath={{" << searchpath << L"}";
        optlist << L" {" << GetFontsFolder() << L"}}";
        p.set_option(optlist.str());
        
        // 设置PDF文档信息
        p.set_info(L"Creator", L"泛生态业务工具集");
        p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
        
        if (p.begin_document(outfile, L"") == -1) {
            wcerr << L"Error: " + p.get_errmsg();
        }

        // 遍历文件列表，逐个合并PDF文件
        for (const std::string& file : fileList) {
            wstring wfile = StringConverter::String2WString(file);

            int indoc, endpage, pageno, page;
            // 打开输入PDF文件
            indoc = p.open_pdi_document(wfile, L"");
            if (indoc == -1) {
                wcerr << L"Error: " + p.get_errmsg();
                continue;
            }

            endpage = (int)p.pcos_get_number(indoc, L"length:pages");

            // 循环处理输入文档的所有页面
            for (pageno = 1; pageno <= endpage; pageno++) {
                page = p.open_pdi_page(indoc, pageno, L"");

                if (page == -1) {
                    wcerr << L"Error: " + p.get_errmsg();
                    continue;
                }
                // 页面大小可能会被fit_pdi_page()调整
                p.begin_page_ext(0, 0, L"width=a4.width height=a4.height");

                // 为文件名创建书签
                if (pageno == 1) p.create_bookmark(wfile, L"");

                // 将导入的页面放置在输出页面上，并调整页面大小
                // 如果页面包含注释，这些注释也会被导入
                p.fit_pdi_page(page, 0, 0, L"adjustpage");
                p.close_pdi_page(page);

                p.end_page_ext(L"");
            }
            p.close_pdi_document(indoc);
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

} // namespace PdfOperations
