#include "comboboxes.h" 

// Some constant definitions
#define COMBOBOXES_NUM_COLORS    17
#define COMBOBOXES_NUM_PENWIDTH  5 
#define COMBOBOXES_NUM_PENSTYLE  5
#define COMBOBOXES_PEN_THICKNESS 2
#define COMBOBOXES_MAINBOX_MARGIN_RIGHT   22
#define COMBOBOXES_MAINBOX_MARGIN_LEFT    5
#define COMBOBOXES_MAINBOX_MARGIN_TOP     7
#define COMBOBOXES_MAINBOX_MARGIN_BOTTOM  7
#define COMBOBOXES_DELEGATE_MARGIN_RIGHT  5
#define COMBOBOXES_DELEGATE_MARGIN_LEFT   5
#define COMBOBOXES_DELEGATE_MARGIN_TOP    3
#define COMBOBOXES_DELEGATE_MARGIN_BOTTOM 3

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

    r.setRight(r.right() - COMBOBOXES_DELEGATE_MARGIN_RIGHT);
    r.setLeft(r.left() + COMBOBOXES_DELEGATE_MARGIN_LEFT);
    r.setTop(r.top() + COMBOBOXES_DELEGATE_MARGIN_TOP);
    r.setBottom(r.bottom() - COMBOBOXES_DELEGATE_MARGIN_BOTTOM);
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
    opt.rect.setRight(opt.rect.right() - COMBOBOXES_MAINBOX_MARGIN_RIGHT);
    opt.rect.setLeft(opt.rect.left() + COMBOBOXES_MAINBOX_MARGIN_LEFT);
    opt.rect.setTop(opt.rect.top() + COMBOBOXES_MAINBOX_MARGIN_TOP);
    opt.rect.setBottom(opt.rect.bottom() - COMBOBOXES_MAINBOX_MARGIN_BOTTOM);
    painter.fillRect(opt.rect, itemData(currentIndex(), 
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
    QPen pen(Qt::black, itemData(currentIndex(), Qt::DisplayRole).toUInt());
    painter.setPen(pen);
    painter.drawLine(opt.rect.left() + COMBOBOXES_MAINBOX_MARGIN_LEFT,
                     opt.rect.top() + (opt.rect.height()/2),
                     opt.rect.right() - COMBOBOXES_MAINBOX_MARGIN_RIGHT, 
                     opt.rect.top() + (opt.rect.height()/2));
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
    QPen pen(Qt::black, COMBOBOXES_PEN_THICKNESS, (enum Qt::PenStyle)
            (itemData(currentIndex(), Qt::DisplayRole).toUInt()));
    painter.setPen(pen);
    painter.drawLine(opt.rect.left() + COMBOBOXES_MAINBOX_MARGIN_LEFT, 
                     opt.rect.top() + (opt.rect.height()/2),
                     opt.rect.right() - COMBOBOXES_MAINBOX_MARGIN_RIGHT, 
                      opt.rect.top() + (opt.rect.height()/2));
}

