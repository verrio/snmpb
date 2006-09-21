#ifndef HISTOGRAM_H__
#define HISTOGRAM_H__

#include <qapplication.h>
#include <qframe.h>
#include <qwt_scale_map.h>
#include <qwt_plot.h>
#include <qwt_plot_interval.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_zoomer.h>
#include <qwt_legend.h>
#include <qlayout.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qlabel.h>

//------------------------------------------------------------
//      histogram example
//
//  This example program features many of the features 
//  of a QwtPlotInterval and QwtPlotBar.
//------------------------------------------------------------

class MainWin : public QFrame 
{
   Q_OBJECT

public:
   MainWin();

private:

  QwtPlot* Plot;
  QwtPlotGrid* PlotGrid;
  QwtPlotInterval* Interval;
  QwtLegend* Legend;
  QVBoxLayout* Layout;
  QwtPlotZoomer* Zoomer;
  QPushButton* Next;

public slots:

  void next();
};


#endif
