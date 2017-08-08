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

#include <qmessagebox.h>
#include <qnamespace.h>
#include <qwt6/qwt_plot_zoomer.h>
#include <qwt6/qwt_plot_layout.h>

#include "preferences.h"
#include "graph.h"
#include "agent.h"
#include "comboboxes.h"

#define ui s->MainUI()

GraphManager::GraphManager(Snmpb *snmpb)
{
    s = snmpb;
    p.setupUi(&pw);

    settings = new QSettings(s->GetGraphsConfigFile(), QSettings::IniFormat, this);

     // Set some properties for the ListView
    ui->GraphList->setSortingEnabled( 0 );
    ui->GraphList->setLineWidth( 2 );
    ui->GraphList->setFrameShape(QFrame::WinPanel);
    ui->GraphList->setFrameShadow(QFrame::Plain);

    ui->PlotList->setSortingEnabled( 0 );
    ui->PlotList->setLineWidth( 2 );
    ui->PlotList->setFrameShape(QFrame::WinPanel);
    ui->PlotList->setFrameShadow(QFrame::Plain);

    connect( ui->GraphList, 
             SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem * ) ),
             this, SLOT( SelectedGraph( QListWidgetItem *, QListWidgetItem * ) ) );
    connect( ui->GraphList, 
             SIGNAL( itemChanged( QListWidgetItem * ) ),
             this, SLOT( GraphNameChange( QListWidgetItem * ) ) );
    connect( ui->GraphName, SIGNAL( editingFinished() ),
             this, SLOT ( SetGraphName() ) );
    connect( ui->GraphAdd, SIGNAL( clicked() ), 
             this, SLOT( Add() ));
    connect( ui->GraphDelete, SIGNAL( clicked() ), 
             this, SLOT( Delete() ));
    connect( ui->PlotList, 
             SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
             this, SLOT( EditPlot( QListWidgetItem * ) ) );
    connect( ui->PlotAdd, SIGNAL( clicked() ), 
             this, SLOT( AddPlot() ));
    connect( ui->PlotDelete, SIGNAL( clicked() ), 
             this, SLOT( DeletePlot() ));    
    connect( p.PlotBrowseOID, SIGNAL( clicked() ), 
             this, SLOT( SetPlotOID() ));
    connect( s->APManagerObj(), SIGNAL( AgentProfileListChanged() ), 
             this, SLOT ( AgentProfileListChange() ) );
    connect( ui->GraphPollInterval, SIGNAL( valueChanged( int ) ),
             this, SLOT ( SetPollInterval() ) );

    // Select the default profile from preferences
    QString cp;
    int prefproto = s->PreferencesObj()->GetCurrentProfile(cp);
    // Fill-in the list of agent profiles from profiles manager
    AgentProfileListChange();
    SelectAgentProfile(&cp, prefproto);

    // then connect the signals (order is important)
    connect( p.PlotAgentProfile, SIGNAL( currentIndexChanged( int ) ), 
             this, SLOT ( SelectAgentProfile() ) );
    connect( p.PlotAgentProtoV1, SIGNAL( toggled(bool) ),
             this, SLOT( SelectAgentProto() ) );
    connect( p.PlotAgentProtoV2, SIGNAL( toggled(bool) ),
             this, SLOT( SelectAgentProto() ) );
    connect( p.PlotAgentProtoV3, SIGNAL( toggled(bool) ),
             this, SLOT( SelectAgentProto() ) );
    connect( p.PlotAgentSettings, 
             SIGNAL( clicked() ), this, SLOT( ShowAgentSettings() ));

    // Loop & load all stored graphs
    currentgraph = NULL;
    ReadConfigFile();

    if (graphs.size() != 0)
        ui->GraphList->setCurrentItem(ui->GraphList->item(0));
}

void GraphManager::ReadConfigFile (void)
{
    int size = settings->beginReadArray("graphs");
    for (int i = 0; i < size; i++)
    {
        settings->setArrayIndex(i);
        QString _name = settings->value("name").toString();
        Graph *newgraph = new Graph(s, &_name);
        
        newgraph->SetPollInterval(settings->value("pollinterval").toInt());
        newgraph->ReadPlotConfig(settings);

        graphs.append(newgraph);
    }
    settings->endArray();
}

void GraphManager::WriteConfigFile (void)
{
    settings->beginWriteArray("graphs");
    settings->remove("");
    for (int i = 0; i < graphs.size(); i++)
    {
        settings->setArrayIndex(i);
        settings->setValue("name", graphs[i]->GetName());
        settings->setValue("pollinterval", graphs[i]->GetPollInterval());
        graphs[i]->WritePlotConfig(settings);

    }
    settings->endArray();
}

