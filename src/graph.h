/**
 * Copyright (c) 2004-2017 Martin Jolicoeur and contributors
 *
 * This file is part of the snmpb project and is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <qwidget.h>
#include <qtabwidget.h>
#include <qwt6/qwt_plot.h>
#include <qwt6/qwt_plot_curve.h>
#include <QTimerEvent>
#include <qlistwidget.h>
#include <qlist.h>
#include <qsettings.h>

#include "snmpb.h"
#include "mibview.h"
#include "comboboxes.h"
#include "ui_plot.h"

#define NUM_PLOT_PER_GRAPH 10
#define PLOT_HISTORY 30

typedef struct
{
    QString name;
    QString agentname;
    int proto;
    int color;
    int width;
    int shape;
    QString oid;
    QListWidgetItem *item;
    QwtPlotCurve *object;
    double data[PLOT_HISTORY];
} Plot;

class Graph: public QwtPlot
{
    Q_OBJECT
    
public:
    Graph(Snmpb *snmpb, QString *n = NULL);
    ~Graph();

    int SelectGraph(QListWidgetItem *i);
    QListWidgetItem *GetWidgetItem(void);

    void SetName(QString n);
    QString GetName(void);
    void SetGraphName(void);
    void SetPollInterval(void);
    void SetPollInterval(int val);
    int GetPollInterval(void);
    void ReadPlotConfig(QSettings* settings);
    void WritePlotConfig(QSettings* settings);

    void AddCurve(QString name, QPen& pen);
    void RemoveCurve(QString name);
    Plot* AddPlot(QString *n = NULL);
    void DeletePlot(QListWidgetItem *i);
    Plot* FindPlot(QListWidgetItem *i);

protected:
    void timerEvent(QTimerEvent *);

private:
    Snmpb *s;

    QListWidgetItem *item;

    QString name;
    int poll_interval;

    int dataCount;
    double timeData[PLOT_HISTORY];
    int timerID;

// curves[NUM_PLOT_PER_GRAPH];
//    BasicMibView* PlotMIBTree;

    QList<Plot *> plots;
};

class GraphManager: public QObject
{
    Q_OBJECT

public:
    GraphManager(Snmpb *snmpb);

protected slots:
    void Add(void);
    void Delete(void);
    void SetGraphName(void);
    void SetPollInterval(void);
    void SelectedGraph(QListWidgetItem * item, QListWidgetItem * old);
    void GraphNameChange(QListWidgetItem * item);

    void AddPlot(void);
    void DeletePlot(void);
    void EditPlot(QListWidgetItem * item);

    void AgentProfileListChange(void);

    void SelectAgentProto(void);
    void SelectAgentProfile(QString *prefprofile = NULL, int prefproto = -1);
    void ShowAgentSettings(void);

    void SetPlotOID(void);

private:
    void ReadConfigFile(void);
    void WriteConfigFile(void);

private:
    bool v1, v2, v3;
    Snmpb *s;
    Ui_Plot p;
    QDialog pw;

    QSettings *settings;
    Graph* currentgraph;
    QList<Graph *> graphs;
};

#endif /* GRAPH_H */

