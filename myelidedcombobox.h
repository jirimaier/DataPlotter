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

#ifndef MYELIDEDCOMBOBOX_H
#define MYELIDEDCOMBOBOX_H

#include <QObject>
#include <QWidget>
#include <QComboBox>
#include <QStylePainter>

/// Pokud je text v comboboxu delší než co se vejde, je oříznut a zakončen třemi tečkami
class MyElidedComboBox : public QComboBox {
  Q_OBJECT

  void paintEvent(QPaintEvent* event);

 public:
  explicit MyElidedComboBox(QWidget* parent = nullptr);

 signals:

};

#endif // MYELIDEDCOMBOBOX_H