void GraphManager::SetGraphName(void)
{
    if (currentgraph)
    {
        currentgraph->SetGraphName();
        WriteConfigFile();
    }
}

void GraphManager::SetPollInterval(void)
{
    if (currentgraph)
    {
        currentgraph->SetPollInterval();
        WriteConfigFile();
    }
}

void GraphManager::Add(void)
{
    Graph *newgraph = new Graph(s);

    // Set default values
//    newuser->SetSecurity(0, "", 0, ""); MART
    ui->Graph->setEnabled(true);
 
    graphs.append(newgraph);

    // Select the new item and change the focus to change its name ...
    ui->GraphList->setCurrentItem(newgraph->GetWidgetItem());
    ui->GraphName->setFocus(Qt::OtherFocusReason);  
    ui->GraphName->selectAll();

    WriteConfigFile();
}

void GraphManager::Delete(void)
{
    QListWidgetItem *p = NULL;

    for (int i = 0; i < graphs.count(); i++) 
    {
        if (currentgraph && (graphs[i] == currentgraph))
        {
            // Delete the graph (removes from the list)
            delete graphs.takeAt(i);
            delete ui->GraphList->takeItem(i);

            currentgraph = NULL;
            // Re-adjust the currentgraph pointer with the new current widget item
            if ((p = ui->GraphList->currentItem()) != NULL)
            {
                for (int i = 0; i < graphs.count(); i++) 
                {
                    if (p == graphs[i]->GetWidgetItem())
                    {
                        currentgraph = graphs[i];
                        break;
                    }
                }
            }
            else
                ui->Graph->setEnabled(false);

            break;
        }
    }
}

void GraphManager::SelectedGraph(QListWidgetItem * item, QListWidgetItem *)
{
    for (int i = 0; i < graphs.count(); i++) 
    {
        if (item == graphs[i]->GetWidgetItem())
        {
            currentgraph = graphs[i];
            graphs[i]->SelectGraph(item);
            return;
        }
    }
}

void GraphManager::GraphNameChange(QListWidgetItem * item)
{
    for (int i = 0; i < graphs.count(); i++) 
    {
        if (item == graphs[i]->GetWidgetItem())
        {
            graphs[i]->SetName(item->text());
            return;
        }
    }
}

void GraphManager::AddPlot(void)
{
    if (currentgraph)
    {
#if 0
        if (!p.PlotObject->text().isEmpty())
        {
            // Create the pen with the combobox values
            QPen pen(p.PlotColor->itemData(
                        p.PlotColor->currentIndex(), 
                        Qt::DisplayRole).value<QColor>(),
                    p.PlotWidth->itemData(
                        p.PlotWidth->currentIndex(), 
                        Qt::DisplayRole).toUInt(),
                    (enum Qt::PenStyle)(p.PlotShape->itemData(
                            p.PlotShape->currentIndex(), 
                            Qt::DisplayRole).toUInt()));
#ifdef NOTYET  
            printf("Creating plot %s\n", ui->PlotObject->currentText().toUtf8().data());
#endif 
            if (!ui->GraphName->text().isEmpty())
            {
                Graph *G = NULL;
                for (int i = 0; i < graphs.count(); i++)
                {
                    G = graphs[i];
                    if (G->title().text() == ui->GraphName->text())
                        break;
                }
                if (G)
                    G->AddCurve(p.PlotObject->text(), pen);
            }
        }
#endif
        QString cp;
        int prefproto = s->PreferencesObj()->GetCurrentProfile(cp);
        SelectAgentProfile(&cp, prefproto);
        p.PlotColor->setCurrentIndex(0);
        p.PlotWidth->setCurrentIndex(0);
        p.PlotShape->setCurrentIndex(0);
        p.PlotObject->setText("");

        if (pw.exec()  == QDialog::Accepted)
        {
            QString plotname = p.PlotAgentProfile->currentText()+": "+p.PlotObject->text();
            Plot *plot = currentgraph->AddPlot(&plotname);

            plot->agentname = p.PlotAgentProfile->currentText();
            if (p.PlotAgentProtoV1->isChecked()) plot->proto = 0;
            else if (p.PlotAgentProtoV2->isChecked()) plot->proto = 1;
            else if (p.PlotAgentProtoV3->isChecked()) plot->proto = 2;
            plot->color = p.PlotColor->currentIndex();
            plot->width = p.PlotWidth->currentIndex();
            plot->shape = p.PlotShape->currentIndex();
            plot->oid = p.PlotObject->text();

            plot->item->setText(plot->name.toUtf8().data());

            WriteConfigFile();
        }
    }
}

