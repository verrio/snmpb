#include <iostream>

#include <qwt_plot_canvas.h>
#include "histogram.h"

MainWin::MainWin()
{
  // create the frame layout
  Layout = new QVBoxLayout( this, 0, 0, "Layout");
  Plot = new QwtPlot( "Plot", this );

  // create default caption
  setCaption("Example Histogram");
  
  // place plot into layout
  Layout->addWidget(Plot);

  // create a zoomer
  Zoomer = new QwtPlotZoomer( QwtPlot::xBottom, 
                  QwtPlot::yLeft,
                  QwtPicker::DragSelection, 
                  QwtPicker::AlwaysOff, 
                  Plot->canvas());

  Zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));

  // Next button
  QHBoxLayout* bLayout = new QHBoxLayout(Layout, 0, "Buttons");
  Next = new QPushButton(QString("Next"), this);
  bLayout->add(Next);
  bLayout->addStretch();
  
  // hook up the button
  connect(Next,
      SIGNAL(clicked()),
      this,
      SLOT(next()));

  // make sure canvas is visible
  Plot->canvas()->show();

  QFont font( "Helvetica [Cronyx]" );
  Plot->setTitleFont(font);

  // Set the canvas background
  Plot->setCanvasBackground(QColor("black"));

  // TODO: add a legend

  // Show the axes
  Plot->setAxisFont(QwtPlot::xBottom,font);
  Plot->setAxisFont(QwtPlot::yLeft,font);
  Plot->setAxisTitle( Plot->xBottom, "Range");
  Plot->setAxisTitle( Plot->yLeft, "Height");
  Plot->setAxisTitleFont(QwtPlot::xBottom,font);
  Plot->setAxisTitleFont(QwtPlot::yLeft,font);
  Plot->enableXBottomAxis(true);
  Plot->enableYLeftAxis(true);
  
  // set margin
  Plot->setMargin(10);
   
  // create a grid for the plot
  PlotGrid = new QwtPlotGrid();

  //  set grid look
  QPen loGridPen(Qt::gray, 0, Qt::DotLine);
  PlotGrid->setMajPen(loGridPen);

  //  attach to plot
  PlotGrid->attach(Plot);

  // create a QwtPlotInterval to display
  Interval = new QwtPlotInterval("Histogram 1");

  // add some data
  QwtArray<QwtDoubleInterval> Intervals(20);
  QwtArray<double> YVals(20);

  //  create some simple data
  for (int i = 0; i < 20; ++i)
  {
    Intervals[i] = QwtDoubleInterval((i*3),(i*3)+3);
    YVals[i] = (double)i;
  }

  // set the data
  Interval->setData(Intervals,
            YVals);
  
  // set initial interval settings
  // pick a color for the bars
  QColor Color(Qt::blue);
  Interval->setStyle(QwtPlotInterval::Bar);
  Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
  Interval->setPen(QPen(Color));
  Interval->setFill(QBrush(Color));
  Interval->setWidthScale(15);  // percent to reduce bar width

  // draw interval in plot
  Interval->attach(Plot);

  // active the zoomer
  Zoomer->setZoomBase(Interval->boundingRect());
  Zoomer->zoom(Interval->boundingRect());
  Zoomer->setEnabled(true);

#if QT_VERSION >= 0x040000
    // The dot lines make Qt4 to send the bounding rect of
    // the rubberband as update region to the canvas. So we
    // need to suppress flickering.

    Plot->canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, true);
#endif

  // make the window bigger
  resize( QSize(600, 480) );

  // get on with the...
  show();
}

