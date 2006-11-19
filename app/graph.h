#ifndef GRAPH_H
#define GRAPH_H

#include <qwidget.h>
#include <qtabwidget.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <QTimerEvent>

#include "snmpb.h"
#include "mibview.h"
#include "comboboxes.h"

#define NUM_PLOT_PER_GRAPH 10
#define PLOT_HISTORY 30

class GraphItem: public QwtPlot
{
public:
    GraphItem(Snmpb *snmpb);
    ~GraphItem();
    
    void AddCurve(QString name, QPen& pen);
    void RemoveCurve(QString name);
    
protected:
    void timerEvent(QTimerEvent *);
    
private:
    Snmpb *s;
     
    int dataCount;
    double timeData[PLOT_HISTORY];
    int timerID;
    
    struct
    {
        QwtPlotCurve *object;
        double data[PLOT_HISTORY];
    } curves[NUM_PLOT_PER_GRAPH];
};

class Graph: public QObject
{
    Q_OBJECT
    
public:
    Graph(Snmpb *snmpb);
    
public slots:
    void CreateGraph(void);
    void DeleteGraph(void);
    void CreatePlot(void);
    void DeletePlot(void);
    void SetObjectString(const QString& oid);
    
private:
    Snmpb *s;

    BasicMibView* PlotMIBTree;
    QList<GraphItem*> Items;
};

#endif /* GRAPH_H */