void GraphManager::DeletePlot(void)
{
    if (currentgraph)
        currentgraph->DeletePlot(ui->PlotList->currentItem());

#if 0
    if (!str.isEmpty())
    {
#ifdef NOTYET
        printf("Deleting plot %s\n", ui->PlotObject->currentText().toUtf8().data());
#endif 
        if (!ui->GraphName->text().isEmpty())
        {
            Graph *G = NULL;
            for (int i = 0; i < graphs.count(); i++)
            {
                G = graphs[i];
                if (G->title().text() == ui->GraphName->text())
                    break;
            }
            if (G) 
                G->RemoveCurve(str);
        }
    }
#endif
}

void GraphManager::EditPlot(QListWidgetItem * item)
{
    if (currentgraph)
    {
        Plot *plot = currentgraph->FindPlot(item);

        SelectAgentProfile(&plot->agentname, plot->proto);
        p.PlotColor->setCurrentIndex(plot->color);
        p.PlotWidth->setCurrentIndex(plot->width);
        p.PlotShape->setCurrentIndex(plot->shape);
        p.PlotObject->setText(plot->oid);

        if (pw.exec()  == QDialog::Accepted)
        {
            plot->name = p.PlotAgentProfile->currentText()+": "+p.PlotObject->text();
            plot->agentname = p.PlotAgentProfile->currentText();
            if (p.PlotAgentProtoV1->isChecked()) plot->proto = 0;
            else if (p.PlotAgentProtoV2->isChecked()) plot->proto = 1;
            else if (p.PlotAgentProtoV3->isChecked()) plot->proto = 2;
            plot->color = p.PlotColor->currentIndex();
            plot->width = p.PlotWidth->currentIndex();
            plot->shape = p.PlotShape->currentIndex();
            plot->oid = p.PlotObject->text();

            plot->item->setText(plot->name.toUtf8().data());

            WriteConfigFile();
        }
    }
}

void GraphManager::SetPlotOID(void)
{
    // Create and run the mib selection dialog
    MibSelection ms(s, ui->Graph, "Browse OID", MIBSELECTION_TYPE);

    if (ms.run())
    {
        p.PlotObject->setText(ms.GetOid()+
                              (ms.GetName().isEmpty()?"":" ("+ms.GetName()+")"));
    }
}

void GraphManager::AgentProfileListChange(void)
{
    int prefproto = -1;
    if (p.PlotAgentProtoV1->isChecked()) prefproto = 0;
    else if (p.PlotAgentProtoV2->isChecked()) prefproto = 1;
    else if (p.PlotAgentProtoV3->isChecked()) prefproto = 2;

    QString cap = ui->AgentProfile->currentText();
    p.PlotAgentProfile->clear();
    p.PlotAgentProfile->addItems(s->APManagerObj()->GetAgentsList());
    if (cap.isEmpty() == false)
    {
        int idx = p.PlotAgentProfile->findText(cap);
        p.PlotAgentProfile->setCurrentIndex(idx>0?idx:0);
        if (idx < 0) prefproto = -1;
    }
    else
        prefproto = -1;

    SelectAgentProfile(NULL, prefproto);
}

void GraphManager::SelectAgentProto(void)
{
    int prefproto = -1;
    if (p.PlotAgentProtoV1->isChecked()) prefproto = 0;
    else if (p.PlotAgentProtoV2->isChecked()) prefproto = 1;
    else if (p.PlotAgentProtoV3->isChecked()) prefproto = 2;

//    ui->MIBTree->SetCurrentAgentIsV1(prefproto==0?true:false);

    SelectAgentProfile(NULL, prefproto);
}

