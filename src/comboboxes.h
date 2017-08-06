/**
 * Copyright (c) 2004-2017 Martin Jolicoeur and contributors
 *
 * This file is part of the snmpb project and is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMBOBOXES_H
#define COMBOBOXES_H

#include <QComboBox>
#include <QItemDelegate>
#include <QStandardItemModel>
#include <QStylePainter>

class QColorBoxDelegate : public QItemDelegate
{
public:
    QColorBoxDelegate(QObject *parent = 0);
    void paint ( QPainter * painter, const QStyleOptionViewItem & option,
                 const QModelIndex & index ) const;
};

class QColorComboBox: public QComboBox
{
public:
    QColorComboBox(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

private:
    QStandardItemModel* model;
    QColorBoxDelegate* delegate;
};

class QPenWidthBoxDelegate : public QItemDelegate
{
public:
    QPenWidthBoxDelegate(QObject *parent = 0);
    void paint ( QPainter * painter, const QStyleOptionViewItem & option,
                 const QModelIndex & index ) const;
};

class QPenWidthComboBox: public QComboBox
{
public:
    QPenWidthComboBox(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

private:
    QStandardItemModel* model;
    QPenWidthBoxDelegate* delegate;
};

class QPenStyleBoxDelegate : public QItemDelegate
{
public:
    QPenStyleBoxDelegate(QObject *parent = 0);
    void paint ( QPainter * painter, const QStyleOptionViewItem & option,
                 const QModelIndex & index ) const;
};

class QPenStyleComboBox: public QComboBox
{
public:
    QPenStyleComboBox(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

private:
    QStandardItemModel* model;
    QPenStyleBoxDelegate* delegate;
};

#endif /* COMBOBOXES_H */
