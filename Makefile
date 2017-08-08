#
# snmpb project top-level makefile.
# Supports Linux, NetBSD & Cygwin/Windows
#
# This file is part of the snmpb project 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
os:=$(shell uname -s)

# Default QT for windows: static 64 bits QT under MSYS2
WINQT_PREFIX=/mingw64/qt5-static/bin/

ifneq ($(findstring BSD,${os}),)
INSTALL=install
SHARE=share
else
INSTALL=install -v
SHARE=share/apps
endif

ifndef INSTALL_PREFIX
INSTALL_PREFIX=/usr
endif

all: snmpb

snmpb: src/snmpb

src/makefile.snmpb:
ifneq ($(findstring MINGW,${os}),)
	cd src; ${WINQT_PREFIX}qmake -o makefile.snmpb snmpb.pro
else
	cd src; qmake -qt=qt5 -o makefile.snmpb snmpb.pro
endif

src/snmpb: src/makefile.snmpb
	$(MAKE) -C src

clean:
	-$(MAKE) -C src clean

install:
	$(INSTALL) -d ${INSTALL_PREFIX}/bin ${INSTALL_PREFIX}/${SHARE}/snmpb/mibs ${INSTALL_PREFIX}/${SHARE}/snmpb/pibs
	$(INSTALL) -m 4755 -D -s -o root src/snmpb ${INSTALL_PREFIX}/bin
	rm -f ${INSTALL_PREFIX}/${SHARE}/snmpb/mibs/Makefile* ${INSTALL_PREFIX}/${SHARE}/snmpb/pibs/Makefile*
	$(INSTALL) -d ${INSTALL_PREFIX}/share/applications ${INSTALL_PREFIX}/share/mime/packages
	$(INSTALL) -m 444 -o root src/snmpb.desktop ${INSTALL_PREFIX}/share/applications
	$(INSTALL) -m 444 -o root src/snmpb.xml ${INSTALL_PREFIX}/share/mime/packages
	cat ${INSTALL_PREFIX}/share/applications/defaults.list | grep -v "text\/x-mib=" | grep -v "text\/x-mib2=" | grep -v "text\/x-pib=" | grep -v "text\/x-pib2=" | grep -v "text\/x-smi=" | grep -v "text\/x-smi2=" | grep -v "text\/x-smi3=" | grep -v "text\/x-smi4=" | grep -v "text\/x-smi5=" > /tmp/snmpb-assoc; cat src/defaults.list >> /tmp/snmpb-assoc;
	$(INSTALL) -m 444 -o root /tmp/snmpb-assoc ${INSTALL_PREFIX}/share/applications/defaults.list
	rm -f /tmp/snmpb-assoc
ifeq ($(findstring BSD,${os}),)
	update-mime-database ${INSTALL_PREFIX}/share/mime
endif
	$(INSTALL) -d ${INSTALL_PREFIX}/share/icons/hicolor/128x128/apps ${INSTALL_PREFIX}/share/pixmaps ${INSTALL_PREFIX}/share/icons/hicolor/scalable/apps
	$(INSTALL) -m 444 -o root src/images/snmpb.png ${INSTALL_PREFIX}/share/icons/hicolor/128x128/apps
	$(INSTALL) -m 444 -o root src/images/snmpb.png ${INSTALL_PREFIX}/share/pixmaps
	$(INSTALL) -m 444 -o root src/images/snmpb.svg ${INSTALL_PREFIX}/share/icons/hicolor/scalable/apps

