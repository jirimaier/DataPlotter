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

#ifndef MYCURSORSLIDER_H
#define MYCURSORSLIDER_H

#include <QObject>
#include <QSlider>

#include "enumsDefinesConstants.h"

class myCursorSlider : public QSlider {
  Q_OBJECT
 public:
  explicit myCursorSlider(QWidget* parent = nullptr);
  void updateRange(int min, int max);
  int getRealValue() { return realPos;};

 private slots:
  void positionChanged(int newpos);

 private:
  int realPos = 0;

 public slots:
  void setRealValue(int newValue);

 signals:
  void realValueChanged(int);
};

#endif // MYCURSORSLIDER_H
