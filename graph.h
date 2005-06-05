#ifndef GRAPH_H
#define GRAPH_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qwt_plot.h>

class GraphItem
{
public:
    GraphItem(QString name, QTabWidget* tab);
    ~GraphItem();
    QString PlotName;
    QwtPlot *PlotObj;
    QTabWidget* Tab;
};

class GraphItemList: public QPtrList<GraphItem>
{
public:
    GraphItemList() { setAutoDelete(TRUE); }
    
protected:
    int compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 )
    {
	    return (strcmp(((GraphItem*)item1)->PlotName.latin1(),
		        ((GraphItem*)item2)->PlotName.latin1()));
    }
};

class Graph: public QObject
{
    Q_OBJECT
    
public:
    Graph(QTabWidget* GT, QPushButton* GC, QPushButton* GD, 
          QComboBox* GN, QComboBox* PO, QPushButton* PA, 
          QPushButton* PD, QComboBox* PI, QComboBox* PC, 
          QComboBox* PS, QComboBox* PW);
    
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
    
    GraphItemList Items;
};

#endif /* GRAPH_H */
