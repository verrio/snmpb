/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#if defined(_MSC_VER) /* MSVC Compiler */
#pragma warning ( disable : 4786 )
#endif

#include <qglobal.h>
#include <qaction.h>
#include <QtPlugin>
#include <QDesignerFormEditorInterface>
#include <QDesignerFormWindowInterface>
#include <QDesignerFormWindowCursorInterface>
#include <QExtensionManager>
#if 1
#include <QErrorMessage>
#endif

#include "qwt_designer_plotdialog.h"
#include "qwt_designer_plugin.h"
#include "qwt_counter.h"
#include "qwt_plot.h"
#include "qwt_wheel.h"
#include "qwt_thermo.h"
#include "qwt_knob.h"
#if 0
#include "qwt_scale_widget.h"
#endif
#include "qwt_scale_engine.h"
#include "qwt_slider.h"
#include "qwt_text_label.h"
#include "qwt_analog_clock.h"
#include "qwt_compass.h"

using namespace QwtDesignerPlugin;

CustomWidgetInterface::CustomWidgetInterface(QObject *parent): 
    QObject(parent),
    d_isInitialized(false)
{
}

bool CustomWidgetInterface::isContainer() const
{
    return false;
}

bool CustomWidgetInterface::isInitialized() const
{
    return d_isInitialized;
}

QIcon CustomWidgetInterface::icon() const
{
    return d_icon;
}

QString CustomWidgetInterface::codeTemplate() const
{
    return d_codeTemplate;
}

QString CustomWidgetInterface::domXml() const
{
    return d_domXml;
}

QString CustomWidgetInterface::group() const
{
    return "Qwt Widgets";
}

QString CustomWidgetInterface::includeFile() const
{
    return d_include;
}

QString CustomWidgetInterface::name() const
{
    return d_name;
}

QString CustomWidgetInterface::toolTip() const
{
    return d_toolTip;
}

QString CustomWidgetInterface::whatsThis() const
{
    return d_whatsThis;
}

void CustomWidgetInterface::initialize(
    QDesignerFormEditorInterface *formEditor)
{
    if ( d_isInitialized )
        return;

    QExtensionManager *manager = formEditor->extensionManager();
    if ( manager )
    {
        manager->registerExtensions(new TaskMenuFactory(manager),
            Q_TYPEID(QDesignerTaskMenuExtension));
    }

    d_isInitialized = true;
}

PlotInterface::PlotInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtPlot";
    d_include = "qwt_plot.h";
    d_icon = QPixmap(":/pixmaps/qwtplot.png");
    d_domXml = 
        "<widget class=\"QwtPlot\" name=\"qwtPlot\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>400</width>\n"
        "   <height>200</height>\n"
        "  </rect>\n"
        " </property>\n"
        "</widget>\n";
}

QWidget *PlotInterface::createWidget(QWidget *parent)
{
    return new QwtPlot(parent);
}

AnalogClockInterface::AnalogClockInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtAnalogClock";
    d_include = "qwt_analog_clock.h";
    d_icon = QPixmap(":/pixmaps/qwtanalogclock.png");
    d_domXml = 
        "<widget class=\"QwtAnalogClock\" name=\"AnalogClock\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>200</width>\n"
        "   <height>200</height>\n"
        "  </rect>\n"
        " </property>\n"
        " <property name=\"lineWidth\">\n"
        "  <number>4</number>\n"
        " </property>\n"
        "</widget>\n";
}

QWidget *AnalogClockInterface::createWidget(QWidget *parent)
{
    return new QwtAnalogClock(parent);
}

CompassInterface::CompassInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtCompass";
    d_include = "qwt_compass.h";
    d_icon = QPixmap(":/pixmaps/qwtcompass.png");
    d_domXml = 
        "<widget class=\"QwtCompass\" name=\"Compass\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>200</width>\n"
        "   <height>200</height>\n"
        "  </rect>\n"
        " </property>\n"
        " <property name=\"lineWidth\">\n"
        "  <number>4</number>\n"
        " </property>\n"
        "</widget>\n";
}

QWidget *CompassInterface::createWidget(QWidget *parent)
{
    return new QwtCompass(parent);
}

CounterInterface::CounterInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtCounter";
    d_include = "qwt_counter.h";
    d_icon = QPixmap(":/pixmaps/qwtcounter.png");
    d_domXml = 
        "<widget class=\"QwtCounter\" name=\"Counter\">\n"
        "</widget>\n";
}

QWidget *CounterInterface::createWidget(QWidget *parent)
{
    return new QwtCounter(parent);
}

DialInterface::DialInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtDial";
    d_include = "qwt_dial.h";
    d_icon = QPixmap(":/pixmaps/qwtdial.png");
    d_domXml = 
        "<widget class=\"QwtDial\" name=\"Dial\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>200</width>\n"
        "   <height>200</height>\n"
        "  </rect>\n"
        " </property>\n"
        " <property name=\"lineWidth\">\n"
        "  <number>4</number>\n"
        " </property>\n"
        "</widget>\n";
}

QWidget *DialInterface::createWidget(QWidget *parent)
{
    return new QwtDial(parent);
}

KnobInterface::KnobInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtKnob";
    d_include = "qwt_knob.h";
    d_icon = QPixmap(":/pixmaps/qwtknob.png");
    d_domXml = 
        "<widget class=\"QwtKnob\" name=\"Knob\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>100</width>\n"
        "   <height>100</height>\n"
        "  </rect>\n"
        " </property>\n"
        "</widget>\n";
}

QWidget *KnobInterface::createWidget(QWidget *parent)
{
    return new QwtKnob(parent);
}

