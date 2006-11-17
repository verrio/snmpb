#include <qmessagebox.h>
#include <qnamespace.h>

#include "graph.h"
#include "agent.h"
#include "comboboxes.h"

GraphItem::GraphItem(QString name, QTabWidget* tab):QwtPlot(name)
{
    Tab = tab;
    Tab->addTab(this, name);
    dataCount = 0;
    timerID = 0;
    
    for ( int i = 0; i < PLOT_HISTORY; i++ )
        timeData[i] = i;

    // Zero all curve structures
    for( int j = 0; j < NUM_PLOT_PER_GRAPH; j++)
    {
        curves[j].object = NULL;
        memset(curves[j].data, 0, sizeof(double)*PLOT_HISTORY);
    }
}

GraphItem::~GraphItem()
{
    // Free curve objects
    for( int j = 0; j < NUM_PLOT_PER_GRAPH; j++)
        if (curves[j].object) delete curves[j].object;

    if (Tab && (Tab->indexOf(this) != -1))
        Tab->removeTab(Tab->indexOf(this));
}

void GraphItem::AddCurve(QString name, QPen& pen)
{
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
}

void GraphItem::RemoveCurve(QString name)
{
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
}

void GraphItem::timerEvent(QTimerEvent *)
{
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
        CurrentAgent->GetSyncValue(curves[0].object->title().text());
    
    setAxisScale(QwtPlot::xBottom, timeData[0], timeData[PLOT_HISTORY - 1]);
    
    for ( int c = 0; c < 1/* TODO */; c++ )
    {
        curves[c].object->setRawData(timeData, curves[c].data, dataCount);
    }

    replot();
}

Graph::Graph(QTabWidget* GT, QPushButton* GC, QPushButton* GD,
             QComboBox* GN, QComboBox* PO, QPushButton* PA,
             QPushButton* PD, QComboBox* PI, QComboBox* PC,
             QComboBox* PS, QComboBox* PW, BasicMibView* PM)
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
    PlotMIBTree = PM;
    
    // Connect some signals
    connect( GC, SIGNAL( clicked() ), this, SLOT( CreateGraph() ));
    connect( GD, SIGNAL( clicked() ), this, SLOT( DeleteGraph() ));
    connect( PA, SIGNAL( clicked() ), this, SLOT( CreatePlot() ));
    connect( PD, SIGNAL( clicked() ), this, SLOT( DeletePlot() ));    
    connect( PM, SIGNAL( SelectedOid(const QString&) ), 
             this, SLOT( SetObjectString(const QString&) ));
}

void Graph::CreateGraph(void)
{
    if (!GraphName->currentText().isEmpty())
    {        
        GraphItem *GI;
        for (int i = 0; i < Items.count(); i++)
        {
            GI = Items[i];
            if (GI->title().text() == GraphName->currentText())
            {
                QString err = QString("Graph \"%1\" already exist !")
                      .arg(GraphName->currentText());
                QMessageBox::information ( NULL, "Graph", err, 
                             QMessageBox::Ok, Qt::NoButton);
                return;
            }
        }
                
        GI = new GraphItem(GraphName->currentText(), GraphTab);
        Items.append(GI);
    }
}

void Graph::DeleteGraph(void)
{
    if (!GraphName->currentText().isEmpty())
    {
        GraphItem *GI;
        for (int i = 0; i < Items.count(); i++)
        {
            GI = Items[i];
            if (GI->title().text() == GraphName->currentText())
            {
                Items.removeAll(GI);
                delete GI;
                return;
            }
        }
    }
}

void Graph::CreatePlot(void)
{
    if (!PlotObject->currentText().isEmpty())
    {
        // Create the pen with the combobox values
        QPen p(PlotColor->itemData(PlotColor->currentIndex(), 
                                   Qt::DisplayRole).value<QColor>(),
               PlotWidth->itemData(PlotWidth->currentIndex(), 
                                   Qt::DisplayRole).toUInt(),
               (enum Qt::PenStyle)(PlotShape->itemData(PlotShape->currentIndex(), 
                                                       Qt::DisplayRole).toUInt()));
    
        printf("Creating plot %s\n", PlotObject->currentText().toLatin1().data());
        
        if (!GraphName->currentText().isEmpty())
        {
            GraphItem *GI = NULL;
            for (int i = 0; i < Items.count(); i++)
            {
                GI = Items[i];
                if (GI->title().text() == GraphName->currentText())
                    break;
            }
            if (GI)
                GI->AddCurve(PlotObject->currentText(), p);
        }
    }
}

void Graph::DeletePlot(void)
{
    if (!PlotObject->currentText().isEmpty())
    {
        printf("Deleting plot %s\n", PlotObject->currentText().toLatin1().data());
        
        if (!GraphName->currentText().isEmpty())
        {
            GraphItem *GI = NULL;
            for (int i = 0; i < Items.count(); i++)
            {
                GI = Items[i];
                if (GI->title().text() == GraphName->currentText())
                    break;
            }
            if (GI) 
                GI->RemoveCurve(PlotObject->currentText());
        }
    }
}

void Graph::SetObjectString(const QString& oid)
{
    PlotObject->insertItem(0, oid);
    PlotObject->setCurrentIndex(0);
}

