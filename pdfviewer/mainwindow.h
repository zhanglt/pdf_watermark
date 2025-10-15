#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QLoggingCategory>
#include <QMainWindow>
#include <QThreadPool>
#include <QSystemTrayIcon>
#include "QPdfDocument"
#include "function.h"
#include "include/mark/multiWatermarkThreadSingle.h "
#include "include/mark/watermarkThread.h"
#include "include/mark/watermarkThreadSingle.h"
#include "include/pdf2image/pdf2ImageThreadSingle.h"
#include "include/search/SearchThread.h"

#include "src/model/transfertablemodel.h"
#include "src/model/devicelistmodel.h"
#include "src/transfer/devicebroadcaster.h"
#include "src/transfer/transferserver.h"

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

  void on_lineEditInput_Search_Key_returnPressed();

  void on_actionSetting_triggered();

  void on_actionMyshare_triggered();

  void on_action_Shareview_triggered();
  void setMainWindowVisibility(bool visible);
  void onShowMainWindowTriggered();

signals:
  void Finished();

 protected:
  /** @brief 重写关闭事件，拦截窗口关闭操作 */
  void closeEvent(QCloseEvent *event) override;
  /** @brief 重写窗口状态改变事件，拦截最小化操作 */
  void changeEvent(QEvent *event) override;

 private:
  Ui::MainWindow *ui;
  // === UI初始化方法 ===
  /** @brief 创建所有QAction对象并连接信号槽 */
  void setupActions();
  /** @brief 设置系统托盘图标和菜单 */
  void setupSystrayIcon();


  /** @brief 打开设置对话框 */
  void onSettingsActionTriggered();

  /**
   * @brief 传输端口更改时重启TransferServer
   * @param newPort 新的传输端口
   *
   * 当设置对话框中的传输端口被修改时调用，
   * 关闭当前的TransferServer并使用新端口重新启动监听
   */
  void onTransferPortChanged(int newPort);

  QSystemTrayIcon* mSystrayIcon;               ///< 系统托盘图标
  QMenu* mSystrayMenu;                         ///< 系统托盘右键菜单

  //TransferTableModel* mSenderModel;            ///< 发送列表数据模型
  //TransferTableModel* mReceiverModel;          ///< 接收列表数据模型
  //DeviceListModel* mDeviceModel;               ///< 设备列表数据模型（局域网内发现的设备）

  DeviceBroadcaster* mBroadcaster;             ///< 设备广播器，用于发现和通告设备
  TransferServer* mTransServer;                ///< 传输服务器，监听接收请求

  // === 主菜单和工具栏动作 ===
  QAction* mShowMainWindowAction;              ///< 显示主窗口动作
  QAction* mSettingsAction;                    ///< 设置动作
  QAction* mAboutAction;                       ///< 关于动作
  //QAction* mAboutQtAction;                     ///< 关于Qt动作
  QAction* mQuitAction;                        ///< 退出动作







  PageSelector *m_pageSelector;

  QPdfDocument *m_document;
  QLabel *m_title;
};

#endif  // MAINWINDOW_H
