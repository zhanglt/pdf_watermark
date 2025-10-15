RC_ICONS = serviceMonitor.ico
TEMPLATE = app
TARGET = pdfviewer

QMAKE_CXXFLAGS += /utf-8
CONFIG += utf8_source


QT += core gui widgets pdfwidgets printsupport svg network sql
QMAKE_CXXFLAGS_RELEASE += -O2       #开启深度优化O3
QMAKE_GENERATOR = ninja
SOURCES += \
    lib/pdflib.cpp \
    main.cpp \
    mainwindow.cpp \
    pageselector.cpp \
    src/function/FileDetector.cpp \
    src/function/FormatConverter.cpp \
    src/function/FileSystemUtils.cpp \
    src/function/GeometryUtils.cpp \
    src/function/PdfOperations.cpp \
    src/QProgressIndicator.cpp \
    src/function/StringConverter.cpp \
    src/function/WatermarkProcessor.cpp \
    src/controllers/ExcelSearchController.cpp \
    src/controllers/PdfConverterController.cpp \
    src/controllers/PdfSplitMergeController.cpp \
    src/controllers/PdfViewerController.cpp \
    src/controllers/WatermarkController.cpp \
    src/function/FileDetector.cpp \
    src/function/FileSystemUtils.cpp \
    src/function/FormatConverter.cpp \
    src/function/GeometryUtils.cpp \
    src/function/PdfOperations.cpp \
    src/function/StringConverter.cpp \
    src/function/WatermarkProcessor.cpp \
    src/lineedit/CustomLineEdit.cpp \
    src/mark/multiWatermarkThreadSingle.cpp \
    src/mark/watermarkThread.cpp \
    src/mark/watermarkThreadSingle.cpp \
    src/mark/wmark.cpp \
    src/model/device.cpp \
    src/model/devicelistmodel.cpp \
    src/model/transferinfo.cpp \
    src/model/transfertablemodel.cpp \
    src/pdf2image/pdf2ImageThreadSingle.cpp \
    src/search/SearchThread.cpp \
    src/share/sharedfilemanager.cpp \
    src/slider/CustomSlider.cpp \
    src/textedit/CustomTextEdit.cpp \
    src/transfer/devicebroadcaster.cpp \
    src/transfer/pullreceiver.cpp \
    src/transfer/pullsender.cpp \
    src/transfer/receiver.cpp \
    src/transfer/sender.cpp \
    src/transfer/transfer.cpp \
    src/transfer/transferserver.cpp \
    src/ui/aboutdialog.cpp \
    src/ui/browsesharedialog.cpp \
    src/ui/receiverselectordialog.cpp \
    src/ui/settings.cpp \
    src/ui/settingsdialog.cpp \
    src/ui/sharemanagerdialog.cpp \
    src/ui/util.cpp \
    zoomselector.cpp

HEADERS += \
    function.h \
    include/QProgressIndicator.h \
    include/function/FileDetector.h \
    include/function/FileSystemUtils.h \
    include/function/FormatConverter.h \
    include/function/GeometryUtils.h \
    include/function/PdfOperations.h \
    include/function/StringConverter.h \
    include/function/WatermarkProcessor.h \
    include/lineedit/CustomLineEdit.h \
    include/mark/mark.h \
    include/mark/multiWatermarkThreadSingle.h \
    include/mark/watermarkThread.h \
    include/mark/watermarkThreadSingle.h \
    include/mytable.h \
    include/pdf2image/pdf2ImageThreadSingle.h \
    include/search/SearchThread.h \
    include/slider/CustomSlider.h \
    include/textedit/CustomTextEdit.h \
    lib/pdflib.h \
    lib/pdflib.hpp \
    mainwindow.h \
    pageselector.h \
    src/controllers/ExcelSearchController.h \
    src/controllers/PdfConverterController.h \
    src/controllers/PdfSplitMergeController.h \
    src/controllers/PdfViewerController.h \
    src/controllers/WatermarkController.h \
    src/model/device.h \
    src/model/devicelistmodel.h \
    src/model/transferinfo.h \
    src/model/transfertablemodel.h \
    src/share/shareconfig.h \
    src/share/sharedfilemanager.h \
    src/share/shareditem.h \
    src/transfer/devicebroadcaster.h \
    src/transfer/pullreceiver.h \
    src/transfer/pullsender.h \
    src/transfer/receiver.h \
    src/transfer/sender.h \
    src/transfer/transfer.h \
    src/transfer/transferserver.h \
    src/ui/aboutdialog.h \
    src/ui/browsesharedialog.h \
    src/ui/receiverselectordialog.h \
    src/ui/settings.h \
    src/ui/settingsdialog.h \
    src/ui/sharemanagerdialog.h \
    src/ui/util.h \
    zoomselector.h

FORMS += \
    mainwindow.ui \
    src/ui/aboutdialog.ui \
    src/ui/browsesharedialog.ui \
    src/ui/receiverselectordialog.ui \
    src/ui/settingsdialog.ui \
    src/ui/sharemanagerdialog.ui

RESOURCES += \
    resources.qrc \
    src/res.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/pdfwidgets/pdfviewer
INSTALLS += target

DISTFILES += \
    lib/pdflib.dll \
    lib/pdflib.lib \
    qss/pdf.css

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/mupdf/lib/release/ -llibmupdf
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/mupdf/lib/debug/ -llibmupdf

INCLUDEPATH += $$PWD/''
DEPENDPATH += $$PWD/''
INCLUDEPATH += $$PWD/lib/mupdf/include/

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/release/liblibmupdf.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/debug/liblibmupdf.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/release/libmupdf.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/debug/libmupdf.lib






win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/mupdf/lib/release/ -llibpkcs7
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/mupdf/lib/debug/ -llibpkcs7

INCLUDEPATH += $$PWD/''
DEPENDPATH += $$PWD/''

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/release/liblibpkcs7.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/debug/liblibpkcs7.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/release/libpkcs7.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/debug/libpkcs7.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/mupdf/lib/release/ -llibmuthreads
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/mupdf/lib/debug/ -llibmuthreads

INCLUDEPATH += $$PWD/lib/mupdf/lib/Release
DEPENDPATH += $$PWD/lib/mupdf/lib/Release

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/release/liblibmuthreads.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/debug/liblibmuthreads.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/release/libmuthreads.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/debug/libmuthreads.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/mupdf/lib/release/ -llibthirdparty
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/mupdf/lib/debug/ -llibthirdparty

INCLUDEPATH += $$PWD/lib/mupdf/lib/Release
DEPENDPATH += $$PWD/lib/mupdf/lib/Release

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/release/liblibthirdparty.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/debug/liblibthirdparty.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/release/libthirdparty.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/mupdf/lib/debug/libthirdparty.lib





win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/libxml2/lib/release/ -llibxml2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/libxml2/lib/debug/ -llibxml2d

INCLUDEPATH += $$PWD/lib/libxml2/include/libxml2
DEPENDPATH += $$PWD/lib/libxml2/include/libxml2

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/iconv/lib/release/ -llibiconv
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/iconv/lib/debug/ -llibiconv

INCLUDEPATH += $$PWD/lib/iconv/include
DEPENDPATH += $$PWD/lib/iconv/include



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/qtxlsx/lib/release/ -lQt5Xlsx
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/qtxlsx/lib/debug/ -lQt5Xlsxd

INCLUDEPATH += $$PWD/pdfviewer/lib/qtxlsx/include
DEPENDPATH += $$PWD/pdfviewer/lib/qtxlsx/include

