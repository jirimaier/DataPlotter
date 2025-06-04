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

#ifndef MYDIAL_H
#define MYDIAL_H

#include <QDebug>
#include <QDial>
#include <QObject>

#include "global.h"

/// Dial that jumps over preset values (1,2,5,10,20,50,100 ...)
class myDial : public QDial {
  Q_OBJECT
 public:
  explicit myDial(QWidget* parent = nullptr);

 private:
  int lastPosition;
  bool var = false;
  int roundToStandardValue(double value);

 private slots:
  void positionChanged(int position);

 public slots:
  /// Move the dial so its position is as close as possible to the selected value (rounded up)
  void updatePosition(double value);

 signals:
  /// The value was changed by the user
  void realValueChanged(double);
};

#endif // MYDIAL_H