void GraphManager::SelectAgentProfile(QString *prefprofile, int prefproto)
{
    AgentProfile *ap = s->APManagerObj()->GetAgentProfile
                        (prefprofile?prefprofile->toUtf8():
                         p.PlotAgentProfile->currentText());
    if (ap)
    {
        bool v1,v2,v3;
        int selectedproto = -1;
        ap->GetSupportedProtocol(&v1, &v2, &v3);

        p.PlotAgentProtoV1->setEnabled(v1);
        p.PlotAgentProtoV2->setEnabled(v2);
        p.PlotAgentProtoV3->setEnabled(v3);

        if ((prefproto == 0) && v1)
        {
            p.PlotAgentProtoV1->setChecked(true);
            selectedproto = 0;
        }
        else 
        if ((prefproto == 1) && v2)
        {
            p.PlotAgentProtoV2->setChecked(true);
            selectedproto = 1;
        }
        else
        if ((prefproto == 2) && v3)
        {
            p.PlotAgentProtoV3->setChecked(true);
            selectedproto = 2;
        }
        else
        if (v1)
        {
            p.PlotAgentProtoV1->setChecked(true);
            selectedproto = 0;
        }
        else
        if (v2)
        {
            p.PlotAgentProtoV2->setChecked(true);
            selectedproto = 1;
        }
        else
        if (v3)
        {
            p.PlotAgentProtoV3->setChecked(true);
            selectedproto = 2;
        }

        if (prefprofile)
        {
            // update the combobox
            int index = p.PlotAgentProfile->findText(prefprofile->toUtf8());
            p.PlotAgentProfile->setCurrentIndex(index);
        }
    }
    else
    {
        p.PlotAgentProtoV1->setEnabled(false);
        p.PlotAgentProtoV2->setEnabled(false);
        p.PlotAgentProtoV3->setEnabled(false);
    }
}

void GraphManager::ShowAgentSettings(void)
{
     s->APManagerObj()->SetSelectedAgent(p.PlotAgentProfile->currentText()); 
     s->APManagerObj()->Execute();
}

class tracker: public QwtPlotZoomer
{
public:
    tracker(QWidget *canvas):
        QwtPlotZoomer(canvas)
    {
        setTrackerMode(AlwaysOn);
    }

    virtual QwtText trackerText(const QPoint &pos) const
    {
        QColor bg(Qt::white);
        bg.setAlpha(200);

        QwtText text = QwtPlotZoomer::trackerText(pos);
        text.setBackgroundBrush( QBrush( bg ));
        return text;
    }
};

Graph::Graph(Snmpb *snmpb, QString *n): QwtPlot(n?*n:"")
{
    s = snmpb;

    item = new QListWidgetItem(ui->GraphList);

    if (n)
    {
        item->setText(n->toUtf8().data());
        SetName(*n);
    }
    else
    {
        item->setText("newgraph");
        SetName("newgraph");
        setTitle("newgraph");
    }

    ui->GraphTab->addTab(this, GetName());
    dataCount = 0;
    timerID = 0;

    new tracker(canvas());
    
    for ( int i = 0; i < PLOT_HISTORY; i++ )
        timeData[i] = i;

#if 0
    // Zero all curve structures
    for( int j = 0; j < NUM_PLOT_PER_GRAPH; j++)
    {
        curves[j].object = NULL;
        memset(curves[j].data, 0, sizeof(double)*PLOT_HISTORY);
    }
#endif
}

Graph::~Graph()
{
#if 0
    // Free curve objects
    for( int j = 0; j < NUM_PLOT_PER_GRAPH; j++)
        if (curves[j].object) delete curves[j].object;
#endif

#if 0 // crashes the app 
    if (ui->GraphTab && (ui->GraphTab->indexOf(this) != -1))
        ui->GraphTab->removeTab(ui->GraphTab->indexOf(this));
#endif
}

void Graph::ReadPlotConfig(QSettings* settings)
{
    for(int i = 0;;i++) 
    {
        char buf[10];
        sprintf(buf, "%d", i);
        settings->beginGroup(buf);

        if (settings->value("name", "").toString() == "")
        {
            settings->endGroup();
            break;
        }

        QString n = settings->value("name").toString();
        Plot *_p = AddPlot(&n);

        _p->agentname = settings->value("agent").toString();
        _p->proto = settings->value("proto").toInt();
        _p->color = settings->value("color").toInt();
        _p->width = settings->value("width").toInt();
        _p->shape = settings->value("shape").toInt();
        _p->oid = settings->value("oid").toString();

        settings->endGroup();
    }
}

void Graph::WritePlotConfig(QSettings* settings)
{
    for (int i = 0; i < plots.count(); i++) 
    {
        char buf[10];
        sprintf(buf, "%d", i);
        settings->beginGroup(buf);
        settings->setValue("name", plots[i]->name);
        settings->setValue("agent", plots[i]->agentname);
        settings->setValue("proto", plots[i]->proto);
        settings->setValue("color", plots[i]->color);
        settings->setValue("width", plots[i]->width);
        settings->setValue("shape", plots[i]->shape);
        settings->setValue("oid", plots[i]->oid);
        settings->endGroup();
    }
}

