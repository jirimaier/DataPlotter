//  Copyright (C) 2020-2024  Jiří Maier

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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "utils.h"
#include <QColor>
#include <QFileDialog>
#include <QMap>
#include <QObject>
#include <QVector>

const char UpdatesApi[] = "https://api.github.com/repos/jirimaier/DataPlotter/releases/latest";
const char DownloadUrl[] = "https://github.com/jirimaier/DataPlotter/releases";

#define ANALOG_COUNT 16
#define MATH_COUNT 3
#define LOGIC_BITS 32
#define LOGIC_GROUPS 3
#define INTERPOLATION_COUNT 2

#define SHOW_OPENGL_RECOMMENDATION_WHEN_SWITCHED_TO_FILLED true

#define TERMINAL_CLICK_BLINK_TIME 100

#define TERMINAL_DEFAULT_WIDTH 14 + 1
#define TERMINAL_DEFAULT_HEIGHT 10

#define IS_NUMERIC_CHAR(a) (isdigit(a) || a == '-' || a == ',')

#define LOGIC_COUNT LOGIC_BITS *LOGIC_GROUPS

/// Počet kanálů v grafu (každý logický bit počítá jako samostatný kanál, nezahrnuje interpolační kanály
#define ALL_COUNT (ANALOG_COUNT + MATH_COUNT + LOGIC_COUNT)

#define POINT_STYLE QCPScatterStyle::ssDisc

#define MAX_PLOT_ZOOMOUT 10000000000

#define PLOT_ELEMENTS_MOUSE_DISTANCE 10
#define TRACER_MOUSE_DISTANCE 20

#define CURSOR_ABSOLUTE ANALOG_COUNT + MATH_COUNT + LOGIC_GROUPS + 2
#define FFT_INDEX(a) (ANALOG_COUNT + MATH_COUNT + LOGIC_GROUPS + a)
#define IS_LOGIC_INDEX(index) ((index >= ANALOG_COUNT + MATH_COUNT) && !IS_FFT_INDEX(index) && index != CURSOR_ABSOLUTE)
#define IS_FFT_INDEX(chID) (chID == FFT_INDEX(0) || chID == FFT_INDEX(1))
#define INDEX_TO_FFT_CHID(chID) (chID - FFT_INDEX(0))

#define INTERPOLATION_CHID(a) (ALL_COUNT + a)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define EXPORT_XY -1
#define EXPORT_FFT -2
#define EXPORT_ALL -3
#define EXPORT_FREQTIME -4

#define IS_ANALOG_OR_MATH(ch) (ch < ANALOG_COUNT + MATH_COUNT)
#define IS_ANALOG_OR_MATH_OR_LOGIC(ch) (ch < ANALOG_COUNT + MATH_COUNT + LOGIC_GROUPS)
#define IS_LOGIC_CH(ch) ((ch >= ANALOG_COUNT + MATH_COUNT))
#define CH_LIST_INDEX_TO_LOGIC_GROUP(group) (group - ANALOG_COUNT - MATH_COUNT)
#define LOGIC_GROUP_TO_CH_LIST_INDEX(group) (group + ANALOG_COUNT + MATH_COUNT)

#define ChID_TO_LOGIC_GROUP(ch) ((ch - ANALOG_COUNT - MATH_COUNT) / LOGIC_BITS)
#define ChID_TO_LOGIC_GROUP_BIT(ch) ((ch - ANALOG_COUNT - MATH_COUNT) % LOGIC_BITS)

#endif // GLOBAL_H
