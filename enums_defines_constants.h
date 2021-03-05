//  Copyright (C) 2020-2021  Jiří Maier

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QObject>
#include <QVector>
#include <cmath>

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
enum enumGraphStyle { line = 0, point = 1, linePoint = 2, filled = 3, square = 4, squareFilled = 5 };
}

namespace DataMode {
enum enumDataMode { unknown, terminal, info, warning, settings, point, channel, echo, logicChannel, logicPoint };
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

namespace HAxisType {
enum enumHAxisType { none = 0, normal = 1, MS = 2, HMS = 3 };
}

namespace MessageLevel {
enum enumMessageLevel { deviceInfo = -1, deviceWarning = 0, error = 1, warning = 2, info = 3 };
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

struct ValueType {
  bool isBinary = true;
  enum Type { unsignedint, integer, floatingpoint, invalid, incomplete } type = incomplete;
  bool bigEndian = false;
  int bytes = 0;
  double multiplier = 1.0;
};

inline ValueType readValuePrefix(QByteArray& buffer, int& detectedPrefixLength) {
  ValueType valType;
  if (buffer.length() < 2)
    return valType; // Incomplete
  if (!isdigit(buffer.at(1))) {
    detectedPrefixLength = 3;
    if (buffer.length() < 3)
      return valType; // Incomplete
    switch (buffer.at(0)) {
      case 'T':
        valType.multiplier = 1e12;
        break;
      case 'G':
        valType.multiplier = 1e9;
        break;
      case 'M':
        valType.multiplier = 1e6;
        break;
      case 'k':
        valType.multiplier = 1e3;
        break;
      case 'h':
        valType.multiplier = 1e2;
        break;
      case 'D':
        valType.multiplier = 1e1;
        break;
      case 'd':
        valType.multiplier = 1e-1;
        break;
      case 'c':
        valType.multiplier = 1e-2;
        break;
      case 'm':
        valType.multiplier = 1e-3;
        break;
      case 'u':
        valType.multiplier = 1e-6;
        break;
      case 'n':
        valType.multiplier = 1e-9;
        break;
      case 'p':
        valType.multiplier = 1e-12;
        break;
      case 'f':
        valType.multiplier = 1e-15;
        break;
      case 'a':
        valType.multiplier = 1e-18;
        break;
      default:
        valType.type = ValueType::invalid;
        return valType; // Invalid
    }
  } else
    detectedPrefixLength = 2;

#define unitPosition 0
#define typePosition detectedPrefixLength-2
#define bytesPosition detectedPrefixLength-1

  switch (tolower(buffer.at(typePosition))) {
    case 'u':
      valType.type = ValueType::unsignedint;
      valType.bytes = buffer.at(bytesPosition) - '0';
      if (valType.bytes != 1 && valType.bytes != 2 && valType.bytes != 3 && valType.bytes != 4)
        valType.type = ValueType::invalid;
      break;
    case 'i':
      valType.type = ValueType::integer;
      valType.bytes = buffer.at(bytesPosition) - '0';
      if (valType.bytes != 1 && valType.bytes != 2 && valType.bytes != 4)
        valType.type = ValueType::invalid;
      break;
    case 'f':
      valType.type = ValueType::floatingpoint;
      valType.bytes = buffer.at(bytesPosition) - '0';
      if (valType.bytes != 4 && valType.bytes != 8)
        valType.type = ValueType::invalid;
      break;
    default:
      valType.type = ValueType::invalid;
      return valType; // Invalid
  }
  valType.bigEndian = (buffer.at(typePosition) == toupper(buffer.at(typePosition)));
  return valType;
}

inline QString valueTypeToString(ValueType val) {
  if (val.isBinary) {
    QString description;
    description = QString::number(val.bytes * 8) + "-bit ";
    switch (val.type) {
      case ValueType::Type::invalid:
        return "Invalid data";
      case ValueType::Type::incomplete:
        return "Incomplete data";
      case ValueType::Type::integer:
        description += "signed integer";
        break;
      case ValueType::Type::unsignedint:
        description += "unsigned integer";
        break;
      case ValueType::Type::floatingpoint:
        description += "floating point";
        break;
      default:
        break;
    }
    if (val.bigEndian)
      description.append(" (big endian)");
    else
      description.append(" (little endian)");
    return description;
  } else
    return ("Decimal");
}

#define IS_NUMERIC_CHAR(a) (isdigit(a) || a == '-' || a == ',')

#define ANALOG_COUNT 16
#define MATH_COUNT 3
#define LOGIC_BITS 32
#define LOGIC_GROUPS 3
#define LOGIC_COUNT LOGIC_BITS *LOGIC_GROUPS
#define ALL_COUNT (ANALOG_COUNT + MATH_COUNT + LOGIC_COUNT)

#define XY_AND_FFT_ALLWAYS_TOGETHER

#define PORT_NUCLEO_DESCRIPTION_IDENTIFIER "ST"

#define POINT_STYLE QCPScatterStyle::ssDisc

#define MAX_PLOT_ZOOMOUT 1000000

#define PLOT_ELEMENTS_MOUSE_DISTANCE 10
#define TRACER_MOUSE_DISTANCE 20

#define FFTID(a) (ANALOG_COUNT + MATH_COUNT + LOGIC_GROUPS +a)
#define IS_FFT(chID) (chID>=FFTID(0))
#define CHID_TO_FFT_CHID(chID)(chID-FFTID(0))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define EXPORT_XY -1
#define EXPORT_FFT -2

#define IS_ANALOG_OR_MATH(ch) (ch < ANALOG_COUNT + MATH_COUNT)
#define IS_ANALOG_OR_MATH_OR_LOGIC(ch) (ch < ANALOG_COUNT + MATH_COUNT+LOGIC_GROUPS)
#define IS_LOGIC_CH(ch) ((ch >= ANALOG_COUNT + MATH_COUNT))
#define CH_LIST_INDEX_TO_LOGIC_GROUP(group) (group - ANALOG_COUNT - MATH_COUNT)
#define LOGIC_GROUP_TO_CH_LIST_INDEX(group) (group + ANALOG_COUNT + MATH_COUNT)

#define ChID_TO_LOGIC_GROUP(ch) ((ch - ANALOG_COUNT - MATH_COUNT) / LOGIC_BITS)
#define ChID_TO_LOGIC_GROUP_BIT(ch) ((ch - ANALOG_COUNT - MATH_COUNT) % LOGIC_BITS)

const static QString lineEndings[4] = {"", "\n", "\r", "\r\n"};

#define LOG_SET_SIZE 48 //                              0     1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18    19    20    21    22    23
const static double logaritmicSettings[LOG_SET_SIZE] = {1e-9, 2e-9, 5e-9, 1e-8, 2e-8, 5e-8, 1e-7, 2e-7, 5e-7, 1e-6, 2e-6, 5e-6, 1e-5, 2e-5, 5e-5, 1e-4, 2e-4, 5e-4, 1e-3, 2e-3, 5e-3, 1e-2, 2e-2, 5e-2,
                                                        // 24 25    26    27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47
                                                        1e-1, 2e-1, 5e-1, 1e0, 2e0, 5e0, 1e1, 2e1, 5e1, 1e2, 2e2, 5e2, 1e3, 2e3, 5e3, 1e4, 2e4, 5e4, 1e5, 2e5, 5e5, 1e6, 2e6, 5e6
                                                       };

inline int indexOfStandardValuesCeil(double value) {
  for (int i = 0; i < LOG_SET_SIZE; i++)
    if (value <= logaritmicSettings[i])
      return i;
  return (LOG_SET_SIZE - 1);
}
inline double ceilToNiceValue(double value) {
  if (value > 0)
    return (logaritmicSettings[indexOfStandardValuesCeil(value)]);
  else if (value < 0)
    return (-logaritmicSettings[indexOfStandardValuesCeil(-value)]);
  else
    return 0;
}

inline int getAnalogChId(int number, ChannelType::enumChannelType type) {
  if (type == ChannelType::analog)
    return (number - 1);
  if (type == ChannelType::math)
    return (number + ANALOG_COUNT - 1);
  return 0;
}

/// Skupina od 0, bit od 0, Vrátí chID
inline int getLogicChannelID(int group, int bit) { return (ANALOG_COUNT + MATH_COUNT + (group) * LOGIC_BITS + bit); }

/// Chid od 0
inline QString getChName(int chid) {
  if (chid == ANALOG_COUNT + MATH_COUNT + LOGIC_GROUPS - 1)
    return (QObject::tr("Logic bit %1").arg(ChID_TO_LOGIC_GROUP_BIT(chid) + 1));
  if (chid >= ANALOG_COUNT + MATH_COUNT)
    return (QObject::tr("Logic %1 bit %2").arg(ChID_TO_LOGIC_GROUP(chid) + 1).arg(ChID_TO_LOGIC_GROUP_BIT(chid) + 1));
  if (chid >= ANALOG_COUNT)
    return (QObject::tr("Math %1").arg(chid - ANALOG_COUNT + 1));
  return (QObject::tr("Ch %1").arg(chid + 1));
}

static int fastLog10(double x) {
  x = abs(x);
  int result = 0;
  if (x > 1)
    for (double d = 10.0; d <= x; d *= 10, result++);
  else
    for (double d = 1; d > x; d /= 10, result--);
  return result;
}

static int fastPow10(int n) {
  if (n == 0)
    return 1;
  if (n > 0)
    return 10 * fastPow10(n - 1);
  else
    return fastPow10(n + 1) / 10;
}

/// Nejbližší vyšší (nebo rovná) mocnina 2
inline int nextPow2(int number) {
  for (int i = 1;; i++)
    if (pow(2, i) >= number)
      return (pow(2, i));
}

static QString toSignificantDigits(double x, double prec, bool noDecimalsIfInteger = false) {
  if (x == 0) {
    QString zero = "0.0000000000";
    return zero.left(prec + 1);
  }
  if (noDecimalsIfInteger)
    if (round(x) == x)
      return QString::number((int)round(x));
  int log10ofX = fastLog10(x);
  if (log10ofX >= prec - 1) {
    return QString::number((int)round(x));
  } else {
    QString result = QString::number((int)round(x * (fastPow10(prec - log10ofX - 1))));
    int decimalPoint = result.length() - prec + log10ofX + 1;
    if (decimalPoint > 0)
      result.insert(decimalPoint, '.');
    else {
      for (; decimalPoint < 0; decimalPoint++)
        result.push_front('0');
      result.push_front('.');
      result.push_front('0');
    }
    return result;
  }
}

inline QString floatToNiceString(double d, int significantDigits, bool justify, bool justifyUnit, bool noDecimalsIfInteger = false) {
  QString text = "";
  QString postfix = "";
  if (qIsInf(d))
    text = "\xe2\x88\x9e "; // Nekonečno
  else if (qIsNaN(d))
    text = "--- ";
  else {
    int order = fastLog10(d);

    if (order >= 18) {
      postfix = " E";
      d /= 1e18;
    } else if (order >= 15) {
      postfix = " P";
      d /= 1e15;
    } else if (order >= 12) {
      postfix = " T";
      d /= 1e12;
    } else if (order >= 9) {
      postfix = " G";
      d /= 1e9;
    } else if (order >= 6) {
      postfix = " M";
      d /= 1e6;
    } else if (order >= 3) {
      postfix = " k";
      d /= 1e3;
    } else if (order >= 0) {
      postfix = justifyUnit ? "  " : " ";
    } else if (order >= -3) {
      postfix = " m";
      d /= 1e-3;
    } else if (order >= -6) {
      postfix = " " + QString::fromUtf8("\xc2\xb5"); // mikro
      d /= 1e-6;
    } else if (order >= -9) {
      postfix = " n";
      d /= 1e-9;
    } else if (order >= -12) {
      postfix = " p";
      d /= 1e-12;
    } else if (order >= -15) {
      postfix = " f";
      d /= 1e-15;
    } else {
      d = 0;
      postfix = justify ? "  " : " ";
    }
    text = toSignificantDigits(d, significantDigits, noDecimalsIfInteger);
  }

  text.append(postfix);
  if (justify) {
    return text.rightJustified(significantDigits + ((text.right(1) == " " && !justifyUnit) ? 3 : 4));
  } else
    return text;
}

struct ChannelSettings_t {
  QColor color = QColor(Qt::black);
  int style = GraphStyle::line;
  double offset = 0;
  double scale = 1;
  bool inverted = false;
  bool visible = true;
};

#endif // SETTINGS_H
