/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>
#include <include/mytable.h>
#include <include/textedit/Customtextedit.h>
#include "include/lineedit/CustomLineEdit.h"
#include "include/slider/CustomSlider.h"
#include "qpdfview.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionQuit;
    QAction *actionAbout;
    QAction *actionAbout_Qt;
    QAction *actionZoom_In;
    QAction *actionZoom_Out;
    QAction *actionPrevious_Page;
    QAction *actionNext_Page;
    QAction *actionContinuous;
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QWidget *widget;
    QPdfView *pdfView;
    QTabWidget *tabWidget;
    QWidget *tab;
    QPushButton *btnSelectOutput;
    QComboBox *cBoxFont;
    QLineEdit *lineEditRotate;
    QLabel *labInput;
    CustomLineEdit *lineEditColor;
    CustomLineEdit *lineEditOutput;
    QLabel *labOutput_4;
    QLineEdit *lineEditOpacity;
    QLabel *labOutput_3;
    QPushButton *btnSelectInput;
    QLabel *labOutput_2;
    QLabel *label;
    QPushButton *btnColorSelect;
    QPushButton *btnAddWater;
    QLabel *labWater;
    QLabel *labOutput;
    CustomLineEdit *lineEditInput;
    CustomSlider *sliderOpacity;
    CustomSlider *sliderRotate;
    QPushButton *btnExportPDF;
    QComboBox *cBoxResolution;
    CustomLineEdit *lineEdit_fs;
    CustomTextEdit *lineEditWaterText;
    QLabel *label_2;
    CustomSlider *sliderFontsize;
    QWidget *tab_transform;
    CustomLineEdit *lineEditImageFile;
    QLabel *label_i2p;
    QPushButton *btnSelectImageFile;
    QPushButton *btnTransform;
    QLabel *label_i2p_2;
    QLabel *label_i2p_3;
    QPushButton *btnTransformBat;
    QPushButton *btnSelectImageDir;
    QLabel *label_i2p_4;
    CustomLineEdit *lineEditImageDir;
    QLabel *label_i2p_5;
    CustomLineEdit *lineEditPdfFile;
    QPushButton *btnPdfToImage;
    QPushButton *btnSelectPDFFile;
    QLabel *label_i2p_6;
    QWidget *tab_Filesplit;
    CustomLineEdit *lineEditInputFilesplit;
    QPushButton *btnSelectFilesplit;
    QLabel *labInput_2;
    CustomLineEdit *lineEditSplitpages;
    QLabel *labInput_3;
    CustomLineEdit *lineEditSplitStart;
    QLabel *labInput_4;
    CustomLineEdit *lineEditSplitEnd;
    QLabel *labInput_5;
    QRadioButton *radioButtonSplitaverage;
    QRadioButton *radioButtonSpliterange;
    CustomLineEdit *lineEditSubPages;
    QLabel *labInput_6;
    CustomLineEdit *lineEditSplitscope;
    QPushButton *btnSplitPdf;
    QLabel *labInput_7;
    QPushButton *btnSelectSplitDir;
    QLabel *labInput_8;
    CustomLineEdit *lineEditSplitOutput;
    QWidget *tab_Filemerge;
    QPushButton *btnAddFile;
    mytable *tableWidgetMerge;
    CustomLineEdit *lineEditOutMerge;
    QLabel *labOutput_5;
    QPushButton *btnSelectMergeDir;
    QPushButton *btnMerge;
    QLabel *labWater_3;
    CustomLineEdit *lineEditMergeOutFile;
    QWidget *tab_2;
    QPushButton *btnSearch;
    QPushButton *btnSelectInput_Search;
    CustomLineEdit *lineEditInput_Search;
    QLabel *labWater_2;
    QTreeWidget *treeWidget_Search;
    CustomLineEdit *lineEditInput_Search_Key;
    QLabel *labWater_4;
    QPushButton *btnSearch_export;
    QTextEdit *textEditLog;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QMenu *menuView;
    QToolBar *mainToolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->setWindowModality(Qt::NonModal);
        MainWindow->resize(818, 796);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f0f6ff, stop:1 #e8f2ff);"));
        MainWindow->setUnifiedTitleAndToolBarOnMac(true);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/images/fileopen.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(icon);
        actionQuit = new QAction(MainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionAbout_Qt = new QAction(MainWindow);
        actionAbout_Qt->setObjectName(QString::fromUtf8("actionAbout_Qt"));
        actionZoom_In = new QAction(MainWindow);
        actionZoom_In->setObjectName(QString::fromUtf8("actionZoom_In"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/images/zoom-in-24.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_In->setIcon(icon1);
        actionZoom_Out = new QAction(MainWindow);
        actionZoom_Out->setObjectName(QString::fromUtf8("actionZoom_Out"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/images/zoom-out-24.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_Out->setIcon(icon2);
        actionPrevious_Page = new QAction(MainWindow);
        actionPrevious_Page->setObjectName(QString::fromUtf8("actionPrevious_Page"));
        actionNext_Page = new QAction(MainWindow);
        actionNext_Page->setObjectName(QString::fromUtf8("actionNext_Page"));
        actionContinuous = new QAction(MainWindow);
        actionContinuous->setObjectName(QString::fromUtf8("actionContinuous"));
        actionContinuous->setCheckable(true);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setEnabled(true);
        pdfView = new QPdfView(widget);
        pdfView->setObjectName(QString::fromUtf8("pdfView"));
        pdfView->setGeometry(QRect(453, 9, 361, 508));
        pdfView->setAutoFillBackground(false);
        pdfView->setStyleSheet(QString::fromUtf8(""));
        tabWidget = new QTabWidget(widget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(0, 0, 451, 519));
        tabWidget->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #f8fbff);\n"
"border-radius: 3px;"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        tab->setStyleSheet(QString::fromUtf8(""));
        btnSelectOutput = new QPushButton(tab);
        btnSelectOutput->setObjectName(QString::fromUtf8("btnSelectOutput"));
        btnSelectOutput->setGeometry(QRect(353, 178, 71, 31));
        btnSelectOutput->setStyleSheet(QString::fromUtf8(""));
        cBoxFont = new QComboBox(tab);
        cBoxFont->addItem(QString());
        cBoxFont->addItem(QString());
        cBoxFont->addItem(QString());
        cBoxFont->addItem(QString());
        cBoxFont->setObjectName(QString::fromUtf8("cBoxFont"));
        cBoxFont->setGeometry(QRect(200, 224, 80, 25));
        cBoxFont->setLayoutDirection(Qt::LeftToRight);
        cBoxFont->setStyleSheet(QString::fromUtf8("color: rgb(107, 107, 107);\n"
"font: 10pt \"\347\255\211\347\272\277\";\n"
"border: 1px solid #b8d4f0;"));
        lineEditRotate = new QLineEdit(tab);
        lineEditRotate->setObjectName(QString::fromUtf8("lineEditRotate"));
        lineEditRotate->setEnabled(false);
        lineEditRotate->setGeometry(QRect(370, 292, 61, 20));
        lineEditRotate->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        lineEditRotate->setReadOnly(true);
        labInput = new QLabel(tab);
        labInput->setObjectName(QString::fromUtf8("labInput"));
        labInput->setGeometry(QRect(10, 130, 81, 20));
        QFont font;
        font.setFamily(QString::fromUtf8("\347\255\211\347\272\277"));
        font.setPointSize(10);
        font.setBold(false);
        font.setItalic(false);
        font.setWeight(50);
        labInput->setFont(font);
        labInput->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        lineEditColor = new CustomLineEdit(tab);
        lineEditColor->setObjectName(QString::fromUtf8("lineEditColor"));
        lineEditColor->setGeometry(QRect(284, 224, 70, 25));
        lineEditColor->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        lineEditColor->setReadOnly(true);
        lineEditOutput = new CustomLineEdit(tab);
        lineEditOutput->setObjectName(QString::fromUtf8("lineEditOutput"));
        lineEditOutput->setGeometry(QRect(90, 178, 261, 31));
        lineEditOutput->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        labOutput_4 = new QLabel(tab);
        labOutput_4->setObjectName(QString::fromUtf8("labOutput_4"));
        labOutput_4->setGeometry(QRect(10, 320, 81, 30));
        labOutput_4->setFont(font);
        labOutput_4->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        lineEditOpacity = new QLineEdit(tab);
        lineEditOpacity->setObjectName(QString::fromUtf8("lineEditOpacity"));
        lineEditOpacity->setEnabled(false);
        lineEditOpacity->setGeometry(QRect(370, 322, 61, 20));
        lineEditOpacity->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        lineEditOpacity->setReadOnly(true);
        labOutput_3 = new QLabel(tab);
        labOutput_3->setObjectName(QString::fromUtf8("labOutput_3"));
        labOutput_3->setGeometry(QRect(10, 290, 81, 22));
        labOutput_3->setFont(font);
        labOutput_3->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        btnSelectInput = new QPushButton(tab);
        btnSelectInput->setObjectName(QString::fromUtf8("btnSelectInput"));
        btnSelectInput->setGeometry(QRect(353, 128, 71, 25));
        btnSelectInput->setStyleSheet(QString::fromUtf8(""));
        labOutput_2 = new QLabel(tab);
        labOutput_2->setObjectName(QString::fromUtf8("labOutput_2"));
        labOutput_2->setGeometry(QRect(10, 218, 81, 30));
        labOutput_2->setFont(font);
        labOutput_2->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        label = new QLabel(tab);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(153, 226, 51, 22));
        label->setStyleSheet(QString::fromUtf8("color: rgb(121, 121, 121);\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        btnColorSelect = new QPushButton(tab);
        btnColorSelect->setObjectName(QString::fromUtf8("btnColorSelect"));
        btnColorSelect->setGeometry(QRect(355, 222, 71, 25));
        btnColorSelect->setStyleSheet(QString::fromUtf8(""));
        btnAddWater = new QPushButton(tab);
        btnAddWater->setObjectName(QString::fromUtf8("btnAddWater"));
        btnAddWater->setGeometry(QRect(90, 360, 101, 31));
        btnAddWater->setStyleSheet(QString::fromUtf8(""));
        labWater = new QLabel(tab);
        labWater->setObjectName(QString::fromUtf8("labWater"));
        labWater->setGeometry(QRect(10, 40, 81, 30));
        labWater->setFont(font);
        labWater->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        labWater->setTextFormat(Qt::PlainText);
        labOutput = new QLabel(tab);
        labOutput->setObjectName(QString::fromUtf8("labOutput"));
        labOutput->setGeometry(QRect(10, 177, 81, 30));
        labOutput->setFont(font);
        labOutput->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        lineEditInput = new CustomLineEdit(tab);
        lineEditInput->setObjectName(QString::fromUtf8("lineEditInput"));
        lineEditInput->setGeometry(QRect(90, 128, 261, 31));
        lineEditInput->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        sliderOpacity = new CustomSlider(tab);
        sliderOpacity->setObjectName(QString::fromUtf8("sliderOpacity"));
        sliderOpacity->setGeometry(QRect(90, 322, 271, 22));
        sliderOpacity->setSingleStep(0);
        sliderOpacity->setValue(20);
        sliderOpacity->setSliderPosition(20);
        sliderOpacity->setOrientation(Qt::Horizontal);
        sliderRotate = new CustomSlider(tab);
        sliderRotate->setObjectName(QString::fromUtf8("sliderRotate"));
        sliderRotate->setGeometry(QRect(90, 292, 271, 22));
        sliderRotate->setMinimum(0);
        sliderRotate->setMaximum(180);
        sliderRotate->setSingleStep(0);
        sliderRotate->setPageStep(10);
        sliderRotate->setValue(57);
        sliderRotate->setSliderPosition(57);
        sliderRotate->setOrientation(Qt::Horizontal);
        btnExportPDF = new QPushButton(tab);
        btnExportPDF->setObjectName(QString::fromUtf8("btnExportPDF"));
        btnExportPDF->setGeometry(QRect(250, 360, 101, 31));
        btnExportPDF->setStyleSheet(QString::fromUtf8(""));
        cBoxResolution = new QComboBox(tab);
        cBoxResolution->addItem(QString());
        cBoxResolution->addItem(QString());
        cBoxResolution->addItem(QString());
        cBoxResolution->addItem(QString());
        cBoxResolution->addItem(QString());
        cBoxResolution->addItem(QString());
        cBoxResolution->setObjectName(QString::fromUtf8("cBoxResolution"));
        cBoxResolution->setGeometry(QRect(91, 224, 61, 25));
        cBoxResolution->setStyleSheet(QString::fromUtf8("border: 1px solid #b8d4f0;"));
        cBoxResolution->setEditable(false);
        lineEdit_fs = new CustomLineEdit(tab);
        lineEdit_fs->setObjectName(QString::fromUtf8("lineEdit_fs"));
        lineEdit_fs->setGeometry(QRect(370, 260, 61, 21));
        lineEdit_fs->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        lineEdit_fs->setReadOnly(true);
        lineEditWaterText = new CustomTextEdit(tab);
        lineEditWaterText->setObjectName(QString::fromUtf8("lineEditWaterText"));
        lineEditWaterText->setGeometry(QRect(90, 10, 341, 111));
        label_2 = new QLabel(tab);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 260, 81, 22));
        label_2->setFont(font);
        label_2->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        sliderFontsize = new CustomSlider(tab);
        sliderFontsize->setObjectName(QString::fromUtf8("sliderFontsize"));
        sliderFontsize->setGeometry(QRect(90, 260, 271, 22));
        sliderFontsize->setMinimum(8);
        sliderFontsize->setMaximum(50);
        sliderFontsize->setSingleStep(0);
        sliderFontsize->setValue(25);
        sliderFontsize->setSliderPosition(25);
        sliderFontsize->setOrientation(Qt::Horizontal);
        tabWidget->addTab(tab, QString());
        labOutput->raise();
        labWater->raise();
        btnSelectOutput->raise();
        lineEditRotate->raise();
        labInput->raise();
        lineEditOutput->raise();
        labOutput_4->raise();
        lineEditOpacity->raise();
        labOutput_3->raise();
        btnSelectInput->raise();
        labOutput_2->raise();
        label->raise();
        btnColorSelect->raise();
        btnAddWater->raise();
        lineEditInput->raise();
        sliderOpacity->raise();
        sliderRotate->raise();
        btnExportPDF->raise();
        cBoxResolution->raise();
        lineEditColor->raise();
        cBoxFont->raise();
        lineEdit_fs->raise();
        lineEditWaterText->raise();
        sliderFontsize->raise();
        label_2->raise();
        tab_transform = new QWidget();
        tab_transform->setObjectName(QString::fromUtf8("tab_transform"));
        lineEditImageFile = new CustomLineEdit(tab_transform);
        lineEditImageFile->setObjectName(QString::fromUtf8("lineEditImageFile"));
        lineEditImageFile->setGeometry(QRect(80, 40, 210, 31));
        lineEditImageFile->setStyleSheet(QString::fromUtf8("color: rgb(52, 52, 52);"));
        lineEditImageFile->setReadOnly(true);
        label_i2p = new QLabel(tab_transform);
        label_i2p->setObjectName(QString::fromUtf8("label_i2p"));
        label_i2p->setGeometry(QRect(10, 42, 71, 31));
        btnSelectImageFile = new QPushButton(tab_transform);
        btnSelectImageFile->setObjectName(QString::fromUtf8("btnSelectImageFile"));
        btnSelectImageFile->setGeometry(QRect(291, 40, 61, 31));
        btnTransform = new QPushButton(tab_transform);
        btnTransform->setObjectName(QString::fromUtf8("btnTransform"));
        btnTransform->setGeometry(QRect(370, 40, 75, 31));
        label_i2p_2 = new QLabel(tab_transform);
        label_i2p_2->setObjectName(QString::fromUtf8("label_i2p_2"));
        label_i2p_2->setGeometry(QRect(10, 10, 71, 21));
        label_i2p_2->setStyleSheet(QString::fromUtf8("font: 700 9pt \"Microsoft YaHei UI\";"));
        label_i2p_3 = new QLabel(tab_transform);
        label_i2p_3->setObjectName(QString::fromUtf8("label_i2p_3"));
        label_i2p_3->setGeometry(QRect(10, 80, 271, 21));
        label_i2p_3->setStyleSheet(QString::fromUtf8("font: 700 9pt \"Microsoft YaHei UI\";"));
        btnTransformBat = new QPushButton(tab_transform);
        btnTransformBat->setObjectName(QString::fromUtf8("btnTransformBat"));
        btnTransformBat->setGeometry(QRect(370, 110, 75, 31));
        btnSelectImageDir = new QPushButton(tab_transform);
        btnSelectImageDir->setObjectName(QString::fromUtf8("btnSelectImageDir"));
        btnSelectImageDir->setGeometry(QRect(291, 110, 71, 31));
        label_i2p_4 = new QLabel(tab_transform);
        label_i2p_4->setObjectName(QString::fromUtf8("label_i2p_4"));
        label_i2p_4->setGeometry(QRect(10, 117, 71, 21));
        lineEditImageDir = new CustomLineEdit(tab_transform);
        lineEditImageDir->setObjectName(QString::fromUtf8("lineEditImageDir"));
        lineEditImageDir->setGeometry(QRect(80, 110, 210, 31));
        lineEditImageDir->setStyleSheet(QString::fromUtf8("color: rgb(52, 52, 52);"));
        lineEditImageDir->setReadOnly(true);
        label_i2p_5 = new QLabel(tab_transform);
        label_i2p_5->setObjectName(QString::fromUtf8("label_i2p_5"));
        label_i2p_5->setGeometry(QRect(10, 152, 101, 21));
        label_i2p_5->setStyleSheet(QString::fromUtf8("font: 700 9pt \"Microsoft YaHei UI\";"));
        lineEditPdfFile = new CustomLineEdit(tab_transform);
        lineEditPdfFile->setObjectName(QString::fromUtf8("lineEditPdfFile"));
        lineEditPdfFile->setGeometry(QRect(80, 180, 210, 31));
        lineEditPdfFile->setStyleSheet(QString::fromUtf8("color: rgb(52, 52, 52);"));
        lineEditPdfFile->setReadOnly(true);
        btnPdfToImage = new QPushButton(tab_transform);
        btnPdfToImage->setObjectName(QString::fromUtf8("btnPdfToImage"));
        btnPdfToImage->setGeometry(QRect(370, 180, 75, 31));
        btnSelectPDFFile = new QPushButton(tab_transform);
        btnSelectPDFFile->setObjectName(QString::fromUtf8("btnSelectPDFFile"));
        btnSelectPDFFile->setGeometry(QRect(291, 180, 71, 31));
        label_i2p_6 = new QLabel(tab_transform);
        label_i2p_6->setObjectName(QString::fromUtf8("label_i2p_6"));
        label_i2p_6->setGeometry(QRect(10, 188, 61, 16));
        tabWidget->addTab(tab_transform, QString());
        label_i2p->raise();
        lineEditImageFile->raise();
        btnSelectImageFile->raise();
        btnTransform->raise();
        label_i2p_2->raise();
        label_i2p_3->raise();
        btnTransformBat->raise();
        btnSelectImageDir->raise();
        label_i2p_4->raise();
        lineEditImageDir->raise();
        label_i2p_5->raise();
        btnPdfToImage->raise();
        btnSelectPDFFile->raise();
        label_i2p_6->raise();
        lineEditPdfFile->raise();
        tab_Filesplit = new QWidget();
        tab_Filesplit->setObjectName(QString::fromUtf8("tab_Filesplit"));
        lineEditInputFilesplit = new CustomLineEdit(tab_Filesplit);
        lineEditInputFilesplit->setObjectName(QString::fromUtf8("lineEditInputFilesplit"));
        lineEditInputFilesplit->setGeometry(QRect(88, 18, 270, 31));
        lineEditInputFilesplit->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        lineEditInputFilesplit->setReadOnly(true);
        btnSelectFilesplit = new QPushButton(tab_Filesplit);
        btnSelectFilesplit->setObjectName(QString::fromUtf8("btnSelectFilesplit"));
        btnSelectFilesplit->setGeometry(QRect(360, 18, 68, 31));
        btnSelectFilesplit->setStyleSheet(QString::fromUtf8(""));
        labInput_2 = new QLabel(tab_Filesplit);
        labInput_2->setObjectName(QString::fromUtf8("labInput_2"));
        labInput_2->setGeometry(QRect(10, 23, 81, 20));
        labInput_2->setFont(font);
        labInput_2->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        lineEditSplitpages = new CustomLineEdit(tab_Filesplit);
        lineEditSplitpages->setObjectName(QString::fromUtf8("lineEditSplitpages"));
        lineEditSplitpages->setGeometry(QRect(88, 58, 51, 21));
        lineEditSplitpages->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        lineEditSplitpages->setReadOnly(true);
        labInput_3 = new QLabel(tab_Filesplit);
        labInput_3->setObjectName(QString::fromUtf8("labInput_3"));
        labInput_3->setGeometry(QRect(10, 60, 81, 20));
        labInput_3->setFont(font);
        labInput_3->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        lineEditSplitStart = new CustomLineEdit(tab_Filesplit);
        lineEditSplitStart->setObjectName(QString::fromUtf8("lineEditSplitStart"));
        lineEditSplitStart->setGeometry(QRect(88, 90, 51, 21));
        lineEditSplitStart->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        labInput_4 = new QLabel(tab_Filesplit);
        labInput_4->setObjectName(QString::fromUtf8("labInput_4"));
        labInput_4->setGeometry(QRect(10, 91, 81, 20));
        labInput_4->setFont(font);
        labInput_4->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        lineEditSplitEnd = new CustomLineEdit(tab_Filesplit);
        lineEditSplitEnd->setObjectName(QString::fromUtf8("lineEditSplitEnd"));
        lineEditSplitEnd->setGeometry(QRect(150, 90, 51, 21));
        lineEditSplitEnd->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        labInput_5 = new QLabel(tab_Filesplit);
        labInput_5->setObjectName(QString::fromUtf8("labInput_5"));
        labInput_5->setGeometry(QRect(10, 130, 81, 20));
        labInput_5->setFont(font);
        labInput_5->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        radioButtonSplitaverage = new QRadioButton(tab_Filesplit);
        radioButtonSplitaverage->setObjectName(QString::fromUtf8("radioButtonSplitaverage"));
        radioButtonSplitaverage->setGeometry(QRect(90, 130, 121, 19));
        radioButtonSplitaverage->setStyleSheet(QString::fromUtf8("font: 10pt \"Microsoft YaHei UI\";"));
        radioButtonSplitaverage->setChecked(true);
        radioButtonSpliterange = new QRadioButton(tab_Filesplit);
        radioButtonSpliterange->setObjectName(QString::fromUtf8("radioButtonSpliterange"));
        radioButtonSpliterange->setGeometry(QRect(90, 170, 101, 19));
        radioButtonSpliterange->setStyleSheet(QString::fromUtf8("font: 10pt \"Microsoft YaHei UI\";"));
        lineEditSubPages = new CustomLineEdit(tab_Filesplit);
        lineEditSubPages->setObjectName(QString::fromUtf8("lineEditSubPages"));
        lineEditSubPages->setGeometry(QRect(211, 129, 51, 21));
        lineEditSubPages->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        labInput_6 = new QLabel(tab_Filesplit);
        labInput_6->setObjectName(QString::fromUtf8("labInput_6"));
        labInput_6->setGeometry(QRect(270, 130, 131, 20));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Microsoft YaHei UI"));
        font1.setPointSize(10);
        font1.setBold(false);
        font1.setItalic(false);
        font1.setWeight(50);
        labInput_6->setFont(font1);
        labInput_6->setStyleSheet(QString::fromUtf8("font: 10pt \"Microsoft YaHei UI\";"));
        lineEditSplitscope = new CustomLineEdit(tab_Filesplit);
        lineEditSplitscope->setObjectName(QString::fromUtf8("lineEditSplitscope"));
        lineEditSplitscope->setGeometry(QRect(210, 170, 191, 21));
        lineEditSplitscope->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        btnSplitPdf = new QPushButton(tab_Filesplit);
        btnSplitPdf->setObjectName(QString::fromUtf8("btnSplitPdf"));
        btnSplitPdf->setGeometry(QRect(90, 280, 81, 31));
        btnSplitPdf->setStyleSheet(QString::fromUtf8(""));
        labInput_7 = new QLabel(tab_Filesplit);
        labInput_7->setObjectName(QString::fromUtf8("labInput_7"));
        labInput_7->setGeometry(QRect(140, 90, 21, 20));
        QFont font2;
        font2.setFamily(QString::fromUtf8("\347\255\211\347\272\277"));
        font2.setPointSize(12);
        font2.setBold(false);
        font2.setItalic(false);
        font2.setWeight(50);
        labInput_7->setFont(font2);
        labInput_7->setStyleSheet(QString::fromUtf8("\n"
"font: 12pt \"\347\255\211\347\272\277\";"));
        btnSelectSplitDir = new QPushButton(tab_Filesplit);
        btnSelectSplitDir->setObjectName(QString::fromUtf8("btnSelectSplitDir"));
        btnSelectSplitDir->setGeometry(QRect(360, 200, 68, 31));
        btnSelectSplitDir->setStyleSheet(QString::fromUtf8(""));
        labInput_8 = new QLabel(tab_Filesplit);
        labInput_8->setObjectName(QString::fromUtf8("labInput_8"));
        labInput_8->setGeometry(QRect(12, 205, 81, 20));
        labInput_8->setFont(font);
        labInput_8->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        lineEditSplitOutput = new CustomLineEdit(tab_Filesplit);
        lineEditSplitOutput->setObjectName(QString::fromUtf8("lineEditSplitOutput"));
        lineEditSplitOutput->setGeometry(QRect(90, 200, 268, 31));
        lineEditSplitOutput->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        lineEditSplitOutput->setReadOnly(true);
        tabWidget->addTab(tab_Filesplit, QString());
        btnSelectFilesplit->raise();
        labInput_2->raise();
        lineEditInputFilesplit->raise();
        labInput_3->raise();
        lineEditSplitpages->raise();
        labInput_4->raise();
        lineEditSplitStart->raise();
        labInput_5->raise();
        radioButtonSplitaverage->raise();
        radioButtonSpliterange->raise();
        lineEditSubPages->raise();
        labInput_6->raise();
        lineEditSplitscope->raise();
        btnSplitPdf->raise();
        labInput_7->raise();
        lineEditSplitEnd->raise();
        btnSelectSplitDir->raise();
        labInput_8->raise();
        lineEditSplitOutput->raise();
        tab_Filemerge = new QWidget();
        tab_Filemerge->setObjectName(QString::fromUtf8("tab_Filemerge"));
        btnAddFile = new QPushButton(tab_Filemerge);
        btnAddFile->setObjectName(QString::fromUtf8("btnAddFile"));
        btnAddFile->setGeometry(QRect(170, 348, 101, 23));
        btnAddFile->setStyleSheet(QString::fromUtf8(""));
        tableWidgetMerge = new mytable(tab_Filemerge);
        if (tableWidgetMerge->columnCount() < 4)
            tableWidgetMerge->setColumnCount(4);
        tableWidgetMerge->setObjectName(QString::fromUtf8("tableWidgetMerge"));
        tableWidgetMerge->setGeometry(QRect(5, 0, 441, 341));
        tableWidgetMerge->setRowCount(0);
        tableWidgetMerge->setColumnCount(4);
        lineEditOutMerge = new CustomLineEdit(tab_Filemerge);
        lineEditOutMerge->setObjectName(QString::fromUtf8("lineEditOutMerge"));
        lineEditOutMerge->setGeometry(QRect(90, 420, 268, 31));
        lineEditOutMerge->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        lineEditOutMerge->setReadOnly(true);
        labOutput_5 = new QLabel(tab_Filemerge);
        labOutput_5->setObjectName(QString::fromUtf8("labOutput_5"));
        labOutput_5->setGeometry(QRect(12, 419, 81, 30));
        labOutput_5->setFont(font);
        labOutput_5->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        btnSelectMergeDir = new QPushButton(tab_Filemerge);
        btnSelectMergeDir->setObjectName(QString::fromUtf8("btnSelectMergeDir"));
        btnSelectMergeDir->setGeometry(QRect(360, 420, 70, 31));
        btnSelectMergeDir->setStyleSheet(QString::fromUtf8(""));
        btnMerge = new QPushButton(tab_Filemerge);
        btnMerge->setObjectName(QString::fromUtf8("btnMerge"));
        btnMerge->setGeometry(QRect(90, 457, 121, 30));
        btnMerge->setStyleSheet(QString::fromUtf8(""));
        labWater_3 = new QLabel(tab_Filemerge);
        labWater_3->setObjectName(QString::fromUtf8("labWater_3"));
        labWater_3->setGeometry(QRect(12, 386, 81, 30));
        labWater_3->setFont(font);
        labWater_3->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        lineEditMergeOutFile = new CustomLineEdit(tab_Filemerge);
        lineEditMergeOutFile->setObjectName(QString::fromUtf8("lineEditMergeOutFile"));
        lineEditMergeOutFile->setGeometry(QRect(90, 386, 161, 31));
        lineEditMergeOutFile->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        tabWidget->addTab(tab_Filemerge, QString());
        btnAddFile->raise();
        tableWidgetMerge->raise();
        labOutput_5->raise();
        btnSelectMergeDir->raise();
        lineEditOutMerge->raise();
        btnMerge->raise();
        labWater_3->raise();
        lineEditMergeOutFile->raise();
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        btnSearch = new QPushButton(tab_2);
        btnSearch->setObjectName(QString::fromUtf8("btnSearch"));
        btnSearch->setGeometry(QRect(280, 45, 52, 25));
        btnSelectInput_Search = new QPushButton(tab_2);
        btnSelectInput_Search->setObjectName(QString::fromUtf8("btnSelectInput_Search"));
        btnSelectInput_Search->setGeometry(QRect(355, 10, 71, 25));
        btnSelectInput_Search->setStyleSheet(QString::fromUtf8(""));
        lineEditInput_Search = new CustomLineEdit(tab_2);
        lineEditInput_Search->setObjectName(QString::fromUtf8("lineEditInput_Search"));
        lineEditInput_Search->setGeometry(QRect(75, 10, 271, 31));
        lineEditInput_Search->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        labWater_2 = new QLabel(tab_2);
        labWater_2->setObjectName(QString::fromUtf8("labWater_2"));
        labWater_2->setGeometry(QRect(0, 10, 81, 30));
        labWater_2->setFont(font);
        labWater_2->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        labWater_2->setTextFormat(Qt::PlainText);
        treeWidget_Search = new QTreeWidget(tab_2);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidget_Search->setHeaderItem(__qtreewidgetitem);
        treeWidget_Search->setObjectName(QString::fromUtf8("treeWidget_Search"));
        treeWidget_Search->setGeometry(QRect(0, 80, 447, 411));
        lineEditInput_Search_Key = new CustomLineEdit(tab_2);
        lineEditInput_Search_Key->setObjectName(QString::fromUtf8("lineEditInput_Search_Key"));
        lineEditInput_Search_Key->setGeometry(QRect(75, 45, 201, 31));
        lineEditInput_Search_Key->setStyleSheet(QString::fromUtf8("color: rgb(103, 103, 103);"));
        labWater_4 = new QLabel(tab_2);
        labWater_4->setObjectName(QString::fromUtf8("labWater_4"));
        labWater_4->setGeometry(QRect(0, 45, 81, 30));
        labWater_4->setFont(font);
        labWater_4->setStyleSheet(QString::fromUtf8("\n"
"font: 10pt \"\347\255\211\347\272\277\";"));
        labWater_4->setTextFormat(Qt::PlainText);
        btnSearch_export = new QPushButton(tab_2);
        btnSearch_export->setObjectName(QString::fromUtf8("btnSearch_export"));
        btnSearch_export->setGeometry(QRect(355, 45, 71, 25));
        tabWidget->addTab(tab_2, QString());
        btnSearch->raise();
        btnSelectInput_Search->raise();
        labWater_2->raise();
        treeWidget_Search->raise();
        labWater_4->raise();
        btnSearch_export->raise();
        lineEditInput_Search->raise();
        lineEditInput_Search_Key->raise();
        textEditLog = new QTextEdit(widget);
        textEditLog->setObjectName(QString::fromUtf8("textEditLog"));
        textEditLog->setGeometry(QRect(0, 520, 815, 61));
        textEditLog->setStyleSheet(QString::fromUtf8("border-color: rgb(255, 255, 255);"));

        gridLayout_2->addWidget(widget, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 818, 23));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        mainToolBar->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #f0f6ff);\n"
"border: 1px solid #b8d4f0;\n"
"border-radius: 4px;"));
        mainToolBar->setMovable(false);
        mainToolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionQuit);
        menuHelp->addAction(actionAbout);
        menuView->addAction(actionZoom_In);
        menuView->addAction(actionZoom_Out);
        menuView->addAction(actionPrevious_Page);
        menuView->addAction(actionNext_Page);
        menuView->addSeparator();
        menuView->addAction(actionContinuous);
        mainToolBar->addAction(actionOpen);
        mainToolBar->addAction(actionZoom_Out);
        mainToolBar->addAction(actionZoom_In);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "PDF Viewer", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200...", nullptr));
