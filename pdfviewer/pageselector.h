#ifndef PAGESELECTOR_H
#define PAGESELECTOR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPdfDocument;
class QPdfPageNavigation;
class QToolButton;
QT_END_NAMESPACE

class PageSelector : public QWidget {
  Q_OBJECT
 public:
  explicit PageSelector(QWidget *parent = nullptr);

  void setPageNavigation(QPdfPageNavigation *pageNavigation);

 private slots:
  void onCurrentPageChanged(int page);
  void pageNumberEdited();

 public:
  QPdfPageNavigation *m_pageNavigation;

  QLineEdit *m_pageNumberEdit;
  QLabel *m_pageCountLabel;

  QToolButton *m_previousPageButton;
  QToolButton *m_nextPageButton;
};

#endif  // PAGESELECTOR_H
