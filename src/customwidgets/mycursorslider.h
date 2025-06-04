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

#ifndef MYCURSORSLIDER_H
#define MYCURSORSLIDER_H

#include <QObject>
#include <QSlider>

#include "global.h"

/// The slider can hold a real value outside its movement range.
/// Moving the slider changes the value within the min/max range (displayed interval in the plot),
/// but if the range changes so the original value is outside, the value is kept
/// until the slider is dragged again. If the stored value is out of range, the slider appears grey.
class myCursorSlider : public QSlider {
  Q_OBJECT
 public:
  explicit myCursorSlider(QWidget* parent = nullptr);

  /// Set the range in which the slider can be moved
  void updateRange(int min, int max);

  /// Return the stored value (which may be outside the range)
  int getRealValue() { return realPos;};

 private slots:
  /// Slider position changed
  void positionChanged(int newpos);

 private:
  /// Stored value (may be outside the range and thus differ from value())
  int realPos = 0;

 public slots:
  /// Set a value that may be outside the slider range
  void setRealValue(int newValue);

 signals:
  /// Stored value changed
  void realValueChanged(int);
};

#endif // MYCURSORSLIDER_H
