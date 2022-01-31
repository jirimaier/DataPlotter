#ifndef MYPEAKPLOT_H
#define MYPEAKPLOT_H

#include <QObject>
#include "myplot.h"

class MyPeakPlot : public MyPlot {
  Q_OBJECT
 public:
  explicit MyPeakPlot(QWidget* parent = nullptr);
  void setUptimeTimer(QElapsedTimer* timer) {uptime = timer;}

  /// Exportuje oba kanály (nebo jeden, když nejsou oba využité)
  QByteArray exportCSV(char separator, char decimal, int precision);

  /// Vrátí rozsah vzorků v daném kanálu, které jsou ve viditelném rozsahu osy X
  QPair<unsigned int, unsigned int> getVisibleSamplesRange(int chID);

  /// Přizpůsobý barvu a popis, vrátí true, pokud se barva změnila
  bool setChSorce(int ch, int sourceChannel, QColor color);

  void setInfoText();

 private:
  bool autoSize = true;
  void autoset();
  void setMouseCursorStyle(QMouseEvent* event);
  void updateTracerText(int index);
  int currentTracerIndex = -1;
  int chSourceChannel[2];
  QColor chSourceColor[2];
  QPair<QVector<double>, QVector<double> > getDataVector(int chID);
  QCPItemText* infoText;
  QElapsedTimer* uptime;
  double timeLength = 100;

  QSharedPointer<MyAxisTickerWithUnit> unitTickerY2;

 public slots:
  /// Přidá data
  void newData(int chID, double freq) ;

  /// Vymaže kanál
  void clear(int chID);

  /// Vymaže celý graf
  void clear();

  /// Nastaví styl kanálu
  void setStyle(int chID, int style);

  /// Zapne/vypne automatický rozsah
  void setAutoSize(bool en);


 private slots:
  void mouseMoved(QMouseEvent* event);
  void mousePressed(QMouseEvent* event);
};

#endif // MYPEAKPLOT_H