void Graph::AddCurve(QString name, QPen& pen)
{
#if 0
    int i = 0;
    
    for (i = 0; i < NUM_PLOT_PER_GRAPH; i++)
    {
        if (curves[i].object && (curves[i].object->title().text() == name))
            return;
        else if (!curves[i].object)
            break;
    }
    
    if (i >= NUM_PLOT_PER_GRAPH)
        return;

    curves[i].object = new QwtPlotCurve(name);
    curves[i].object->attach(this);
    curves[i].object->setPen(pen);

    if (!timerID)
        timerID = startTimer(1000); // 1 second
    
    replot();
#endif
}

void Graph::RemoveCurve(QString name)
{
#if 0
    /* No other curve left, kill the timer first ... */
    if (timerID && ((/*TODO*/1-1) == 0))
    {
        killTimer(timerID);
        timerID = 0;
    }

    for (int i = 0; i < NUM_PLOT_PER_GRAPH; i++)
    {
        if (curves[i].object && (curves[i].object->title().text() == name))
        {
            delete(curves[i].object);
            curves[i].object = NULL;
            return;
        }
        else if (!curves[i].object)
            return;
    }
#endif
}

void Graph::timerEvent(QTimerEvent *)
{
#if 0
    if ( dataCount < PLOT_HISTORY )
    {
        dataCount++;
    }
    else
    {
        /* Time shift of 1 sec */
        for ( int j = 0; j < PLOT_HISTORY; j++ )
            timeData[j]++;
        
        for ( int i = 0; i < PLOT_HISTORY - 1; i++ )
        {
            for ( int c = 0; c < 1; c++ )
            {
                curves[c].data[i] = curves[c].data[i+1];
            }
        }    
    }
    
    /* Set the data */
    curves[0].data[dataCount-1] = 
        s->AgentObj()->GetSyncValue(curves[0].object->title().text());
    
    setAxisScale(QwtPlot::xBottom, timeData[0], timeData[PLOT_HISTORY - 1]);
    
    for ( int c = 0; c < 1/* TODO */; c++ )
    {
        if (curves[c].object)
            curves[c].object->setRawSamples(timeData, curves[c].data, dataCount);
    }

    replot();
#endif
}

int Graph::SelectGraph(QListWidgetItem * i)
{
    if (i == item)
    {
        ui->Graph->setEnabled(true);

        ui->GraphName->setText(name);
        ui->GraphPollInterval->setValue(poll_interval);

        // Unlink all elements from list
        while (ui->PlotList->takeItem(0));

        for (int i = plots.count()-1; i >= 0; i--)
            ui->PlotList->insertItem(0, plots[i]->item);
 
        return 1;
    }

    return 0;
}

QListWidgetItem *Graph::GetWidgetItem(void)
{
    return item;
}

void Graph::SetName(QString n)
{
    name = n;
    ui->GraphName->setText(name);
}

QString Graph::GetName(void)
{
    return name;
}

void Graph::SetGraphName(void)
{
    name = ui->GraphName->text();
    item->setText(name);

    setTitle(name);
    ui->GraphTab->setTabText(ui->GraphTab->indexOf(this), name);
}

void Graph::SetPollInterval(void)
{
    poll_interval = ui->GraphPollInterval->value();
}

void Graph::SetPollInterval(int val)
{
    poll_interval = val;
}

int Graph::GetPollInterval(void)
{
    return poll_interval;
}

Plot* Graph::AddPlot(QString *n)
{
    Plot *_p = new Plot;
    plots.append(_p);
    _p->item = new QListWidgetItem(ui->PlotList);

    ui->PlotList->setCurrentItem(_p->item);

    if (n)
    {
        _p->item->setText(n->toUtf8().data());
        _p->name = *n;
    }
    else
    {
        _p->item->setText("newplot");
        _p->name = "newplot";
    }

    return _p;
}

void Graph::DeletePlot(QListWidgetItem *item)
{
    for (int i = 0; i < plots.count(); i++) 
    {
        if (item == plots[i]->item)
        {
            // Delete the plot (removes from the list)
            delete plots.takeAt(i);
            delete ui->PlotList->takeItem(i);

            break;
        }
    }
}

Plot* Graph::FindPlot(QListWidgetItem *item)
{
    for (int i = 0; i < plots.count(); i++) 
        if (item == plots[i]->item)
            return plots[i];

    return NULL;
}
