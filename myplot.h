#ifndef MYPLOT_H
#define MYPLOT_H

#include "enums_defines_constants.h"
#include "qcustomplot.h"
#include <QWidget>

class MyPlot : public QCustomPlot {
  Q_OBJECT
private:
  void updateGridX();
  void updateGridY();
  int xGridHint = -4;
  int yGridHint = -4;
  double lastGridX = 0, lastGridY = 0;
  QSharedPointer<QCPAxisTickerTime> timeTickerX, longTimeTickerX;
  QSharedPointer<QCPAxisTickerFixed> fixedTickerX, fixedTickerY;

public:
  explicit MyPlot(QWidget *parent = nullptr);
  void updateCursor(int cursor, double cursorPosition);
  void setCursorVisible(int cursor, bool visible) {
    if (cursors.at(cursor)->visible() != visible) {
      cursors.at(cursor)->setVisible(visible);
      replot();
    }
  }
  double getVDiv() { return fixedTickerY->tickStep(); }
  double getHDiv() { return fixedTickerX->tickStep(); }
  QImage toPNG() { return (this->toPixmap().toImage()); };

protected:
  QCPLayer *cursorLayer;
  QVector<QCPItemLine *> cursors;
  void initcursors();
  void setMouseControlls(bool enabled);

public slots:
  void setVerticalDiv(double value);
  void setHorizontalDiv(double value);
  void setShowVerticalValues(bool enabled);
  void setShowHorizontalValues(int type);
  void setXTitle(QString title);
  void setYTitle(QString title);
  void setGridHintX(int hint);
  void setGridHintY(int hint);

private slots:
  void onXRangeChanged(QCPRange range);
  void onYRangeChanged(QCPRange range);

signals:
  void gridChanged();
};
#endif // MYPLOT_H
