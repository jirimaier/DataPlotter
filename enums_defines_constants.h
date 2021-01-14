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

namespace PlotStatus {
enum enumerator { run, pause };
}

namespace ChannelType {
enum enumerator { analog, math, logic };
}

namespace FFTType {
enum enumerator { spectrum = 0, periodogram = 1, pwelch = 2 };
}

namespace FFTWindow {
enum enumerator { rectangular = 0, hamming = 1, hann = 2, blackman = 3 };
}

namespace PlotRange {
enum enumerator { freeMove = 0, fixedRange = 1, rolling = 2 };
}

namespace GraphStyle {
enum enumerator { line = 0, point = 1, linePoint = 2, filled = 3, square = 4, squareFilled = 5 };
}

namespace DataMode {
enum enumerator { unknown, terminal, info, warning, settings, point, channel, echo };
}

namespace OutputLevel {
enum enumerator { device = 0, error = 1, warning = 2, info = 3 };
}

namespace MathOperations {
enum enumerator { add = 0, subtract = 1, multiply = 2, divide = 3 };
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

namespace Cursors {
enum enumerator { X1 = 0, X2 = 1, Y1 = 2, Y2 = 3 };
}

namespace MessageTarget {
enum enumerator { manual, serial1 };
}

namespace TerminalMode {
enum enumerator { none, debug, clicksend, select };
}

namespace ValueType {
enum enumerator { unrecognised, u1, u2, u3, u4, U1, U2, U3, U4, i1, i2, i3, i4, I1, I2, I3, I4, f4, F4, d8, D8 };
}

#define ANALOG_COUNT 16
#define MATH_COUNT 3
#define LOGIC_BITS 32
#define LOGIC_GROUPS 3
#define LOGIC_COUNT LOGIC_BITS *LOGIC_GROUPS
#define ALL_COUNT (ANALOG_COUNT + MATH_COUNT + LOGIC_COUNT)

#define PORT_NUCLEO_DESCRIPTION_IDENTIFIER "ST"

#define POINT_STYLE QCPScatterStyle::ssDisc

#define MAX_PLOT_ZOOMOUT 1000000

#define XYID ANALOG_COUNT + MATH_COUNT + LOGIC_GROUPS
#define FFTID ANALOG_COUNT + MATH_COUNT + LOGIC_GROUPS + 1

namespace Global {

const static QString lineEndings[4] = {"", "\n", "\r", "\r\n"};

#define LOG_SET_SIZE 31 //                              0       1       2       3      4      5      6     7     8     9    10   11  12 13 14  15  16  17  18   19   20   21    22    23    24     25     26     27      28      29      30
const static double logaritmicSettings[LOG_SET_SIZE] = {0.0001, 0.0002, 0.0005, 0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000};
} // namespace Global

struct GlobalFunctions {
  static int indexOfStandardValuesCeil(double value) {
    for (int i = 0; i < LOG_SET_SIZE; i++)
      if (value <= Global::logaritmicSettings[i])
        return i;
    return 28;
  }
  static int ceilToNiceValue(double value) {
    if (value > 0)
      return (Global::logaritmicSettings[indexOfStandardValuesCeil(value)]);
    else if (value < 0)
      return (-Global::logaritmicSettings[indexOfStandardValuesCeil(-value)]);
    else
      return 0;
  }

  static int getAnalogChId(int number, ChannelType::enumerator type) {
    if (type == ChannelType::analog)
      return (number - 1);
    if (type == ChannelType::math)
      return (number + ANALOG_COUNT - 1);
    return 0;
  }

  /// Skupina od 0, bit od 0, Vrátí chID
  static int getLogicChannelID(int group, int bit) { return (ANALOG_COUNT + MATH_COUNT + (group)*LOGIC_BITS + bit); }

  /// Chid od 0
  static QString getChName(int chid) {
    if (chid >= ANALOG_COUNT + MATH_COUNT)
      return ((QObject::tr("Logic %1 bit %2").arg((chid - ANALOG_COUNT - MATH_COUNT) / LOGIC_BITS + 1).arg((chid - ANALOG_COUNT - MATH_COUNT) % LOGIC_BITS + 1)));
    if (chid >= ANALOG_COUNT)
      return ((QObject::tr("Math %1").arg(chid - ANALOG_COUNT + 1)));
    return ((QObject::tr("Ch %1").arg(chid + 1)));
  }

  static QString floatToNiceString(double d, int prec = 2, bool zeroPadAtUnit = false) {
    QString text = "";
    if (d == Q_INFINITY) {
      text = "inf ";
      if (zeroPadAtUnit)
        text.append(" ");
    } else if (d == Q_QNAN) {
      text = "nan ";
      if (zeroPadAtUnit)
        text.append(" ");
    } else if (abs(d) >= 1e18)
      text = (QString::number(d / 1e18, 'f', prec) + " E");
    else if (abs(d) >= 1e15)
      text = (QString::number(d / 1e15, 'f', prec) + " P");
    else if (abs(d) >= 1e12)
      text = (QString::number(d / 1e12, 'f', prec) + " T");
    else if (abs(d) >= 1e9)
      text = (QString::number(d / 1e9, 'f', prec) + " G");
    else if (abs(d) >= 1e6)
      text = (QString::number(d / 1e6, 'f', prec) + " M");
    else if (abs(d) >= 1e3)
      text = (QString::number(d / 1e3, 'f', prec) + " k");
    else if (abs(d) >= 1) {
      text = (QString::number(d, 'f', prec) + " ");
      if (zeroPadAtUnit)
        text.append(" ");
    } else if (abs(d) >= 1e-3)
      text = (QString::number(d * 1e3, 'f', prec) + " m");
    else if (abs(d) >= 1e-6)
      text = (QString::number(d * 1e6, 'f', prec) + " " + QString::fromUtf8("\xc2\xb5")); // mikro
    else if (abs(d) >= 1e-9)
      text = (QString::number(d * 1e9, 'f', prec) + " n");
    else if (abs(d) >= 1e-12)
      text = (QString::number(d * 1e12, 'f', prec) + " p");
    else if (abs(d) >= 1e-15)
      text = (QString::number(d * 1e15, 'f', prec) + " f");
    else if (abs(d) >= 1e-18)
      text = (QString::number(d * 1e18, 'f', prec) + " a");
    else {
      text = "0.00 ";
      if (zeroPadAtUnit)
        text.append(" ");
    }
    return text.rightJustified(9);
    return text;
  }
};

struct ChannelSettings_t {
  QColor color = QColor(Qt::blue);
  int style = GraphStyle::line;
  double offset = 0;
  double scale = 1;
  bool inverted = false;
  bool visible = true;
};

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define CMD_END_LENGTH (sizeof(CMD_END) - 1)
#define TIMEOUT_SYMBOL_LENGTH (sizeof(TIMEOUT_SYMBOL) - 1)
#define CMD_BEGIN_LENGTH (sizeof(CMD_BEGIN) - 1)

#define XY_CHANNEL -1
#define FFT_CHANNEL -2

#endif // SETTINGS_H
