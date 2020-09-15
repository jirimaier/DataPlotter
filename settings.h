#ifndef SETTINGS_H
#define SETTINGS_H

#include "enums.h"
#include <QObject>
#include <QVector>

struct channelSettings_t {
  QColor color = QColor(Qt::black);
  double offset = 0;
  double scale = 1;
  int style = PLOT_STYLE_LINE;
};

class Settings {
public:
  Settings();

  struct plotSettings_t {
    double rollingRange = 100;
    int zoom = 1000;
    double horizontalPos = 500;
    void replaceData();
    void clearGraphs();
    double verticalRange = 10;
    int verticalCenter = 0;
  } plotSettings;

  struct binDataSettings_t {
    int bits = 8;
    double valueMin = 0;
    double valueMax = 255;
    double timeStep = 1;
    int numCh = 1;
    int firstCh = 1;
    bool continuous = false;
  } binDataSettings;

  int dataMode = DATA_MODE_DATA_UNKNOWN;

  QVector<channelSettings_t *> channelSettings;

  int plotRangeType = PLOT_RANGE_FIXED;
};

#endif // SETTINGS_H
