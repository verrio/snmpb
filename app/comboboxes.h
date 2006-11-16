#ifndef COMBOBOXES_H
#define COMBOBOXES_H

#include <QComboBox>
#include <QItemDelegate>
#include <QStylePainter>

class QColorComboBox: public QComboBox
{
public:
    QColorComboBox(QWidget *parent = 0):QComboBox(parent) {}

    void paintEvent(QPaintEvent *)
    {
        QStylePainter painter(this);
        QStyleOptionComboBox optcb;
        QStyleOptionFocusRect optfc;
        optcb.initFrom(this);
        optfc.initFrom(this);
        QRect r = optfc.rect;
        // draw the combobox frame.
        painter.drawComplexControl(QStyle::CC_ComboBox, optcb);
        // draw the element
        r.setRight(r.right() - 20);
        r.setLeft(r.left() + 5);
        r.setTop(r.top() + 7);
        r.setBottom(r.bottom() - 7);
        painter.fillRect(r, itemData(currentIndex(), 
                            Qt::DisplayRole).value<QColor>());
    }
};

class ColorBoxDelegate : public QItemDelegate
{
public:
    ColorBoxDelegate(QObject *parent = 0): QItemDelegate(parent) {}

    void paint ( QPainter * painter, const QStyleOptionViewItem & option,
                 const QModelIndex & index ) const
    {
        QRect r = option.rect;

        if (option.state & QStyle::State_Selected) {
            painter->save();
            painter->setBrush(option.palette.highlight());
            painter->setPen(Qt::NoPen);
            painter->drawRect(option.rect);
            painter->setPen(QPen(option.palette.highlightedText(), 0));
        }

        r.setRight(r.right() - 3);
        r.setLeft(r.left() + 3);
        r.setTop(r.top() + 3);
        r.setBottom(r.bottom() - 3);
        painter->fillRect( r, index.data().value<QColor>());

        if (option.state & QStyle::State_Selected)
            painter->restore();
    }
};

#endif /* COMBOBOXES_H */
