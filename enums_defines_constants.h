#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QObject>
#include <QVector>

namespace PlotStatus {
enum enumerator { run, pause };
}

namespace PlotRange {
enum enumerator { freeMove, fixedRange, rolling };
}

namespace GraphStyle {
enum enumerator { line, point, linePoint, hidden };
}

namespace DataMode {
enum enumerator { unknown, terminal, info, warning, settings, point, channel };
}

namespace OutputLevel {
enum enumerator { device = 0, error = 1, warning = 2, info = 3 };
}

namespace MathOperations {
enum enumetrator { add, subtract, multiply, divide, xy };
}

namespace DataLineType {
enum enumerator { command, dataEnded, dataTimeouted, dataImplicitEnded, debugMessage };
}

namespace HAxisType {
enum enumerator { none, fixed, MS, HMS };
}

namespace MessageLevel {
enum enumerator { deviceInfo = -1, deviceWarning = 0, error = 1, warning = 2, info = 3 };
}

#define CHANNEL_COUNT 64
#define MATH_COUNT 4

#define PORT_NUCLEO_DESCRIPTION_IDENTIFIER "ST"

#define POINT_STYLE QCPScatterStyle::ssDisc

#define MAX_PLOT_ZOOMOUT 1000000

namespace Global {
static bool platformIsBigEndian = false;

const static QString lineEndings[4] = {"", "\n", "\r", "\r\n"};

#define LOG_SET_SIZE 31 //                              0       1       2       3      4      5      6     7     8     9    10   11  12 13 14  15  16  17  18   19   20   21    22    23    24     25     26     27      28      29      30
const static double logaritmicSettings[LOG_SET_SIZE] = {0.0001, 0.0002, 0.0005, 0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000};
} // namespace Global

struct GlobalFunctions {
  static int roundToStandardValue(double value) {
    for (int i = 0; i < LOG_SET_SIZE; i++)
      if (value <= Global::logaritmicSettings[i])
        return i;
    return 28;
  }
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

Q_DECLARE_METATYPE(ChannelSettings_t)
Q_DECLARE_METATYPE(PlotSettings_t)
Q_DECLARE_METATYPE(PlotFrame_t)
Q_DECLARE_METATYPE(DataMode::enumerator)
Q_DECLARE_METATYPE(OutputLevel::enumerator)
Q_DECLARE_METATYPE(MessageLevel::enumerator)
Q_DECLARE_METATYPE(PlotStatus::enumerator)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define CMD_END_LENGTH (sizeof(CMD_END) - 1)
#define TIMEOUT_SYMBOL_LENGTH (sizeof(TIMEOUT_SYMBOL) - 1)
#define CMD_BEGIN_LENGTH (sizeof(CMD_BEGIN) - 1)

#define XY_CHANNEL -1
#define FFT_CHANNEL -1

#endif // SETTINGS_H
