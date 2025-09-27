/**
 * @file wmark.cpp
 * @brief PDF水印处理核心实现文件
 * @author PDF工具集项目组
 * @date 2024
 * 
 * 本文件实现了PDF文档水印添加的核心功能，基于PDFlib库提供
 * 完整的PDF水印处理解决方案。支持单页和多页水印添加、
 * 字体设置、颜色控制、旋转角度和透明度调整等功能。
 * 
 * 主要功能模块：
 * - 系统路径获取（字体文件夹、临时文件夹）
 * - 字符串转换工具（QString到wstring等）
 * - PDF页数获取和页面信息读取
 * - 单页水印添加功能
 * - 多页批量水印处理
 * - Qt框架集成接口
 * 
 * 依赖库：PDFlib、Windows API、Qt5框架
 */

//#pragma execution_character_set("utf-8")
//#include "../../stdafx.h"
#include <Windows.h>           // Windows API函数
#include <fcntl.h>             // 文件控制操作
#include <io.h>                // 输入输出操作
#include <sysinfoapi.h>        // 系统信息API

#include <codecvt>             // 字符编码转换
#include <iostream>            // 标准输入输出流
#include <locale>              // 本地化支持
#include <string>              // 字符串操作

#include "include/mark/mark.h"  // 水印处理头文件
#include "lib/pdflib.hpp"       // PDFlib库头文件

using namespace std;
using namespace pdflib;
#include <QDebug>              // Qt调试输出

#include "function.h"           // 通用功能函数
#include "lib/pdflib.hpp"       // PDFlib库头文件
//#include "E:/test/qt/win/QtWidgetsApplication1/lib/pdflib.hpp"
#pragma comment(lib, "E:/test/qt/win/QtWidgetsApplication1/lib/pdflib.lib")  // 链接PDFlib库

/*
std::wstring String2WString(const std::string& s)
{
        std::string strLocale = setlocale(LC_ALL, "");
        const char* chSrc = s.c_str();
        size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
        wchar_t* wchDest = new wchar_t[nDestSize];
        wmemset(wchDest, 0, nDestSize);
        mbstowcs(wchDest, chSrc, nDestSize);
        std::wstring wstrResult = wchDest;
        delete[] wchDest;
        setlocale(LC_ALL, strLocale.c_str());
        return wstrResult;
}
*/
/**
 * @brief 获取Windows系统字体文件夹路径
 * @return 系统字体文件夹的完整路径（宽字符串格式）
 * 
 * 通过Windows API获取系统目录，然后拼接fonts子目录路径。
 * 如果获取失败，返回默认的字体文件夹路径。
 */
std::wstring GetFontsFolder() {
  char buffer[MAX_PATH + 1];
  auto result = GetWindowsDirectoryA(buffer, MAX_PATH);  // 获取Windows系统目录
  if (result) {
    return String2WString(std::string(buffer).append("\\fonts"));  // 拼接fonts目录
  }
  return L"c:/windows/fonts";  // 获取失败时的默认路径
}
/**
 * @brief 获取Windows系统临时文件夹路径
 * @return 系统临时文件夹的完整路径
 * 
 * 通过Windows API获取系统目录，然后拼接temp子目录路径。
 * 如果获取失败，返回默认的临时文件夹路径。
 */
std::string GetTmpFolder() {
  char buffer[MAX_PATH + 1];
  auto result = GetWindowsDirectoryA(buffer, MAX_PATH);  // 获取Windows系统目录
  if (result) {
    return std::string(buffer).append("\\temp");  // 拼接temp目录
  }
  return "c:/windows/temp";  // 获取失败时的默认路径
}

/**
 * @brief 从完整路径中提取文件名
 * @param path 完整的文件路径
 * @return 提取出的文件名（不包含路径）
 * 
 * 查找路径中最后一个反斜杠的位置，提取其后的文件名部分。
 */
std::string GetFilename(string path) {
  string::size_type iPos = path.find_last_of('\\') + 1;  // 找到最后一个反斜杠位置
  string filename = path.substr(iPos, path.length() - iPos);  // 提取文件名
  return filename;
}
/**
 * @brief 将C风格字符数组转换为标准字符串
 * @param c C风格字符数组指针
 * @return 转换后的标准字符串对象
 * 
 * 简单的字符数组到字符串的转换函数。
 */
std::string char2string(char* c) {
  string str = c;  // 隐式转换为string对象
  return str;
}

