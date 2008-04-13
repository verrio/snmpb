/*
    Copyright (C) 2004-2008  Martin Jolicoeur (snmpb1@gmail.com) 

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
