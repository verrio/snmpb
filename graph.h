#ifndef GRAPH_H
#define GRAPH_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qwt_plot.h>
#include "mibview.h"

#define NUM_PLOT_PER_GRAPH 10
#define PLOT_HISTORY 30

class GraphItem: public QwtPlot
{
public:
    GraphItem(QString name, QTabWidget* tab);
    ~GraphItem();
    
    void AddCurve(QString name, QPen& pen);
    void RemoveCurve(QString name);
    
protected:
    void timerEvent(QTimerEvent *);
    
private:
    QTabWidget* Tab;
    
    int dataCount;
    double timeData[PLOT_HISTORY];
    int timerID;
    
    struct
    {
        long key;
        QString name;
        double data[PLOT_HISTORY];
    } curves[NUM_PLOT_PER_GRAPH];
};

class GraphItemList: public QPtrList<GraphItem>
{
public:
    GraphItemList() { setAutoDelete(TRUE); }
    
protected:
    int compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 )
    {
	    return (strcmp(((GraphItem*)item1)->title().latin1(),
		        ((GraphItem*)item2)->title().latin1()));
    }
};

class Graph: public QObject
{
    Q_OBJECT
    
public:
    Graph(QTabWidget* GT, QPushButton* GC, QPushButton* GD, 
          QComboBox* GN, QComboBox* PO, QPushButton* PA, 
          QPushButton* PD, QComboBox* PI, QComboBox* PC, 
          QComboBox* PS, QComboBox* PW, MibView* PM);
    
public slots:
    void CreateGraph(void);
    void DeleteGraph(void);
    void CreatePlot(void);
    void DeletePlot(void);
    
private:
    QTabWidget* GraphTab;
    QPushButton* GraphCreate;
    QPushButton* GraphDelete;
    QComboBox* GraphName;
    QComboBox* PlotObject;
    QPushButton* PlotAdd;
    QPushButton* PlotDelete;
    QComboBox* PlotIndex;
    QComboBox* PlotColor;
    QComboBox* PlotShape;
    QComboBox* PlotWidth;
    MibView* PlotMIBTree;
    
    GraphItemList Items;
};

#endif /* GRAPH_H */
