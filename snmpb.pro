SOURCES	+= \
	snmp++/src/address.cpp \
	snmp++/src/asn1.cpp \
	snmp++/src/auth_priv.cpp \
	snmp++/src/counter.cpp \
	snmp++/src/ctr64.cpp \
	snmp++/src/eventlist.cpp \
	snmp++/src/eventlistholder.cpp \
	snmp++/src/gauge.cpp \
	snmp++/src/idea.cpp \
	snmp++/src/integer.cpp \
	snmp++/src/md5c.cpp \
	snmp++/src/mp_v3.cpp \
	snmp++/src/msec.cpp \
	snmp++/src/msgqueue.cpp \
	snmp++/src/notifyqueue.cpp \
	snmp++/src/octet.cpp \
	snmp++/src/oid.cpp \
	snmp++/src/pdu.cpp \
	snmp++/src/reentrant.cpp \
	snmp++/src/sha.cpp \
	snmp++/src/snmpmsg.cpp \
	snmp++/src/target.cpp \
	snmp++/src/timetick.cpp \
	snmp++/src/userdefined.cpp \
	snmp++/src/usertimeout.cpp \
	snmp++/src/usm_v3.cpp \
	snmp++/src/uxsnmp.cpp \
	snmp++/src/v3.cpp \
	snmp++/src/vb.cpp \
	main.cpp \
	mibnode.cpp \
	mibview.cpp \
	mibmodule.cpp \
	agent.cpp \
	trap.cpp
HEADERS	+= mibnode.h \
	mibview.h \
	mibmodule.h \
	agent.h \
	trap.h
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= mainw.ui
IMAGES	= images/filenew \
	images/fileopen \
	images/filesave \
	images/print \
	images/undo \
	images/redo \
	images/editcut \
	images/editcopy \
	images/editpaste \
	images/searchfind \
	images/agentcap.png \
	images/column_item.png \
	images/compliance.png \
	images/folder_blue.png \
	images/folder_red.png \
	images/folder_blue_open.png \
	images/folder_red_open.png \
	images/folder_yellow.png \
	images/folder_yellow_open.png \
	images/group.png \
	images/notification.png \
	images/scalar.png \
	images/collapse.png \
	images/expand.png \
	images/snmpb.png
TEMPLATE	=app
CONFIG	+= qt warn_on debug
win32:DEFINES	+= WIN32
INCLUDEPATH	+= snmp++/include/
win32:INCLUDEPATH += C:\smi\include C:\openssl\include
win32:LIBS	+= -L. C:\smi\lib\smi.lib C:\openssl\lib\vc\libeay32.lib
unix:LIBS	+= -L. -lsmi -lcrypto
LANGUAGE	= C++
