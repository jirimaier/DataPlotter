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

#include "global.h"

/// Slide může mít hodnotu (real value) mimo rozsah pohybu posuvníku
/// Posuvníkem se mění hodnota v rozsahu min max (zobrazený interval v grafu)
/// ale pokud si min max změní tak, že původní hodnota je mimo rozsah, tak hodnota zůstane zachována,
/// dokud není sliderem prhnuto na nové místo. Když je nastavená hodnota vně rozsahu posuvníku, je posuvník šedý.
class myCursorSlider : public QSlider {
  Q_OBJECT
 public:
  explicit myCursorSlider(QWidget* parent = nullptr);

  /// Nastaví rozsah v kterém lze posuvník tahat
  void updateRange(int min, int max);

  /// Vrátí nastavenou hodnotu (která může být mimo rozsah pohybu
  int getRealValue() { return realPos;};

 private slots:
  /// Změnila se poloha posuvníku
  void positionChanged(int newpos);

 private:
  /// Nastavená hodnota (může být mimo rozsah, tedy jiná než value() ).
  int realPos = 0;

 public slots:
  /// Nastavý hodnotu, která může být mimo rozsah pohybu
  void setRealValue(int newValue);

 signals:
  /// Změnila se nastavená hodnota
  void realValueChanged(int);
};

#endif // MYCURSORSLIDER_H
