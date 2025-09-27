#ifndef MULTIWATERMARKTHREADSINGLE_H
#define MULTIWATERMARKTHREADSINGLE_H

#include <qmap.h>

#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <QVector>
using namespace std;

typedef QMap<QString, int> wMap;  // typedef操作符为QMap起一别名

class multiWatermarkThreadSingle : public QObject, public QRunnable {
  Q_OBJECT
 public:
  multiWatermarkThreadSingle(QMutex *mutex = nullptr, wMap *wmap = nullptr);

  explicit multiWatermarkThreadSingle(QObject *parent = nullptr);

  void recvNum(int num);
  void setInputFilename(QString input);
  void setOutputFilename(QString output);
  void setText(QString text);
  void setOpacity(QString opacity);
  void setColor(QString color);
  void setFont(QString font);
  void setFontsize(QString fontsize);
  void setRotate(QString rotate);

  void run() override;
  QMutex *m_mutex;
  wMap *m_wmap;
  int m_value;

 signals:
  void sendArray(QVector<int> num);
  void addFinish(const wMap &map);

 private:
  int m_num;
  QString m_text = "联通数字科技有限公司总部投标专用文档", m_opacity = "15",
          m_color = "gray", m_font = "simkai", m_fontsize, m_rotate = "45",
          m_input, m_output;
};

#endif  // MULTIWATERMARKTHREADSINGLE_H
