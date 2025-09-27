TEMPLATE = subdirs
SUBDIRS += pdfviewer

QMAKE_CXXFLAGS += /utf-8
CONFIG += utf8_source

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/pdfviewer/lib/mupdf/lib/release/ -llibpkcs7
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/pdfviewer/lib/mupdf/lib/debug/ -llibpkcs7

INCLUDEPATH += $$PWD/''
DEPENDPATH += $$PWD/''

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/release/liblibpkcs7.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/debug/liblibpkcs7.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/release/libpkcs7.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/debug/libpkcs7.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/pdfviewer/lib/mupdf/lib/release/ -llibmuthreads
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/pdfviewer/lib/mupdf/lib/debug/ -llibmuthreads

INCLUDEPATH += $$PWD/pdfviewer/lib/mupdf/lib/Release
DEPENDPATH += $$PWD/pdfviewer/lib/mupdf/lib/Release

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/release/liblibmuthreads.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/debug/liblibmuthreads.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/release/libmuthreads.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/debug/libmuthreads.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/pdfviewer/lib/mupdf/lib/release/ -llibthirdparty
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/pdfviewer/lib/mupdf/lib/debug/ -llibthirdparty

INCLUDEPATH += $$PWD/pdfviewer/lib/mupdf/lib/Release
DEPENDPATH += $$PWD/pdfviewer/lib/mupdf/lib/Release

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/release/liblibthirdparty.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/debug/liblibthirdparty.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/release/libthirdparty.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/pdfviewer/lib/mupdf/lib/debug/libthirdparty.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/pdfviewer/lib/qtxlsx/lib/release/ -lQt5Xlsx
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/pdfviewer/lib/qtxlsx/lib/debug/ -lQt5Xlsxd

INCLUDEPATH += $$PWD/pdfviewer/lib/qtxlsx/lib/Release
DEPENDPATH += $$PWD/pdfviewer/lib/qtxlsx/lib/Release
