/*
    Copyright (C) 2004-2010  Martin Jolicoeur (snmpb1@gmail.com) 

    This file is part of the SnmpB project 
    (http://sourceforge.net/projects/snmpb)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "comboboxes.h" 

// Some constant definitions
#define COMBOBOXES_NUM_COLORS    17
#define COMBOBOXES_NUM_PENWIDTH  5 
#define COMBOBOXES_NUM_PENSTYLE  5
#define COMBOBOXES_PEN_THICKNESS 2
#define COMBOBOXES_MAINBOX_MARGIN_RIGHT   3 
#define COMBOBOXES_MAINBOX_MARGIN_LEFT    3
#define COMBOBOXES_MAINBOX_MARGIN_TOP     3
#define COMBOBOXES_MAINBOX_MARGIN_BOTTOM  3 
#define COMBOBOXES_DELEGATE_MARGIN_RIGHT  5
#define COMBOBOXES_DELEGATE_MARGIN_LEFT   5
#define COMBOBOXES_DELEGATE_MARGIN_TOP    1 
#define COMBOBOXES_DELEGATE_MARGIN_BOTTOM 1 

QColorBoxDelegate::QColorBoxDelegate(QObject *parent): QItemDelegate(parent)
{
}

void QColorBoxDelegate::paint ( QPainter * painter, 
                                const QStyleOptionViewItem & option,
                                const QModelIndex & index ) const
{
    QRect r = option.rect;

    if (option.state & QStyle::State_Selected)
    {
        painter->save();
        painter->setBrush(option.palette.highlight());
        painter->setPen(Qt::NoPen);
        painter->drawRect(r);
    }

    r.adjust(COMBOBOXES_DELEGATE_MARGIN_LEFT, 
             COMBOBOXES_DELEGATE_MARGIN_TOP, 
             - COMBOBOXES_DELEGATE_MARGIN_RIGHT, 
             - COMBOBOXES_DELEGATE_MARGIN_BOTTOM);
    painter->fillRect(r, index.data().value<QColor>());

    if (option.state & QStyle::State_Selected)
        painter->restore();
}

QColorComboBox::QColorComboBox(QWidget *parent):QComboBox(parent)
{
    // Fill the color combobox ...
    model = new QStandardItemModel(COMBOBOXES_NUM_COLORS, 1);
    setModel(model);
    delegate = new QColorBoxDelegate(this);
    setItemDelegate(delegate);

    for (int row = 0; row < COMBOBOXES_NUM_COLORS; row++)
    {
        QModelIndex index = model->index(row, 0, QModelIndex());
        switch (row)
        {
            case 0: model->setData(index, QVariant(Qt::black)); break;
            case 1: model->setData(index, QVariant(Qt::white)); break;
            case 2: model->setData(index, QVariant(Qt::darkGray)); break;
            case 3: model->setData(index, QVariant(Qt::gray)); break;
            case 4: model->setData(index, QVariant(Qt::lightGray)); break;
            case 5: model->setData(index, QVariant(Qt::red)); break;
            case 6: model->setData(index, QVariant(Qt::green)); break;
            case 7: model->setData(index, QVariant(Qt::blue)); break;
            case 8: model->setData(index, QVariant(Qt::cyan)); break;
            case 9: model->setData(index, QVariant(Qt::magenta)); break;            
            case 10: model->setData(index, QVariant(Qt::yellow)); break;
            case 11: model->setData(index, QVariant(Qt::darkRed)); break;
            case 12: model->setData(index, QVariant(Qt::darkGreen)); break;
            case 13: model->setData(index, QVariant(Qt::darkBlue)); break;
            case 14: model->setData(index, QVariant(Qt::darkCyan)); break;
            case 15: model->setData(index, QVariant(Qt::darkMagenta)); break;
            case 16: model->setData(index, QVariant(Qt::darkYellow)); break;
            default: break;
        }
    }
}

void QColorComboBox::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    QStyleOptionFocusRect opt;
    opt.initFrom(this);
    QStyleOptionComboBox optcb;
    optcb.initFrom(this);
    // draw the combobox frame.
    painter.drawComplexControl(QStyle::CC_ComboBox, optcb);
    // draw the element
    QRect ef = style()->subControlRect(QStyle::CC_ComboBox, &optcb, 
                                       QStyle::SC_ComboBoxEditField, this);
    ef.adjust(COMBOBOXES_MAINBOX_MARGIN_LEFT, 
              COMBOBOXES_MAINBOX_MARGIN_TOP, 
              - COMBOBOXES_MAINBOX_MARGIN_RIGHT, 
              - COMBOBOXES_MAINBOX_MARGIN_BOTTOM);
    painter.fillRect(ef, itemData(currentIndex(), 
                     Qt::DisplayRole).value<QColor>());
}

QPenWidthBoxDelegate::QPenWidthBoxDelegate(QObject *parent): QItemDelegate(parent)
{
}

void QPenWidthBoxDelegate::paint ( QPainter * painter, 
                                   const QStyleOptionViewItem & option,
                                   const QModelIndex & index ) const
{
    if (option.state & QStyle::State_Selected)
    {
        painter->save();
        painter->setBrush(option.palette.highlight());
        painter->setPen(Qt::NoPen);
        painter->drawRect(option.rect);
    }

    QPen pen(Qt::black, index.data().toUInt());
    painter->setPen(pen);
    painter->drawLine(option.rect.left() + COMBOBOXES_DELEGATE_MARGIN_LEFT, 
                      option.rect.top() + (option.rect.height()/2), 
                      option.rect.right() - COMBOBOXES_DELEGATE_MARGIN_RIGHT, 
                      option.rect.top() + (option.rect.height()/2));

    if (option.state & QStyle::State_Selected)
        painter->restore();
}

QPenWidthComboBox::QPenWidthComboBox(QWidget *parent):QComboBox(parent)
{
    // Fill the pen width combobox ...
    model = new QStandardItemModel(COMBOBOXES_NUM_PENWIDTH, 1);
    setModel(model);
    delegate = new QPenWidthBoxDelegate(this);
    setItemDelegate(delegate);

    for (int row = 0; row < COMBOBOXES_NUM_PENWIDTH; row++)
    {
        QModelIndex index = model->index(row, 0, QModelIndex());
        model->setData(index, QVariant(row + 1));
    }
}

void QPenWidthComboBox::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    QStyleOptionFocusRect opt;
    opt.initFrom(this);
    QStyleOptionComboBox optcb;
    optcb.initFrom(this);
    // draw the combobox frame.
    painter.drawComplexControl(QStyle::CC_ComboBox, optcb);
    // draw the element
    QRect ef = style()->subControlRect(QStyle::CC_ComboBox, &optcb, 
                                       QStyle::SC_ComboBoxEditField, this);
    QPen pen(Qt::black, itemData(currentIndex(), Qt::DisplayRole).toUInt());
    painter.setPen(pen);
    painter.drawLine(ef.left() + COMBOBOXES_MAINBOX_MARGIN_LEFT,
                     ef.top() + (ef.height()/2),
                     ef.right() - COMBOBOXES_MAINBOX_MARGIN_RIGHT, 
                     ef.top() + (ef.height()/2));
}

QPenStyleBoxDelegate::QPenStyleBoxDelegate(QObject *parent): QItemDelegate(parent)
{
}

void QPenStyleBoxDelegate::paint ( QPainter * painter, 
                                   const QStyleOptionViewItem & option,
                                   const QModelIndex & index ) const
{
    if (option.state & QStyle::State_Selected)
    {
        painter->save();
        painter->setBrush(option.palette.highlight());
        painter->setPen(Qt::NoPen);
        painter->drawRect(option.rect);
    }

    QPen pen(Qt::black, COMBOBOXES_PEN_THICKNESS, 
             (enum Qt::PenStyle)(index.data().toUInt()));
    painter->setPen(pen);
    painter->drawLine(option.rect.left() + COMBOBOXES_DELEGATE_MARGIN_LEFT, 
                      option.rect.top() + (option.rect.height()/2), 
                      option.rect.right() - COMBOBOXES_DELEGATE_MARGIN_RIGHT, 
                      option.rect.top() + (option.rect.height()/2));

    if (option.state & QStyle::State_Selected)
        painter->restore();
}

QPenStyleComboBox::QPenStyleComboBox(QWidget *parent):QComboBox(parent)
{
    // Fill the pen shape combobox ...
    model = new QStandardItemModel(COMBOBOXES_NUM_PENSTYLE, 1);
    setModel(model);
    delegate = new QPenStyleBoxDelegate(this);
    setItemDelegate(delegate);

    for (int row = 0; row < COMBOBOXES_NUM_PENSTYLE; row++)
    {
        QModelIndex index = model->index(row, 0, QModelIndex());
        switch (row)
        {
            case 0: model->setData(index, QVariant(Qt::SolidLine)); break;
            case 1: model->setData(index, QVariant(Qt::DashLine)); break;
            case 2: model->setData(index, QVariant(Qt::DotLine)); break;
            case 3: model->setData(index, QVariant(Qt::DashDotLine)); break;
            case 4: model->setData(index, QVariant(Qt::DashDotDotLine)); break;
            default: break;
        }
    }
}

void QPenStyleComboBox::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    QStyleOptionFocusRect opt;
    opt.initFrom(this);
    QStyleOptionComboBox optcb;
    optcb.initFrom(this);
    // draw the combobox frame.
    painter.drawComplexControl(QStyle::CC_ComboBox, optcb);
    // draw the element
    QRect ef = style()->subControlRect(QStyle::CC_ComboBox, &optcb, 
                                       QStyle::SC_ComboBoxEditField, this);
    QPen pen(Qt::black, COMBOBOXES_PEN_THICKNESS, (enum Qt::PenStyle)
            (itemData(currentIndex(), Qt::DisplayRole).toUInt()));
    painter.setPen(pen);
    painter.drawLine(ef.left() + COMBOBOXES_MAINBOX_MARGIN_LEFT, 
                     ef.top() + (ef.height()/2),
                     ef.right() - COMBOBOXES_MAINBOX_MARGIN_RIGHT, 
                     ef.top() + (ef.height()/2));
}

