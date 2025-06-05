#include "qserialportinfo.h"
#include <QColor>
#include <QFileDialog>
#include <QMap>
#include <QObject>
#include <QVector>
#include <cmath>

#ifndef UTILS_H
#define UTILS_H

namespace PlotStatus {
enum enumPlotStatus { run, pause };
}

namespace ChannelType {
enum enumChannelType { analog, math, logic };
}

namespace FFTType {
enum enumFFTType { spectrum = 0, periodogram = 1, pwelch = 2 };
}

namespace FFTWindow {
enum enumFFTWindow { rectangular = 0, hamming = 1, hann = 2, blackman = 3 };
}

namespace PlotRange {
enum enumPlotRange { freeMove = 0, fixedRange = 1, rolling = 2 };
}

namespace GraphStyle {
enum enumGraphStyle { line = 0, point = 1, linePoint = 2, logic = 3, logicFilled = 4, logicpoints = 5, logicSquare = 6, logicSquareFilled = 7 };
}

namespace GraphType {
enum enumGraphType { analog, math, logic };
}

namespace DataMode {
enum enumDataMode { unknown, terminal, info, warning, settings, point, channel, echo, initialEcho, logicChannel, logicPoint, deviceerror, requestfile, qml, qmldirect, qmlvar, savefile };
}

namespace OutputLevel {
enum enumOutputLevel { device = 0, error = 1, warning = 2, info = 3 };
}

namespace MathOperations {
enum enumMathOperations { add = 0, subtract = 1, multiply = 2, divide = 3 };
}

namespace DataLineType {
enum enumDataLineType { command, dataEnded, dataTimeouted, dataImplicitEnded, debugMessage };
}

namespace MessageLevel {
enum enumMessageLevel : int { deviceInfo = -1, deviceWarning = 0, error = 1, warning = 2, info = 3 };
}

namespace Cursors {
enum enumCursors { Cursor1 = 0, Cursor2 = 1 };
}

namespace MessageTarget {
enum enumMessageTarget { manual, serial1 };
}

namespace TerminalMode {
enum enumTerminalMode { none, debug, clicksend, select };
}

struct UnitOfMeasure {
  UnitOfMeasure() {}
  UnitOfMeasure(QString rawUnit);
  enum Mode { usePrefix, noPrefix, index, time } mode = noPrefix;
  QString text = "";
  QString special = "";
  UnitOfMeasure reciprocal() {
    if (text == "s")
      return UnitOfMeasure("-Hz");
    else
      return UnitOfMeasure("!/" + text);
  }
  bool isDecibell() { return text.left(2) == "dB"; }
};

struct ValueType {
  ValueType(bool bin = true) : isBinary(bin) {}
  bool isBinary = true;
  enum Type { unsignedint, integer, floatingpoint, invalid, incomplete } type = incomplete;
  bool bigEndian = false;
  int bytes = 0;
  double multiplier = 1.0;
};

ValueType readValuePrefix(QByteArray &buffer, int &detectedPrefixLength);

QString valueTypeToString(ValueType val);

const static QString lineEndings[4] = {"", "\n", "\r", "\r\n"};

#define LOG_SET_SIZE 58 //                              0     1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18    19    20    21    22    23
const static double logaritmicSettings[LOG_SET_SIZE] = {1e-9, 2e-9, 5e-9, 1e-8, 2e-8, 5e-8, 1e-7, 2e-7, 5e-7, 1e-6, 2e-6, 5e-6, 1e-5, 2e-5, 5e-5, 1e-4, 2e-4, 5e-4, 1e-3, 2e-3, 5e-3, 1e-2, 2e-2, 5e-2,
                                                        // 24 25    26    27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51
                                                        1e-1, 2e-1, 5e-1, 1e0, 2e0, 5e0, 1e1, 2e1, 5e1, 1e2, 2e2, 5e2, 1e3, 2e3, 5e3, 1e4, 2e4, 5e4, 1e5, 2e5, 5e5, 1e6, 2e6, 5e6, 1e7, 2e7, 5e7, 1e8, 2e8, 5e8, 1e9, 2e9, 5e9, 1e10};
double ceilToNiceValue(double value);

double floorToNiceValue(double value);

int getAnalogChId(int number, ChannelType::enumChannelType type);

/// Group from 0 and bit from 0, returns chID
int getLogicChannelID(int group, int bit);

/// chid starting from 0
QString getChName(int chid);

int intLog10(double x);

/// Nearest power of two that is greater or equal
int nextPow2(int number);

/// Converts the number to text with (prec) significant digits
/// TrimZeroes removes zeros at the end of the decimal part
/// (e.g. instead of 1.200 it shows 1.2)
QString toSignificantDigits(double x, int prec, bool trimZeroes = false);

QString floatToNiceString(double d, int significantDigits, bool justify, bool justifyUnit, bool noDecimalsIfInteger = false, UnitOfMeasure unit = UnitOfMeasure(""));

inline double ceilToMultipleOf(double value, double multipleOf) { return (std::ceil(value / multipleOf) * multipleOf); }

bool operator==(const QSerialPortInfo &lhs, const QSerialPortInfo &rhs);

struct ChannelSettings_t {
  QColor color1 = QColor(Qt::black);
  QColor color2 = QColor(Qt::white);
  int style = GraphStyle::line;
  double offset = 0;
  double scale = 1;
  bool inverted = false;
  bool visible = true;
  bool interpolate = false;
  QColor color(int theme) const { return theme == 1 ? color1 : color2; }
};

struct ChannelExpectedRange {
  double maximum = 0;
  double minimum = 0;
  bool unknown = true;
};

#endif // UTILS_H
