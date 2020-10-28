#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QObject>
#include <QVector>

enum PlotStatus { run, pause, single };

enum PlotRange { freeMove, fixedRange, rolling };

enum GraphStyle { line, point, linePoint, hidden };

enum DataMode { unknown, string, binData, terminal, info, warning, settings };

enum OutputLevel { none, low, high };

enum MathOperations { add, subtract, multiply, divide, xy };

enum DataLineType { command, dataEnded, dataTimeouted, dataImplicitEnded, debugMessage };

#define CHANNEL_COUNT 64
#define MATH_COUNT 4

#define CMD_BEGIN "<cmd>"
#define TIMEOUT_SYMBOL "<timeout>"
#define CMD_END "<end>"

#define PORT_NUCLEO_DESCRIPTION_IDENTIFIER "ST"

#define POINT_STYLE QCPScatterStyle::ssDisc

#define OUTPUT_SHORT_LINE_MAX_LENGTH 15

#define RESET_ON_CONNECT_DELAY 5

#define MAX_PLOT_ZOOMOUT 1000000

const QString lineEndings[4] = {"", "\n", "\r", "\r\n"};

//                            0      1      2      3      4      5      6       7
const float refreshRates[8] = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 100.0f, 144.0f};

#define LOG_SET_SIZE 31 //                       0       1       2       3      4      5      6     7     8     9    10   11  12 13 14  15  16  17  18   19   20   21    22    23    24     25     26     27      28      29      30
const double logaritmicSettings[LOG_SET_SIZE] = {0.0001, 0.0002, 0.0005, 0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000};

struct BinDataSettings_t {
  int bits = 8;
  double valueMin = 0;
  double valueMax = 255;
  double timeStep = 1;
  int numCh = 1;
  int firstCh = 1;
  bool continuous = false;
};

struct ChannelSettings_t {
  QColor color = QColor(Qt::black);
  int style = GraphStyle::line;
  QString name = "";
};

struct PlotSettings_t {
  double rollingRange = 100;
  int zoom = 1000;
  double horizontalPos = 500;
  double verticalRange = 10;
  int verticalCenter = 0;
};

struct PlotFrame_t {
  double xMinTotal = 0, xMaxTotal = 10, yMinTotal = 0, yMaxTotal = 10, xMinView = 0, xMaxView = 10, yMinView = 0, yMaxView = 10;
};

Q_DECLARE_METATYPE(BinDataSettings_t)
Q_DECLARE_METATYPE(ChannelSettings_t)
Q_DECLARE_METATYPE(PlotSettings_t)
Q_DECLARE_METATYPE(PlotFrame_t)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define CMD_END_LENGTH (sizeof(CMD_END) - 1)
#define TIMEOUT_SYMBOL_LENGTH (sizeof(TIMEOUT_SYMBOL) - 1)
#define CMD_BEGIN_LENGTH (sizeof(CMD_BEGIN) - 1)

#define XY_CHANNEL -1
#define FFT_CHANNEL -1

#endif // SETTINGS_H
