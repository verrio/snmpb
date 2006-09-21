TEMPLATE = subdirs

SUBDIRS = \
    bode \
    cpuplot \
    curvdemo1   \
    curvdemo2 \
    data_plot \
    simple_plot \
    event_filter \
    realtime_plot \
    spectrogram \
    histogram \
    sysinfo \
    radio \
    dials \
    sliders

##############################################
# The svgmap example is only available, if 
# qwt_plot_svgitem.[h|cpp] have been enabled
# in qwt.pro.
##############################################

# SUBDIRS += svgmap
