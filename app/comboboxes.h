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