/*
int main(int argc, char* argv[]) {
        GetOpt_pp ops(argc, argv);
        std::string infile = ops.getopt<std::string>('i', "input", "in.pdf");
        std::string outfile = ops.getopt<std::string>('o', "output", "out.pdf");
        wstring mark_txt = String2WString(ops.getopt<std::string>('t', "text",
"联通数字科技有限公司总部投标专用文档")); wstring mark_opacity =
String2WString(ops.getopt<std::string>('p', "opacity", "15%")); wstring
mark_color = String2WString(ops.getopt<std::string>('c', "color", "gray"));
        wstring mark_rotate = String2WString(ops.getopt<std::string>('r',
"rotate", "40")); wstring mark_font =
String2WString(ops.getopt<std::string>('f', "font", "simkai")); wstring
mark_number =String2WString(ops.getopt<std::string>('n', "number", "1"));
//水印数量，1或者3 wstring file_pages
=String2WString(ops.getopt<std::string>('s', "pages", "")); // 文件页数 wstring
file_pagesize =String2WString(ops.getopt<std::string>('z', "size", "")); //
页面尺寸

        if (file_pages.size() > 1) {
      getpages(file_pages,"pages");
                exit(0);

        }
        if (file_pagesize.size() > 1) {
          getpages(file_pagesize, "size");
          exit(0);
        }

        int numArg = 0;
        for (int i = 1; i < argc; ++i) {
                ++numArg;
        }
        //cout<<numArg<<endl;
        //获取输入文件
        if (numArg == 0) {
                usage();
                exit(0);
        }

        int r;
        // n=3 页面添加3条水印文本
        if (mark_number == L"3") {
          string tmpfile = GetTmpFolder() + "\\" + GetFilename(infile);
          r = setMark(infile, outfile, mark_txt, mark_opacity, mark_font,
                      mark_rotate, mark_color, L"0.6", L"top", L"-10");
          if (r == 0) {
            // DeleteFile(String2WString(infile).c_str());
            // rename(outfile.c_str(), infile.c_str());
            r = setMark(outfile, tmpfile, mark_txt, mark_opacity, mark_font,
                        mark_rotate, mark_color, L"0.6", L"center", L"10");
            if (r == 0) {
              r = setMark(tmpfile, outfile, mark_txt, mark_opacity, mark_font,
                          mark_rotate, mark_color, L"0.6", L"bottom", L"40");
              if (r == 0) {
                DeleteFile(String2WString(tmpfile).c_str());
              } else {
                return 2;
              }
            } else {
              return 2;
            }
          } else {
            return 2;
          }
        } else {
          r = setSingleMark(infile, outfile, mark_txt, mark_opacity, mark_font,
                            mark_rotate, mark_color);
        }

        return 0;
}
*/

/**
 * @brief 为PDF文件添加高级多页水印（支持位置和缩放控制）
 * @param infile 输入PDF文件路径
 * @param outfile 输出PDF文件路径
 * @param mark_txt 水印文本内容
 * @param mark_opacity 水印透明度（如"15%"）
 * @param mark_font 水印字体名称
 * @param mark_rotate 水印旋转角度
 * @param mark_color 水印颜色
 * @param scale 水印缩放比例
 * @param vertalign 垂直对齐方式（top/center/bottom）
 * @param vertshift 垂直偏移量
 * @return 处理结果，0表示成功，2表示失败
 * 
 * 使用PDFlib库为PDF文档的每一页添加水印，支持高级参数控制，
 * 包括位置对齐、缩放比例和偏移量等。
 */
