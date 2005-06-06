#include <qptrlist.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qcolor.h>
#include <qnamespace.h>
#include "graph.h"

class ColorListBoxItem : public QListBoxItem
{
public:
    ColorListBoxItem(QColor col)
        : QListBoxItem()
    {
        color = col;
        setCustomHighlighting( TRUE );
    }

protected:
    virtual void paint( QPainter * );
    virtual int width( const QListBox* ) const { return 65; }
    virtual int height( const QListBox* ) const { return 18; }
    
private:
    QColor color;
};

class PenWidthListBoxItem : public QListBoxItem
{
public:
    PenWidthListBoxItem(uint w)
        : QListBoxItem()
    {
        thewidth = w;
        setCustomHighlighting( TRUE );
    }

protected:
    virtual void paint( QPainter * );
    virtual int width( const QListBox* ) const { return 65; }
    virtual int height( const QListBox* ) const { return 18; }
    
private:
    uint thewidth;
};

class PenStyleListBoxItem : public QListBoxItem
{
public:
    PenStyleListBoxItem(enum Qt::PenStyle s)
        : QListBoxItem()
    {
        style = s;
        setCustomHighlighting( TRUE );
    }

protected:
    virtual void paint( QPainter * );
    virtual int width( const QListBox* ) const { return 65; }
    virtual int height( const QListBox* ) const { return 18; }
    
private:
    enum Qt::PenStyle style;
};

void ColorListBoxItem::paint( QPainter *painter )
{
    // evil trick: find out whether we are painted onto our listbox
    bool in_list_box = listBox() && listBox()->viewport() == painter->device();

    QRect r ( 0, 0, width( listBox() ), height( listBox() ) );
    if ( in_list_box && isSelected() )
        painter->eraseRect( r );
    painter->fillRect( 3, 3, width(listBox())-6, height(listBox())-6, color);
    if ( in_list_box && isCurrent() )
        listBox()->style().drawPrimitive( QStyle::PE_FocusRect, painter, r, 
                                          listBox()->colorGroup() );
}

void PenWidthListBoxItem::paint( QPainter *painter )
{
    // evil trick: find out whether we are painted onto our listbox
    bool in_list_box = listBox() && listBox()->viewport() == painter->device();
    
    QRect r ( 0, 0, width( listBox() ), height( listBox() ) );
    if ( in_list_box && isSelected() )
        painter->eraseRect( r );
    QPen pen( Qt::black, thewidth);
    painter->setPen(pen);
    painter->drawLine ( 0, height(listBox())/2, width(listBox()), height(listBox())/2);    
    if ( in_list_box && isCurrent() )
        listBox()->style().drawPrimitive( QStyle::PE_FocusRect, painter, r, 
                                          listBox()->colorGroup() );
}

void PenStyleListBoxItem::paint( QPainter *painter )
{
    // evil trick: find out whether we are painted onto our listbox
    bool in_list_box = listBox() && listBox()->viewport() == painter->device();
    
    QRect r ( 0, 0, width( listBox() ), height( listBox() ) );
    if ( in_list_box && isSelected() )
        painter->eraseRect( r );
    QPen pen( Qt::black, 2, style);
    painter->setPen(pen);
    painter->drawLine ( 0, height(listBox())/2, width(listBox()), height(listBox())/2);    
    if ( in_list_box && isCurrent() )
        listBox()->style().drawPrimitive( QStyle::PE_FocusRect, painter, r, 
                                          listBox()->colorGroup() );
}

GraphItem::GraphItem(QString name, QTabWidget* tab)
{
    PlotName = name;
    Tab = tab;
    PlotObj = new QwtPlot(PlotName);
    Tab->addTab(PlotObj, PlotName);
}

GraphItem::~GraphItem()
{
    Tab->removePage(PlotObj);
    delete PlotObj;
}

Graph::Graph(QTabWidget* GT, QPushButton* GC, QPushButton* GD,
             QComboBox* GN, QComboBox* PO, QPushButton* PA,
             QPushButton* PD, QComboBox* PI, QComboBox* PC,
             QComboBox* PS, QComboBox* PW)
{
    GraphTab = GT;
    GraphCreate = GC;
    GraphDelete = GD;
    GraphName = GN;
    PlotObject = PO;
    PlotAdd = PA;
    PlotDelete = PD;
    PlotIndex = PI;
    PlotColor = PC;
    PlotShape = PS;
    PlotWidth = PW;
    
    // Connect some signals
    connect( GC, SIGNAL( clicked() ), this, SLOT( CreateGraph() ));
    connect( GD, SIGNAL( clicked() ), this, SLOT( DeleteGraph() ));
    connect( PA, SIGNAL( clicked() ), this, SLOT( CreatePlot() ));
    connect( PD, SIGNAL( clicked() ), this, SLOT( DeletePlot() ));    
    
    // Fill the color combobox ...
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::black) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::white) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::darkGray) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::gray) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::lightGray) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::red) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::green) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::blue) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::cyan) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::magenta) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::yellow) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::darkRed) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::darkGreen) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::darkBlue) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::darkCyan) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::darkMagenta) );
    PlotColor->listBox()->insertItem( new ColorListBoxItem(Qt::darkYellow) );
    
    // Fill the pen width combobox ...
    PlotWidth->listBox()->insertItem( new PenWidthListBoxItem(1) );
    PlotWidth->listBox()->insertItem( new PenWidthListBoxItem(2) );
    PlotWidth->listBox()->insertItem( new PenWidthListBoxItem(3) );
    PlotWidth->listBox()->insertItem( new PenWidthListBoxItem(4) );
    PlotWidth->listBox()->insertItem( new PenWidthListBoxItem(5) );
    
    // Fill the pen shape combobox ...
    PlotShape->listBox()->insertItem( new PenStyleListBoxItem(Qt::SolidLine) );
    PlotShape->listBox()->insertItem( new PenStyleListBoxItem(Qt::DashLine) );
    PlotShape->listBox()->insertItem( new PenStyleListBoxItem(Qt::DotLine) );
    PlotShape->listBox()->insertItem( new PenStyleListBoxItem(Qt::DashDotLine) );
    PlotShape->listBox()->insertItem( new PenStyleListBoxItem(Qt::DashDotDotLine) );
}

void Graph::CreateGraph(void)
{
    if (!GraphName->currentText().isEmpty())
    {        
        QPtrListIterator<GraphItem> it( Items );
        GraphItem *GI;
        while ( (GI = it.current()) != 0 ) {
            if (GI->PlotName == GraphName->currentText())
            {
                QString err = QString("Graph \"%1\" already exist !")
                      .arg(GraphName->currentText());
                QMessageBox::information ( NULL, "Graph", err, 
                             QMessageBox::Ok, QMessageBox::NoButton);
                return;
            }
            ++it;
        }
                
        GI = new GraphItem(GraphName->currentText(), GraphTab);
        Items.append(GI);
    }
}

void Graph::DeleteGraph(void)
{
    if (!GraphName->currentText().isEmpty())
    {
        QPtrListIterator<GraphItem> it( Items );
        GraphItem *GI;
        while ( (GI = it.current()) != 0 ) {
            if (GI->PlotName == GraphName->currentText())
            {
                Items.remove(GI);
            }
            ++it;
        }
    }
}

void Graph::CreatePlot(void)
{
    if (!PlotObject->currentText().isEmpty())
    {
        printf("Creating plot %s\n", PlotObject->currentText().latin1());
    }
}

void Graph::DeletePlot(void)
{
    if (!PlotObject->currentText().isEmpty())
    {
        printf("Deleting plot %s\n", PlotObject->currentText().latin1());
    }
}
