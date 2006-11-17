#ifndef COMBOBOXES_H
#define COMBOBOXES_H

#include <QComboBox>
#include <QItemDelegate>
#include <QStandardItemModel>
#include <QStylePainter>

class QColorBoxDelegate : public QItemDelegate
{
public:
    QColorBoxDelegate(QObject *parent = 0): QItemDelegate(parent) {}

    void paint ( QPainter * painter, const QStyleOptionViewItem & option,
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

        r.setRight(r.right() - 3);
        r.setLeft(r.left() + 3);
        r.setTop(r.top() + 3);
        r.setBottom(r.bottom() - 3);
        painter->fillRect(r, index.data().value<QColor>());

        if (option.state & QStyle::State_Selected)
            painter->restore();
    }
};

class QColorComboBox: public QComboBox
{
public:
    QColorComboBox(QWidget *parent = 0):QComboBox(parent)
    {
        // Fill the color combobox ...
        ColorModel = new QStandardItemModel(17, 1);
        setModel(ColorModel);
        ColorDelegate = new QColorBoxDelegate(this);
        setItemDelegate(ColorDelegate);

        for (int row = 0; row < 17; row++)
        {
            QModelIndex index = ColorModel->index(row, 0, QModelIndex());
            switch (row)
            {
                case 0: ColorModel->setData(index, QVariant(Qt::black)); break;
                case 1: ColorModel->setData(index, QVariant(Qt::white)); break;
                case 2: ColorModel->setData(index, QVariant(Qt::darkGray)); break;
                case 3: ColorModel->setData(index, QVariant(Qt::gray)); break;
                case 4: ColorModel->setData(index, QVariant(Qt::lightGray)); break;
                case 5: ColorModel->setData(index, QVariant(Qt::red)); break;
                case 6: ColorModel->setData(index, QVariant(Qt::green)); break;
                case 7: ColorModel->setData(index, QVariant(Qt::blue)); break;
                case 8: ColorModel->setData(index, QVariant(Qt::cyan)); break;
                case 9: ColorModel->setData(index, QVariant(Qt::magenta)); break;            
                case 10: ColorModel->setData(index, QVariant(Qt::yellow)); break;
                case 11: ColorModel->setData(index, QVariant(Qt::darkRed)); break;
                case 12: ColorModel->setData(index, QVariant(Qt::darkGreen)); break;
                case 13: ColorModel->setData(index, QVariant(Qt::darkBlue)); break;
                case 14: ColorModel->setData(index, QVariant(Qt::darkCyan)); break;
                case 15: ColorModel->setData(index, QVariant(Qt::darkMagenta)); break;
                case 16: ColorModel->setData(index, QVariant(Qt::darkYellow)); break;
                default: break;
            }
        }
    }

    void paintEvent(QPaintEvent *)
    {
        QStylePainter painter(this);
        QStyleOptionFocusRect opt;
        opt.initFrom(this);
        QStyleOptionComboBox optcb;
        optcb.initFrom(this);
        // draw the combobox frame.
        painter.drawComplexControl(QStyle::CC_ComboBox, optcb);
        // draw the element
        opt.rect.setRight(opt.rect.right() - 20);
        opt.rect.setLeft(opt.rect.left() + 5);
        opt.rect.setTop(opt.rect.top() + 7);
        opt.rect.setBottom(opt.rect.bottom() - 7);
        painter.fillRect(opt.rect, itemData(currentIndex(), 
                                   Qt::DisplayRole).value<QColor>());
    }

private:
    QStandardItemModel* ColorModel;
    QColorBoxDelegate* ColorDelegate;
};

class QPenWidthBoxDelegate : public QItemDelegate
{
public:
    QPenWidthBoxDelegate(QObject *parent = 0): QItemDelegate(parent) {}

    void paint ( QPainter * painter, const QStyleOptionViewItem & option,
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
        painter->drawLine(option.rect.left() + 3, 
                          option.rect.top()+(option.rect.height()/2), 
                          option.rect.right() - 3, 
                          option.rect.top()+(option.rect.height()/2));

        if (option.state & QStyle::State_Selected)
            painter->restore();
    }
};