int setMark(std::string infile, std::string outfile, wstring mark_txt,
            wstring mark_opacity, wstring mark_font, wstring mark_rotate,
            wstring mark_color, wstring scale, wstring vertalign,
            wstring vertshift) {
  // wcout << mark_color << endl;
  PDFlib p;  // 创建PDFlib实例
  int indoc, endpage, pageno, page, font;  // PDF处理相关变量
  
  // 设置字符映射和字体搜索路径
  const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
  wostringstream optlist;
  optlist << L"searchpath={{" << searchpath << L"}";
  optlist << L" {" << GetFontsFolder() << L"}}";
  p.set_option(optlist.str());  // 应用搜索路径设置
  // wcout << optlist.str() << endl;

  p.set_info(L"Creator", L"泛生态业务工具集");
  p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
  indoc = p.open_pdi_document(String2WString(infile), L"");
  // font = p.load_font(L"Helvetica", L"winansi", L"unicode");
  // 设置字体、字符集
  p.set_option(L"FontOutline={" + mark_font + L"=" + mark_font + L".ttf}");
  font = p.load_font(mark_font, L"unicode", L"");
  if (font == -1) {
    wcerr << L"Error: " << p.get_errmsg() << endl;
    return 2;
  }
  endpage = (int)p.pcos_get_number(indoc, L"length:pages");

  try {
    if (p.begin_document(String2WString(outfile), L"") == -1) {
      wcerr << L"Error: " << p.get_errmsg() << endl;
      return 2;
    }
    p.begin_template_ext(10, 10,
                         L"watermark={location=ontop opacity=" + mark_opacity +
                             L"  horizalign=right horizshift=-200 scale=" +
                             scale + L" vertalign=" + vertalign +
                             L" vertshift=" + vertshift + L"}  topdown=true");
    p.fit_textline(mark_txt, 800, 200,
                   L"fontsize=10 fontname=" + mark_font +
                       L" encoding=unicode fillcolor=" + mark_color +
                       L" boxsize={95 42} rotate=" + mark_rotate);
    p.end_template_ext(0, 0);

    for (pageno = 1; pageno <= endpage; pageno++) {
      page = p.open_pdi_page(indoc, pageno, L"");

      p.begin_page_ext(0, 0, L"width=a4.width height=a4.height");

      p.fit_pdi_page(page, 0, 0, L"adjustpage");

      p.close_pdi_page(page);

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

/**
 * @brief 为PDF文件添加简单单页水印
 * @param in 输入PDF文件路径（标准字符串格式）
 * @param out 输出PDF文件路径（标准字符串格式）
 * @param mark_txt 水印文本内容
 * @param mark_opacity 水印透明度（如"15%"）
 * @param mark_font 水印字体名称
 * @param mark_rotate 水印旋转角度
 * @param mark_color 水印颜色
 * @return 处理结果，0表示成功，2表示失败
 * 
 * 使用PDFlib库为PDF文档的每一页添加基本水印，相比setMark函数
 * 参数较少，适用于简单的水印添加需求。
 */
int setSingleMark(string in, string out, wstring mark_txt, wstring mark_opacity,
                  wstring mark_font, wstring mark_rotate, wstring mark_color) {
  int indoc, endpage, pageno, page, font;  // PDF处理相关变量
  wstring infile = String2WString(in);     // 转换输入文件路径为宽字符串
  wstring outfile = String2WString(out);   // 转换输出文件路径为宽字符串
  try {
    PDFlib p;
    const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
    wostringstream optlist;
    optlist << L"searchpath={{" << searchpath << L"}";
    optlist << L" {" << GetFontsFolder() << L"}}";
    p.set_option(optlist.str());
    // wcout << optlist.str() << endl;
    if (p.begin_document(outfile, L"") == -1) {
      wcerr << L"Error: " << p.get_errmsg() << endl;
      return 2;
    }
    p.set_info(L"Creator", L"泛生态业务工具集");
    p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");

    indoc = p.open_pdi_document(infile, L"");
    // font = p.load_font(L"Helvetica", L"winansi", L"unicode");
    // 设置字体、字符集
    p.set_option(L"FontOutline={" + mark_font + L"=" + mark_font + L".ttf}");
    font = p.load_font(mark_font, L"unicode", L"");

    if (font == -1) {
      wcerr << L"Error: " << p.get_errmsg() << endl;
      return 2;
    }
    // wcout << L"infile:" << infile << endl;
    endpage = (int)p.pcos_get_number(indoc, L"length:pages");
    // wcout << L"mark_opacity:" << mark_opacity << endl;

    p.begin_template_ext(
        0, 0, L"watermark={location=ontop opacity=" + mark_opacity + L"}");

    // p.fit_textline(L"继续与设计院、业主重点合作伙伴沟通，争取达成共赢合作模式",
    // 0, 0,L"fontsize=10 fontname=STSong-Light encoding=unicode  fillcolor=red
    // boxsize={595 842} rotate=15 stamp=ll2ur");
    // p.fit_textline(L"“十四五”武都引水灌区现代化改造工程信息化(“智慧武引”二期)系统",
    // 0, 0,L"fontsize=10 fontname=STSong-Light encoding=unicode  fillcolor=red
    // boxsize={195 142}  rotate=-45 "); wstring mark = L"fontsize=10 fontname="
    // + mark_font + L" encoding=unicode  fillcolor=" + mark_color + L"
    // boxsize={95 42}  rotate=" + mark_rotate;
    wstring mark = L"fontsize=10 fontname=" + mark_font +
                   L" encoding=unicode  fillcolor=" + mark_color +
                   L" boxsize={95 42}  rotate=" + mark_rotate;
    // wcout<< mark<<endl;
    p.fit_textline(mark_txt, 100, 100, mark);
    p.end_template_ext(0, 0);

    for (pageno = 1; pageno <= endpage; pageno++) {
      page = p.open_pdi_page(indoc, pageno, L"");

      p.begin_page_ext(0, 0, L"width=a4.width height=a4.height");

      p.fit_pdi_page(page, 0, 0, L"adjustpage");

      p.close_pdi_page(page);

      p.end_page_ext(L"");
    }
    p.end_document(L"");

  }

  catch (PDFlib::Exception& ex) {
    wcerr << L"PDFlib 发生异常: " << endl
          << L"[" << ex.get_errnum() << L"] " << ex.get_apiname() << L": "
          << ex.get_errmsg() << endl
          << L": " << L"错误的参数选项请使用 - h参数查看帮助" << endl;
    return 2;
  }
  return 0;
}

/**
 * @brief 获取PDF文件的总页数或页面尺寸信息
 * @param filename PDF文件路径（宽字符串格式）
 * @param flag 信息类型标志："pages"获取页数，其他值获取页面尺寸
 * @return PDF文件的总页数，失败时返回0
 * 
 * 使用PDFlib库打开PDF文档并获取文档信息。根据flag参数输出不同信息：
 * - "pages": 输出总页数
 * - 其他值: 输出第一页的宽度和高度（格式："宽度,高度"）
 */
int getpages(wstring filename, string flag) {
  PDFlib m_oPDF;  // 创建PDFlib实例
  try {
    // 打开源PDF文件获取文档ID
    // pageName = String2WString(char2string(optarg));
    int nSrcPDFID = m_oPDF.open_pdi_document(filename, L"");
    if (nSrcPDFID == -1) {
      cout << "打开源pdf文件失败!" << endl;
      return 0;
    }
    // m_oPDF.set_option(L"searchpath={{ print_cache }}");//设置默认路径

    // 根据文档ID获取PDF中的信息

    int endpage =
        (int)m_oPDF.pcos_get_number(nSrcPDFID, L"length:pages");  // 获取总页数
    auto fWidth =
        m_oPDF.pcos_get_number(nSrcPDFID, L"pages[0]/width");    // 获取第一页宽度（如595）
    auto fHeight =
        m_oPDF.pcos_get_number(nSrcPDFID, L"pages[0]/height");   // 获取第一页高度（如842）

    // printf("%d", endpage);

    // 根据标志输出不同类型的信息
    if (flag == "pages") {
      cout << endpage << endl;                    // 输出页数
    } else {
      cout << fWidth << "," << fHeight << endl;  // 输出页面尺寸（宽,高）
    }

    //		m_oPDF.close_font(font);
    m_oPDF.close_pdi_document(nSrcPDFID);  // 关闭PDF文档
    return endpage;
  } catch (PDFlib::Exception& e) {
    // 捕获并输出PDFlib异常信息
    wprintf(m_oPDF.get_errmsg().c_str());
    return 0;
  } catch (...) {
    // 捕获其他类型异常
    return 0;
  }
}

/**
 * @brief Qt框架集成的水印添加主接口函数
 * @param i 输入PDF文件路径
 * @param o 输出PDF文件路径
 * @param t 水印文本内容
 * @param p 水印透明度百分比（如"15%"）
 * @param c 水印颜色
 * @param r 水印旋转角度
 * @param f 水印字体名称
 * @param s 水印缩放比例（目前未使用）
 * @param va 垂直对齐方式（目前未使用）
 * @param vs 垂直偏移量（目前未使用）
 * @return 处理结果，0表示成功，2表示失败
 * 
 * 这是Qt应用程序的主要水印接口，将QString参数转换为wstring后
 * 调用PDFlib进行水印处理。支持特殊字体（如NSimSun）的处理。
 */
int addWatermark(const QString& i, const QString& o, const QString& t,
                 const QString& p, const QString& c, const QString& r,
                 const QString& f, const QString& s, const QString& va,
                 const QString& vs) {
  // 将Qt字符串参数转换为宽字符串
  wstring infile = QString2WString(i);        // 输入文件路径
  wstring outfile = QString2WString(o);       // 输出文件路径
  wstring mark_txt = QString2WString(t);      // 水印文本

  wstring mark_opacity = QString2WString(p);  // 透明度
  wstring mark_color = QString2WString(c);    // 颜色
  wstring mark_rotate = QString2WString(r);   // 旋转角度
  wstring mark_font = QString2WString(f);     // 字体
  wstring mark_scale = QString2WString(s);    // 缩放比例（暂未使用）
  wstring mark_vertalign = QString2WString(va);  // 垂直对齐（暂未使用）
  wstring mark_vertshift = QString2WString(vs);  // 垂直偏移（暂未使用）

  // wstring mark1 = L"fontsize=10 fontname=" + mark_font + L"
  // encoding=unicode fillcolor=" + mark_color + L" boxsize={95 42} rotate="
  // + mark_rotate; wcout
  // << mark_txt << endl; qDebug() << "mark_txt:" << mark_txt;//流输出方式
  // qDebug() << getpages(infile, "pages");

  int indoc, endpage, pageno, page, font;

  try {
    PDFlib p;
    const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
    wostringstream optlist;
    wstring mark;
    optlist << L"searchpath={{" << searchpath << L"}";
    optlist << L" {" << GetFontsFolder() << L"}}";
    p.set_option(optlist.str());
    if (p.begin_document(outfile, L"") == -1) {
      wcerr << L"Error: " << p.get_errmsg() << endl;
      return 2;
    }
    p.set_info(L"Creator", L"泛生态业务工具集");
    p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");
    indoc = p.open_pdi_document(infile, L"");
    // font = p.load_font(L"Helvetica", L"winansi", L"unicode");
    // 设置字体、字符集
    // p.set_option(L"FontOutline={" + mark_font + L"=" + mark_font + L".ttf}");
    // wcout << L"font:" << mark_font << endl;
    // font = p.load_font(mark_font, L"unicode", L"");
    if (mark_font == L"NSimSun") {
      p.set_option(L"FontOutline={NSimSun=simsun.ttc}");
      font = p.load_font(L"NSimSun", L"unicode", L"embedding");
    } else {
      p.set_option(L"FontOutline={" + mark_font + L"=" + mark_font + L".ttf}");
      font = p.load_font(mark_font, L"unicode", L"");
    }

    if (font == -1) {
      wcerr << L"Error: " << p.get_errmsg() << endl;
      return 2;
    }
    // wcout << L"infile:" << infile << endl;
    endpage = (int)p.pcos_get_number(indoc, L"length:pages");
    // wcout << L"mark_opacity:" << mark_opacity << endl;

    p.begin_template_ext(
        0, 0, L"watermark={location=ontop opacity=" + mark_opacity + L"}");
    /*
    p.begin_template_ext(0, 0,
                         L"watermark={location=ontop opacity=" + mark_opacity +
                             L"  horizalign=right horizshift=-200 scale=" +
                             mark_scale + L" vertalign=" + mark_vertalign +
                             L" vertshift=" + mark_vertshift +
                             L"}  topdown=true");
*/
    mark = L"fontsize=10 fontname=" + mark_font +
           L" encoding=unicode  fillcolor=" + mark_color +
           L" boxsize={95 42}  rotate=" + mark_rotate;
    // wcout<< mark<<endl;
    p.fit_textline(mark_txt, 100, 100, mark);
    p.end_template_ext(0, 0);

    for (pageno = 1; pageno <= endpage; pageno++) {
      page = p.open_pdi_page(indoc, pageno, L"");

      p.begin_page_ext(0, 0, L"width=a4.width height=a4.height");

      p.fit_pdi_page(page, 0, 0, L"adjustpage");

      p.close_pdi_page(page);

      p.end_page_ext(L"");
    }
    p.end_document(L"");
  }

  catch (PDFlib::Exception& ex) {
    wcerr << L"PDFlib 发生异常: " << endl
          << L"[" << ex.get_errnum() << L"] " << ex.get_apiname() << L": "
          << ex.get_errmsg() << endl
          << L": " << L"错误的参数选项请使用 - h参数查看帮助" << endl;
    return 2;
  }
  return 0;
}

/*

int main(int argc, char* argv[]){

        GetOpt_pp ops(argc, argv);


        wstring infile = String2WString(ops.getopt<std::string>('i', "input",
"in.pdf")); wstring outfile = String2WString(ops.getopt<std::string>('o',
"output", "out.pdf")); wstring mark_txt =
String2WString(ops.getopt<std::string>('t', "text",
"联通数字科技有限公司总部投标专用文档")); wstring mark_opacity =
String2WString(ops.getopt<std::string>('p', "opacity", "15%")); wstring
mark_color = String2WString(ops.getopt<std::string>('c', "color", "gray"));
        wstring mark_rotate = String2WString(ops.getopt<std::string>('r',
"rotate", "45")); wstring mark_font =
String2WString(ops.getopt<std::string>('f', "font", "simkai")); wstring
file_pages = String2WString(ops.getopt<std::string>('s', "pages", ""));

        //wstring mark1 = L"fontsize=10 fontname=" + mark_font + L"
encoding=unicode  fillcolor=" + mark_color + L" boxsize={95 42}  rotate=" +
mark_rotate;
        //wcout << mark1 << endl;

        if (file_pages.size()>1){
                getpages(file_pages);
                exit(0);

        }




        int indoc, endpage, pageno, page, font;

        int numArg = 0;
        for (int i = 1; i < argc; ++i) {
                ++numArg;
        }
        //cout<<numArg<<endl;
        //获取输入文件
        if (numArg == 0) {
                usage();
                exit(0);
        }

        try {

                PDFlib p;
                const wstring searchpath = L"./PDFlib-CMap-5.0/resource/cmap";
                wostringstream optlist;
                optlist << L"searchpath={{" << searchpath << L"}";
                optlist << L" {" << GetFontsFolder() << L"}}";
                p.set_option(optlist.str());
                //wcout << optlist.str() << endl;
                if (p.begin_document(outfile, L"") == -1) {
                        wcerr << L"Error: " << p.get_errmsg() << endl;
                        return 2;
                }
                p.set_info(L"Creator", L"泛生态业务工具集");
                p.set_info(L"Title", L"本文档来自于泛生态业务投标案例");


                indoc = p.open_pdi_document(infile, L"");
                //font = p.load_font(L"Helvetica", L"winansi", L"unicode");
                // 设置字体、字符集
                p.set_option(L"FontOutline={" + mark_font + L"=" + mark_font +
L".ttf}");

                //wcout << L"font:" << mark_font << endl;

                font = p.load_font(mark_font, L"unicode", L"");

                if (font == -1) {
                        wcerr << L"Error: " << p.get_errmsg() << endl;
                        return 2;
                }
                //wcout << L"infile:" << infile << endl;
                endpage = (int)p.pcos_get_number(indoc, L"length:pages");
                //wcout << L"mark_opacity:" << mark_opacity << endl;


                p.begin_template_ext(0, 0,
                        L"watermark={location=ontop opacity=" + mark_opacity +
L"}");


                //p.fit_textline(L"继续与设计院、业主重点合作伙伴沟通，争取达成共赢合作模式",
0, 0,L"fontsize=10 fontname=STSong-Light encoding=unicode  fillcolor=red
boxsize={595 842} rotate=15 stamp=ll2ur");
                //p.fit_textline(L"“十四五”武都引水灌区现代化改造工程信息化(“智慧武引”二期)系统",
0, 0,L"fontsize=10 fontname=STSong-Light encoding=unicode  fillcolor=red
boxsize={195 142}  rotate=-45 ");
                //wstring mark = L"fontsize=10 fontname=" + mark_font + L"
encoding=unicode  fillcolor=" + mark_color + L" boxsize={95 42}  rotate=" +
mark_rotate; wstring mark = L"fontsize=10 fontname=" + mark_font + L"
encoding=unicode  fillcolor=" + mark_color + L" boxsize={95 42}  rotate=" +
mark_rotate;
                //wcout<< mark<<endl;
                p.fit_textline(mark_txt, 100, 100, mark);
                p.end_template_ext(0, 0);




                for (pageno = 1; pageno <= endpage; pageno++)
                {
                        page = p.open_pdi_page(indoc, pageno, L"");

                        p.begin_page_ext(0, 0, L"width=a4.width
height=a4.height");

                        p.fit_pdi_page(page, 0, 0, L"adjustpage");

                        p.close_pdi_page(page);

                        p.end_page_ext(L"");
                }
                p.end_document(L"");

        }

        catch (PDFlib::Exception& ex) {

                wcerr << L"PDFlib 发生异常: " << endl
                        << L"[" << ex.get_errnum() << L"] " << ex.get_apiname()
                        << L": " << ex.get_errmsg() << endl
                        << L": " << L"错误的参数选项请使用 - h参数查看帮助" <<
endl; return 2;
        }



        return 0;

}

*/
