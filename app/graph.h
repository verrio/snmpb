#ifndef GRAPH_H
#define GRAPH_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qwt_plot.h>
//Added by qt3to4:
#include <QTimerEvent>
#include <q3combobox.h>
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
#if 0 //TODO
        QwtPlotCurve key;
#endif
        QString name;
        double data[PLOT_HISTORY];
    } curves[NUM_PLOT_PER_GRAPH];
};

class Graph: public QObject
{
    Q_OBJECT
    
public:
    Graph(QTabWidget* GT, QPushButton* GC, QPushButton* GD, 
          QComboBox* GN, QComboBox* PO, QPushButton* PA, 
          QPushButton* PD, QComboBox* PI, QComboBox* PC, 
          QComboBox* PS, QComboBox* PW, BasicMibView* PM);
    
public slots:
    void CreateGraph(void);
    void DeleteGraph(void);
    void CreatePlot(void);
    void DeletePlot(void);
    void SetObjectString(const QString& oid);
    
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
    BasicMibView* PlotMIBTree;
    
    QList<GraphItem*> Items;
};

#endif /* GRAPH_H */