void MainWin::next()
{
  static int step = 1;
  QColor Color;

  switch (step) 
  {
  case 1:    
    Color.setNamedColor("red");
    Interval->setStyle(QwtPlotInterval::Bar3D);
    Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense4Pattern));
    Interval->setWidthScale(30);  // percent to reduce bar width
    break;

  case 2:
    Color.setNamedColor("green");
    Interval->setStyle(QwtPlotInterval::Step);
    Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense5Pattern));    
    break;

  case 3:
    Color.setNamedColor("yellow");
    Interval->setStyle(QwtPlotInterval::Tiled);
    Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense6Pattern));
    Interval->setWidthScale(50);  // percent to reduce bar width
    break;

  case 4:
    Color.setNamedColor("cyan");
    Interval->setStyle(QwtPlotInterval::Bar);
    Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setBaseline(5);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense7Pattern));
    Interval->setWidthScale(0);  // percent to reduce bar width
    break;

  case 5:    
    Color.setNamedColor("magenta");
    Interval->setStyle(QwtPlotInterval::Bar3D);
    Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
    Interval->setLabel(QwtPlotInterval::Height);
    Interval->setBaseline(10);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense2Pattern));
    break;

  case 6:
    Color.setNamedColor("gray");
    Interval->setStyle(QwtPlotInterval::Step);
    Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
    Interval->setLabel(QwtPlotInterval::Range);
    Interval->setBaseline(15);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color));
    break;

  case 7:
    Color.setNamedColor("white");
    Interval->setStyle(QwtPlotInterval::Tiled);
    Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setBaseline(19);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense7Pattern));
    break;

  case 8:
    Color.setNamedColor("green");
    Interval->setStyle(QwtPlotInterval::Bar3D);
    Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setBaseline(20);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense4Pattern));
    break;

  case 9:
    Color.setNamedColor("blue");
    Interval->setStyle(QwtPlotInterval::Bar);
    Interval->setIntervalAttribute(QwtPlotInterval::Xfy);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setBaseline(0);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color));
    Interval->setWidthScale(75);  // percent to reduce bar width
    break;

  case 10:    
    Color.setNamedColor("red");
    Interval->setStyle(QwtPlotInterval::Bar3D);
    Interval->setIntervalAttribute(QwtPlotInterval::Xfy);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense4Pattern));
    Interval->setWidthScale(30);  // percent to reduce bar width
    break;

  case 11:
    Color.setNamedColor("green");
    Interval->setStyle(QwtPlotInterval::Step);
    Interval->setIntervalAttribute(QwtPlotInterval::Xfy);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense5Pattern));    
    break;

  case 12:
    Color.setNamedColor("yellow");
    Interval->setStyle(QwtPlotInterval::Tiled);
    Interval->setIntervalAttribute(QwtPlotInterval::Xfy);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense6Pattern));
    break;

  case 13:
    Color.setNamedColor("cyan");
    Interval->setStyle(QwtPlotInterval::Bar);
    Interval->setIntervalAttribute(QwtPlotInterval::Xfy);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setBaseline(5);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense7Pattern));
    break;

  case 14:    
    Color.setNamedColor("magenta");
    Interval->setStyle(QwtPlotInterval::Bar3D);
    Interval->setIntervalAttribute(QwtPlotInterval::Xfy);
    Interval->setLabel(QwtPlotInterval::Height);
    Interval->setBaseline(10);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense2Pattern));
    break;

  case 15:
    Color.setNamedColor("gray");
    Interval->setStyle(QwtPlotInterval::Step);
    Interval->setIntervalAttribute(QwtPlotInterval::Xfy);
    Interval->setLabel(QwtPlotInterval::Range);
    Interval->setBaseline(15);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color));
    break;

  case 16:
    Color.setNamedColor("white");
    Interval->setStyle(QwtPlotInterval::Tiled);
    Interval->setIntervalAttribute(QwtPlotInterval::Xfy);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setBaseline(19);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense7Pattern));
    break;

  case 17:
    Color.setNamedColor("green");
    Interval->setStyle(QwtPlotInterval::Bar3D);
    Interval->setIntervalAttribute(QwtPlotInterval::Xfy);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setBaseline(20);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color,Qt::Dense4Pattern));
    break;

  default:
    Color.setNamedColor("blue");
    Interval->setStyle(QwtPlotInterval::Bar);
    Interval->setIntervalAttribute(QwtPlotInterval::Yfx);
    Interval->setLabel(QwtPlotInterval::IntHeight);
    Interval->setBaseline(0);
    Interval->setPen(QPen(Color));
    Interval->setFill(QBrush(Color));

    step = 0;
  }

  step++;
  Plot->replot();
}

int main (int argc, char **argv)
{
    QApplication a(argc, argv);

    MainWin w;

    a.setMainWidget(&w);
    w.show();

    return a.exec();
}