#if QT_CONFIG(tooltip)
        actionOpen->setToolTip(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\346\226\207\344\273\266", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionQuit->setText(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272", nullptr));
#if QT_CONFIG(tooltip)
        actionQuit->setToolTip(QCoreApplication::translate("MainWindow", "\345\205\263\351\227\255\345\272\224\347\224\250", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionQuit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAbout->setText(QCoreApplication::translate("MainWindow", "\345\205\263\344\272\216...", nullptr));
        actionAbout_Qt->setText(QCoreApplication::translate("MainWindow", "\345\205\263\344\272\216 Qt", nullptr));
#if QT_CONFIG(tooltip)
        actionAbout_Qt->setToolTip(QCoreApplication::translate("MainWindow", "\345\205\263\344\272\216 Qt", nullptr));
#endif // QT_CONFIG(tooltip)
        actionZoom_In->setText(QCoreApplication::translate("MainWindow", "\346\224\276\345\244\247", nullptr));
#if QT_CONFIG(shortcut)
        actionZoom_In->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+=", nullptr));
#endif // QT_CONFIG(shortcut)
        actionZoom_Out->setText(QCoreApplication::translate("MainWindow", "\347\274\251\345\260\217", nullptr));
#if QT_CONFIG(tooltip)
        actionZoom_Out->setToolTip(QCoreApplication::translate("MainWindow", "\347\274\251\345\260\217", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionZoom_Out->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+-", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPrevious_Page->setText(QCoreApplication::translate("MainWindow", "\344\270\212\344\270\200\351\241\265", nullptr));
#if QT_CONFIG(tooltip)
        actionPrevious_Page->setToolTip(QCoreApplication::translate("MainWindow", "\344\270\212\344\270\200\351\241\265", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionPrevious_Page->setShortcut(QCoreApplication::translate("MainWindow", "PgUp", nullptr));
#endif // QT_CONFIG(shortcut)
        actionNext_Page->setText(QCoreApplication::translate("MainWindow", "\344\270\213\344\270\200\351\241\265", nullptr));
#if QT_CONFIG(tooltip)
        actionNext_Page->setToolTip(QCoreApplication::translate("MainWindow", "\344\270\213\344\270\200\351\241\265", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionNext_Page->setShortcut(QCoreApplication::translate("MainWindow", "PgDown", nullptr));
#endif // QT_CONFIG(shortcut)
        actionContinuous->setText(QCoreApplication::translate("MainWindow", "\346\273\232\345\212\250\346\230\276\347\244\272", nullptr));
#if QT_CONFIG(tooltip)
        actionContinuous->setToolTip(QCoreApplication::translate("MainWindow", "\346\273\232\345\212\250\346\230\276\347\244\272", nullptr));
#endif // QT_CONFIG(tooltip)
        btnSelectOutput->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\347\233\256\345\275\225", nullptr));
        cBoxFont->setItemText(0, QCoreApplication::translate("MainWindow", "\346\226\260\345\256\213\344\275\223", nullptr));
        cBoxFont->setItemText(1, QCoreApplication::translate("MainWindow", "\346\245\267\344\275\223", nullptr));
        cBoxFont->setItemText(2, QCoreApplication::translate("MainWindow", "\344\273\277\345\256\213", nullptr));
        cBoxFont->setItemText(3, QCoreApplication::translate("MainWindow", "\351\273\221\344\275\223", nullptr));

        lineEditRotate->setText(QCoreApplication::translate("MainWindow", "57", nullptr));
        labInput->setText(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\350\276\223\345\205\245\357\274\232", nullptr));
#if QT_CONFIG(tooltip)
        lineEditColor->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" color:#0055ff;\">\350\256\276\347\275\256\346\260\264\345\215\260\346\226\207\346\234\254\347\232\204\351\242\234\350\211\262</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        lineEditColor->setText(QCoreApplication::translate("MainWindow", "#000000", nullptr));
        lineEditOutput->setText(QString());
        lineEditOutput->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250\347\233\256\345\275\225\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        labOutput_4->setText(QCoreApplication::translate("MainWindow", "\344\270\215\351\200\217\346\230\216\345\272\246\357\274\232", nullptr));
        lineEditOpacity->setText(QCoreApplication::translate("MainWindow", "20", nullptr));
        labOutput_3->setText(QCoreApplication::translate("MainWindow", "\346\226\207\346\234\254\346\227\213\350\275\254\357\274\232", nullptr));
        btnSelectInput->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\347\233\256\345\275\225", nullptr));
        labOutput_2->setText(QCoreApplication::translate("MainWindow", "\345\210\206 \350\276\250 \347\216\207\357\274\232", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\345\255\227\344\275\223:", nullptr));
        btnColorSelect->setText(QCoreApplication::translate("MainWindow", "\345\255\227\344\275\223\351\242\234\350\211\262", nullptr));
#if QT_CONFIG(tooltip)
        btnAddWater->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" color:#0055ff;\">\344\270\272\346\272\220PDF\346\226\207\344\273\266\345\242\236\345\212\240\346\260\264\345\215\260\346\226\207\346\234\254</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        btnAddWater->setText(QCoreApplication::translate("MainWindow", "\345\242\236\345\212\240\346\260\264\345\215\260", nullptr));
        labWater->setText(QCoreApplication::translate("MainWindow", "\346\260\264\345\215\260\346\226\207\346\234\254\357\274\232", nullptr));
        labOutput->setText(QCoreApplication::translate("MainWindow", "\350\276\223\345\207\272\347\233\256\345\275\225\357\274\232", nullptr));
        lineEditInput->setText(QString());
        lineEditInput->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250\347\233\256\345\275\225\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
#if QT_CONFIG(tooltip)
        btnExportPDF->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:10pt; color:#0055ff;\">\345\260\206\345\242\236\345\212\240\350\277\207\346\260\264\345\215\260\347\232\204PDF\345\257\274\345\207\272\344\270\272\345\233\276\347\211\207PDF</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        btnExportPDF->setText(QCoreApplication::translate("MainWindow", "\345\257\274\345\207\272PDF", nullptr));
        cBoxResolution->setItemText(0, QCoreApplication::translate("MainWindow", "100", nullptr));
        cBoxResolution->setItemText(1, QCoreApplication::translate("MainWindow", "300", nullptr));
        cBoxResolution->setItemText(2, QCoreApplication::translate("MainWindow", "150", nullptr));
        cBoxResolution->setItemText(3, QCoreApplication::translate("MainWindow", "120", nullptr));
        cBoxResolution->setItemText(4, QCoreApplication::translate("MainWindow", "90", nullptr));
        cBoxResolution->setItemText(5, QCoreApplication::translate("MainWindow", "72", nullptr));

#if QT_CONFIG(tooltip)
        cBoxResolution->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" color:#0055ff;\">\345\275\261\345\223\215\345\257\274\345\207\272pdf\347\232\204\346\270\205\346\231\260\345\272\246\357\274\214 \346\225\260\345\200\274\350\266\212\351\253\230\346\270\205\346\231\260\345\272\246\350\266\212\351\253\230\357\274\214\345\257\274\345\207\272\346\226\207\344\273\266\350\266\212\345\244\247\343\200\202</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        lineEdit_fs->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" color:#0055ff;\">\345\215\225\350\241\214\346\226\207\346\234\254\346\227\266\345\277\275\347\225\245\345\255\227\345\217\267\345\261\236\346\200\247</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        lineEdit_fs->setText(QCoreApplication::translate("MainWindow", "25", nullptr));
        lineEditWaterText->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'SimSun'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Microsoft YaHei UI';\">\350\201\224\351\200\232\346\225\260\345\255\227\347\247\221\346\212\200\346\234\211\351\231\220\345\205\254\345\217\270\346\200\273\351\203\250xx\351\241\271\347\233\256\344\270\223\347\224\250\346\226\207\346\241\243</span></p></body></html>", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "\345\255\227\344\275\223\345\255\227\345\217\267:", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("MainWindow", "\346\260\264\345\215\260\346\223\215\344\275\234", nullptr));
        lineEditImageFile->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250\346\226\207\344\273\266\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        label_i2p->setText(QCoreApplication::translate("MainWindow", "\345\233\276\347\211\207\346\226\207\344\273\266:", nullptr));
        btnSelectImageFile->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251", nullptr));
        btnTransform->setText(QCoreApplication::translate("MainWindow", "\350\275\254\346\215\242", nullptr));
        label_i2p_2->setText(QCoreApplication::translate("MainWindow", "\345\233\276\347\211\207\350\275\254PDF:", nullptr));
        label_i2p_3->setText(QCoreApplication::translate("MainWindow", "\346\211\271\351\207\217\350\275\254PDF:", nullptr));
        btnTransformBat->setText(QCoreApplication::translate("MainWindow", "\350\275\254\346\215\242", nullptr));
        btnSelectImageDir->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251", nullptr));
        label_i2p_4->setText(QCoreApplication::translate("MainWindow", "\345\233\276\347\211\207\347\233\256\345\275\225:", nullptr));
        lineEditImageDir->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250\347\233\256\345\275\225\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        label_i2p_5->setText(QCoreApplication::translate("MainWindow", "PDF\350\275\254\345\233\276\347\211\207:", nullptr));
        lineEditPdfFile->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250\346\226\207\344\273\266\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        btnPdfToImage->setText(QCoreApplication::translate("MainWindow", "\350\275\254\346\215\242", nullptr));
        btnSelectPDFFile->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251", nullptr));
        label_i2p_6->setText(QCoreApplication::translate("MainWindow", "PDF\346\226\207\344\273\266:", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_transform), QCoreApplication::translate("MainWindow", "\350\275\254\346\215\242\345\267\245\345\205\267", nullptr));
        lineEditInputFilesplit->setText(QString());
        lineEditInputFilesplit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250PDF\346\226\207\344\273\266\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        btnSelectFilesplit->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\346\226\207\344\273\266", nullptr));
        labInput_2->setText(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\350\276\223\345\205\245\357\274\232", nullptr));
        lineEditSplitpages->setText(QString());
        lineEditSplitpages->setPlaceholderText(QString());
        labInput_3->setText(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\351\241\265\346\225\260\357\274\232", nullptr));
        lineEditSplitStart->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        lineEditSplitStart->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250\347\233\256\345\275\225\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        labInput_4->setText(QCoreApplication::translate("MainWindow", "\346\213\206\345\210\206\350\214\203\345\233\264\357\274\232", nullptr));
        lineEditSplitEnd->setText(QString());
        lineEditSplitEnd->setPlaceholderText(QString());
        labInput_5->setText(QCoreApplication::translate("MainWindow", "\346\213\206\345\210\206\346\226\271\345\274\217\357\274\232", nullptr));
        radioButtonSplitaverage->setText(QCoreApplication::translate("MainWindow", "\345\271\263\345\235\207\346\213\206\345\210\206 \346\257\217", nullptr));
        radioButtonSpliterange->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\350\214\203\345\233\264", nullptr));
        lineEditSubPages->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        lineEditSubPages->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250\347\233\256\345\275\225\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        labInput_6->setText(QCoreApplication::translate("MainWindow", "\351\241\265\344\277\235\345\255\230\344\270\200\344\270\252\346\226\207\346\241\243", nullptr));
        lineEditSplitscope->setText(QString());
        lineEditSplitscope->setPlaceholderText(QCoreApplication::translate("MainWindow", "\347\244\272\344\276\213\357\274\232 1-7,4-12,10-20", nullptr));
#if QT_CONFIG(tooltip)
        btnSplitPdf->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:10pt; color:#0055ff;\">\345\260\206\345\242\236\345\212\240\350\277\207\346\260\264\345\215\260\347\232\204PDF\345\257\274\345\207\272\344\270\272\345\233\276\347\211\207PDF</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        btnSplitPdf->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213\346\213\206\345\210\206", nullptr));
        labInput_7->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        btnSelectSplitDir->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\347\233\256\345\275\225", nullptr));
        labInput_8->setText(QCoreApplication::translate("MainWindow", "\350\276\223\345\207\272\347\233\256\345\275\225\357\274\232", nullptr));
        lineEditSplitOutput->setText(QString());
        lineEditSplitOutput->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250\347\233\256\345\275\225\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_Filesplit), QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\346\213\206\345\210\206", nullptr));
#if QT_CONFIG(tooltip)
        btnAddFile->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:10pt; color:#0055ff;\">\346\224\257\346\214\201PDF\346\226\207\344\273\266\346\213\226\345\205\245\350\241\250\346\240\274</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        btnAddFile->setText(QCoreApplication::translate("MainWindow", "+\345\242\236\345\212\240\346\226\207\344\273\266", nullptr));
#if QT_CONFIG(tooltip)
        tableWidgetMerge->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:10pt; color:#0055ff;\">\346\224\257\346\214\201PDF\346\226\207\344\273\266\346\213\226\345\205\245\350\241\250\346\240\274</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        lineEditOutMerge->setText(QString());
        lineEditOutMerge->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250\347\233\256\345\275\225\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        labOutput_5->setText(QCoreApplication::translate("MainWindow", "\350\276\223\345\207\272\347\233\256\345\275\225\357\274\232", nullptr));
        btnSelectMergeDir->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\347\233\256\345\275\225", nullptr));
        btnMerge->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213\345\220\210\345\271\266\346\226\207\344\273\266", nullptr));
        labWater_3->setText(QCoreApplication::translate("MainWindow", "\350\276\223\345\207\272\345\220\215\347\247\260\357\274\232", nullptr));
#if QT_CONFIG(tooltip)
        lineEditMergeOutFile->setToolTip(QCoreApplication::translate("MainWindow", "\345\220\210\345\271\266\345\220\216\347\232\204\346\226\207\344\273\266\345\220\215\347\247\260", nullptr));
#endif // QT_CONFIG(tooltip)
        lineEditMergeOutFile->setText(QString());
        lineEditMergeOutFile->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\220\210\345\271\266\345\220\216\347\232\204\346\226\207\344\273\266\345\220\215\347\247\260", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_Filemerge), QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\345\220\210\345\271\266", nullptr));
        btnSearch->setText(QCoreApplication::translate("MainWindow", "\346\220\234\347\264\242", nullptr));
        btnSelectInput_Search->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\347\233\256\345\275\225", nullptr));
        lineEditInput_Search->setText(QString());
        lineEditInput_Search->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\217\257\344\273\245\346\213\226\345\212\250excel\346\226\207\344\273\266\347\233\256\345\275\225\345\210\260\350\257\245\346\226\207\346\234\254\346\241\206\344\270\255", nullptr));
        labWater_2->setText(QCoreApplication::translate("MainWindow", "xlsx\347\233\256\345\275\225\357\274\232", nullptr));
        lineEditInput_Search_Key->setText(QString());
        lineEditInput_Search_Key->setPlaceholderText(QCoreApplication::translate("MainWindow", "\350\276\223\345\205\245\346\220\234\347\264\242\345\205\263\351\224\256\345\255\227", nullptr));
        labWater_4->setText(QCoreApplication::translate("MainWindow", "\345\205\263\351\224\256\345\255\227\357\274\232", nullptr));
        btnSearch_export->setText(QCoreApplication::translate("MainWindow", "\345\257\274\345\207\272", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("MainWindow", "\350\256\276\345\244\207\346\220\234\347\264\242", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "\345\270\256\345\212\251", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "\350\247\206\345\233\276", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