class QPenWidthComboBox: public QComboBox
{
public:
    QPenWidthComboBox(QWidget *parent = 0):QComboBox(parent)
    {
        // Fill the pen width combobox ...
        PenWidthModel = new QStandardItemModel(5, 1);
        setModel(PenWidthModel);
        PenWidthDelegate = new QPenWidthBoxDelegate(this);
        setItemDelegate(PenWidthDelegate);

        for (int row = 0; row < 5; row++)
        {
            QModelIndex index = PenWidthModel->index(row, 0, QModelIndex());
            PenWidthModel->setData(index, QVariant(row+1));
        }
    }

    void paintEvent(QPaintEvent *)
    {
        QStylePainter painter(this);
        QStyleOptionFocusRect opt;
        opt.initFrom(this);
        QStyleOptionComboBox optcb;
        optcb.initFrom(this);
        // draw the combobox frame.
        painter.drawComplexControl(QStyle::CC_ComboBox, optcb);
        // draw the element
        QPen pen(Qt::black, itemData(currentIndex(), Qt::DisplayRole).toUInt());
        painter.setPen(pen);
        painter.drawLine(opt.rect.left() + 5,
                         opt.rect.top()+(opt.rect.height()/2),
                         opt.rect.right() - 20, 
                         opt.rect.top()+(opt.rect.height()/2));
    }

private:
    QStandardItemModel* PenWidthModel;
    QPenWidthBoxDelegate* PenWidthDelegate;
};

class QPenStyleBoxDelegate : public QItemDelegate
{
public:
    QPenStyleBoxDelegate(QObject *parent = 0): QItemDelegate(parent) {}

    void paint ( QPainter * painter, const QStyleOptionViewItem & option,
                 const QModelIndex & index ) const
    {
        if (option.state & QStyle::State_Selected)
        {
            painter->save();
            painter->setBrush(option.palette.highlight());
            painter->setPen(Qt::NoPen);
            painter->drawRect(option.rect);
        }

        QPen pen(Qt::black, 2, (enum Qt::PenStyle)(index.data().toUInt()));
        painter->setPen(pen);
        painter->drawLine(option.rect.left() + 3, 
                          option.rect.top()+(option.rect.height()/2), 
                          option.rect.right() - 3, 
                          option.rect.top()+(option.rect.height()/2));

        if (option.state & QStyle::State_Selected)
            painter->restore();
    }
};

class QPenStyleComboBox: public QComboBox
{
public:
    QPenStyleComboBox(QWidget *parent = 0):QComboBox(parent)
    {
        // Fill the pen shape combobox ...
        PenStyleModel = new QStandardItemModel(5, 1);
        setModel(PenStyleModel);
        PenStyleDelegate = new QPenStyleBoxDelegate(this);
        setItemDelegate(PenStyleDelegate);

        for (int row = 0; row < 5; row++)
        {
            QModelIndex index = PenStyleModel->index(row, 0, QModelIndex());
            switch (row)
            {
                case 0: PenStyleModel->setData(index, QVariant(Qt::SolidLine)); break;
                case 1: PenStyleModel->setData(index, QVariant(Qt::DashLine)); break;
                case 2: PenStyleModel->setData(index, QVariant(Qt::DotLine)); break;
                case 3: PenStyleModel->setData(index, QVariant(Qt::DashDotLine)); break;
                case 4: PenStyleModel->setData(index, QVariant(Qt::DashDotDotLine)); break;
                default: break;
            }
        }
    }

    void paintEvent(QPaintEvent *)
    {
        QStylePainter painter(this);
        QStyleOptionFocusRect opt;
        opt.initFrom(this);
        QStyleOptionComboBox optcb;
        optcb.initFrom(this);
        // draw the combobox frame.
        painter.drawComplexControl(QStyle::CC_ComboBox, optcb);
        // draw the element
        QPen pen(Qt::black, 2, (enum Qt::PenStyle)
                 (itemData(currentIndex(), Qt::DisplayRole).toUInt()));
        painter.setPen(pen);
        painter.drawLine(opt.rect.left() + 5, 
                         opt.rect.top()+(opt.rect.height()/2),
                         opt.rect.right() - 20, 
                         opt.rect.top()+(opt.rect.height()/2));
    }

private:
    QStandardItemModel* PenStyleModel;
    QPenStyleBoxDelegate* PenStyleDelegate;
};

#endif /* COMBOBOXES_H */
