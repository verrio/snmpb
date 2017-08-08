SOURCES	+= snmp++/address.cpp \
	snmp++/asn1.cpp \
	snmp++/auth_priv.cpp \
	snmp++/counter.cpp \
	snmp++/ctr64.cpp \
	snmp++/eventlist.cpp \
	snmp++/eventlistholder.cpp \
	snmp++/gauge.cpp \
	snmp++/idea.cpp \
	snmp++/integer.cpp \
	snmp++/log.cpp \
	snmp++/md5c.cpp \
	snmp++/mp_v3.cpp \
	snmp++/msec.cpp \
	snmp++/msgqueue.cpp \
	snmp++/notifyqueue.cpp \
	snmp++/octet.cpp \
	snmp++/oid.cpp \
	snmp++/pdu.cpp \
	snmp++/reentrant.cpp \
	snmp++/sha.cpp \
	snmp++/snmpmsg.cpp \
	snmp++/target.cpp \
	snmp++/timetick.cpp \
	snmp++/usm_v3.cpp \
	snmp++/uxsnmp.cpp \
	snmp++/v3.cpp \
	snmp++/vb.cpp \
	snmp++/IPv6Utility.cpp \
	main.cpp \
	snmpb.cpp \
	mibnode.cpp \
	mibview.cpp \
	mibmodule.cpp \
	agent.cpp \
	trap.cpp \
	graph.cpp \
	comboboxes.cpp \
	mibhighlighter.cpp \
	markerwidget.cpp \
	mibeditor.cpp \
	mibtextedit.cpp \
	mibselection.cpp \
	logsnmpb.cpp \
	discovery.cpp \
	agentprofile.cpp \
	usmprofile.cpp \
	preferences.cpp

HEADERS	+= snmpb.h \
    snmpbapp.h \
	mibnode.h \
	mibview.h \
	mibmodule.h \
	agent.h \
	trap.h \
	graph.h \
	comboboxes.h \
	mibhighlighter.h \
	markerwidget.h \
	mibeditor.h \
	mibtextedit.h \
	mibselection.h \
	logsnmpb.h \
	discovery.h \
	agentprofile.h \
	usmprofile.h \
	preferences.h

FORMS = mainw.ui agentprofile.ui usmprofile.ui preferences.ui gotoline.ui find.ui replace.ui varbinds.ui plot.ui
TEMPLATE = app
CONFIG += qt warn_on debug
RESOURCES = snmpb.qrc
INCLUDEPATH += snmp++/ snmp++/include
LIBS += -L. -lsmi -l:libcrypto.so.1.1 -lqwt6-qt5
LANGUAGE = C++

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
win32 {
  CONFIG += release
  RC_FILE = snmpb.rc
  LIBS += -lws2_32
}
macx:ICON = images/snmpb.icns

