SOURCES	+= main.cpp \
	mibnode.cpp \
	mibview.cpp \
	mibmodule.cpp \
	agent.cpp
HEADERS	+= mibnode.h \
	mibview.h \
	mibmodule.h \
	agent.h
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
CONFIG	+= qt warn_on release
INCLUDEPATH	+= ../../snmp++/include/
LIBS	+= -lsmi -L. -lsnmp++ -lcrypto
LANGUAGE	= C++