#if 0
ScaleWidgetInterface::ScaleWidgetInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtScale";
    d_include = "qwt_scale_widget.h";
    d_icon = QPixmap(":/pixmaps/qwtscale.png");
    d_domXml = 
        "<widget class=\"QwtScale\" name=\"Scale\">\n"
        "</widget>\n";
}

QWidget *ScaleWidgetInterface::createWidget(QWidget *parent)
{
    return = new QwtScaleWidget(parent);
}
#endif

SliderInterface::SliderInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtSlider";
    d_include = "qwt_slider.h";
    d_icon = QPixmap(":/pixmaps/qwtslider.png");
    d_domXml = 
        "<widget class=\"QwtSlider\" name=\"Slider\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>200</width>\n"
        "   <height>60</height>\n"
        "  </rect>\n"
        " </property>\n"
        "</widget>\n";
}

QWidget *SliderInterface::createWidget(QWidget *parent)
{
    QwtSlider *slider = new QwtSlider(parent);
#if 0
    slider->setScalePosition(QwtSlider::Bottom);
    slider->setRange(0.0, 10.0, 1.0, 0);
    slider->setValue(3.0);
#endif
    return slider;
}

TextLabelInterface::TextLabelInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtTextLabel";
    d_include = "qwt_text_label.h";

#ifdef __GNUC__
#warning QwtTextLabel icon is missing
#endif

    d_icon = QPixmap(":/pixmaps/qwtwidget.png");
    d_domXml = 
        "<widget class=\"QwtTextLabel\" name=\"TextLabel\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>100</width>\n"
        "   <height>20</height>\n"
        "  </rect>\n"
        " </property>\n"
        "</widget>\n";
}

QWidget *TextLabelInterface::createWidget(QWidget *parent)
{
    return new QwtTextLabel(parent);
}

ThermoInterface::ThermoInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtThermo";
    d_include = "qwt_thermo.h";
    d_icon = QPixmap(":/pixmaps/qwtthermo.png");
    d_domXml = 
        "<widget class=\"QwtThermo\" name=\"Thermo\">\n"
        "</widget>\n";
}

QWidget *ThermoInterface::createWidget(QWidget *parent)
{
    return new QwtThermo(parent);
}

WheelInterface::WheelInterface(QObject *parent): 
    CustomWidgetInterface(parent)
{
    d_name = "QwtWheel";
    d_include = "qwt_wheel.h";
    d_icon = QPixmap(":/pixmaps/qwtwheel.png");
    d_domXml = 
        "<widget class=\"QwtWheel\" name=\"Wheel\">\n"
        "</widget>\n";
}

QWidget *WheelInterface::createWidget(QWidget *parent)
{
    return new QwtWheel(parent);
}

CustomWidgetCollectionInterface::CustomWidgetCollectionInterface(   
        QObject *parent): 
    QObject(parent)
{
    d_plugins.append(new PlotInterface(this));
    d_plugins.append(new AnalogClockInterface(this));
    d_plugins.append(new CompassInterface(this));
    d_plugins.append(new CounterInterface(this));
    d_plugins.append(new DialInterface(this));
    d_plugins.append(new KnobInterface(this));
#if 0
    d_plugins.append(new ScaleWidgetInterface(this));
#endif
    d_plugins.append(new SliderInterface(this));
    d_plugins.append(new TextLabelInterface(this));
    d_plugins.append(new ThermoInterface(this));
    d_plugins.append(new WheelInterface(this));
}

QList<QDesignerCustomWidgetInterface*> 
    CustomWidgetCollectionInterface::customWidgets(void) const
{
    return d_plugins;
}

TaskMenuFactory::TaskMenuFactory(QExtensionManager *parent): 
    QExtensionFactory(parent)
{
}

QObject *TaskMenuFactory::createExtension(
    QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(QDesignerTaskMenuExtension))
    {
        if (QwtPlot *plot = qobject_cast<QwtPlot*>(object))
            return new TaskMenuExtension(plot, parent);
        if (QwtDial *dial = qobject_cast<QwtDial*>(object))
            return new TaskMenuExtension(dial, parent);
    }

    return QExtensionFactory::createExtension(object, iid, parent);
}


TaskMenuExtension::TaskMenuExtension(QWidget *widget, QObject *parent):
    QObject(parent),    
    d_widget(widget)
{
    d_editAction = new QAction(tr("Edit Qwt Attributes ..."), this);
    connect(d_editAction, SIGNAL(triggered()), 
        this, SLOT(editProperties()));
}

QList<QAction *> TaskMenuExtension::taskActions() const
{
    QList<QAction *> list;
    list.append(d_editAction);
    return list;
}

QAction *TaskMenuExtension::preferredEditAction() const
{
    return d_editAction;
}

void TaskMenuExtension::editProperties()
{
    const QVariant v = d_widget->property("propertiesDocument");
    if ( v.type() != QVariant::String )
        return;

    QString properties = v.toString();

    if ( qobject_cast<QwtPlot*>(d_widget) )
    {
        PlotDialog dialog(properties);
        connect(&dialog, SIGNAL(edited(const QString&)), 
            SLOT(applyProperties(const QString &)));
        (void)dialog.exec();
        return;
    }

    static QErrorMessage *errorMessage = NULL;
    if ( errorMessage == NULL )
        errorMessage = new QErrorMessage();
    errorMessage->showMessage("Not implemented yet.");
}

void TaskMenuExtension::applyProperties(const QString &properties)
{
    QDesignerFormWindowInterface *formWindow
        = QDesignerFormWindowInterface::findFormWindow(d_widget);
    if ( formWindow && formWindow->cursor() )
        formWindow->cursor()->setProperty("propertiesDocument", properties);
}

Q_EXPORT_PLUGIN2(QwtDesignerPlugin, CustomWidgetCollectionInterface)
