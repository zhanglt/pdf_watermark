#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QLoggingCategory>
#include <QMainWindow>
#include <QThreadPool>

#include "QPdfDocument"
#include "function.h"
#include "include/mark/multiWatermarkThreadSingle.h "
#include "include/mark/watermarkThread.h"
#include "include/mark/watermarkThreadSingle.h"
#include "include/pdf2image/pdf2ImageThreadSingle.h"
#include "include/search/SearchThread.h"
#include <QMetaType>
Q_DECLARE_LOGGING_CATEGORY(lcExample)
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}

class QPdfDocument;
class QPdfView;
QT_END_NAMESPACE

class PageSelector;
class ZoomSelector;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  ZoomSelector *m_zoomSelector;
  watermarkThread *wmThread;
  watermarkThreadSingle *wmThreadSinge;
  multiWatermarkThreadSingle *mwmThreadSinge;
  pdf2imageThreadSingle *pdf2imageThread;
  QThreadPool threadPool;
  int *COUNT;
  QAtomicInt m_completedCount;
  int m_totalFiles;

 public slots:
  void open(const QUrl &docLocation, QPdfDocument::DocumentError &err);
  void viewWatermark();
  void addWatermarkSingle(QString text, QString inputDir, QString outputDir,
                          QString color, QString opacity, QString rotate,
                          QString font, QString fontSize);
  void exportPdf(QDir dir);
  // void SlotSetCurrRow();
 private slots:
  void bookmarkSelected(const QModelIndex &index);
  void qprogresssindicat();
  void initTable();

  // action handlers
  void on_actionOpen_triggered();
  void on_actionQuit_triggered();
  void on_actionAbout_triggered();
  void on_actionAbout_Qt_triggered();
  void on_actionZoom_In_triggered();
  void on_actionZoom_Out_triggered();
  void on_actionPrevious_Page_triggered();
  void on_actionNext_Page_triggered();
  void on_actionContinuous_triggered();

  void on_btnAddWater_clicked();
  void on_btnColorSelect_clicked();
  void on_btnSelectInput_clicked();
  void on_btnSelectOutput_clicked();

  void on_btnExportPDF_clicked();
  void on_cBoxFont_currentIndexChanged();

  void on_btnSelectImageFile_clicked();

  void on_btnTransform_clicked();

  void on_btnSelectImageDir_clicked();

  void on_btnTransformBat_clicked();

  void on_btnSelectPDFFile_clicked();

  void on_btnPdfToImage_clicked();

  void on_btnSelectFilesplit_clicked();

  void on_lineEditInputFilesplit_textChanged(const QString &filename);

  void on_btnSelectSplitDir_clicked();

  void on_btnSplitPdf_clicked();

  void on_lineEditSplitOutput_textChanged(const QString &filepath);

  void on_btnAddFile_clicked();

  void on_btnSelectMergeDir_clicked();

  void on_btnMerge_clicked();

  void on_lineEditInputFilesplit_editingFinished();



  void on_btnSelectInput_Search_clicked();


  void on_tabWidget_currentChanged(int index);
  
  void onSearchFinished(const QList<SearchResult> &results);
  void onSearchProgress(int processed, int total, const QString &currentFileName);
  void onSearchError(const QString &error);
  
  bool isTreeWidgetEmpty(QTreeWidget *treeWidget);
  QString exportTreeWidgetToExcel(QTreeWidget &treeWidget, const QString &exportFilePath);



  void on_btnSearch_clicked();

  void on_btnSearch_export_clicked();

signals:
  void Finished();

 private:
  Ui::MainWindow *ui;

  PageSelector *m_pageSelector;

  QPdfDocument *m_document;
  QLabel *m_title;
};

#endif  // MAINWINDOW_H
