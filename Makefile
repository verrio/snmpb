#
# snmpb project top-level makefile. Supports Linux, MacOSX, NetBSD & Cygwin/Windows
#
#    Copyright (C) 2004-2011 Martin Jolicoeur (snmpb1@gmail.com) 
#
#    This file is part of the SnmpB project 
#    (http://sourceforge.net/projects/snmpb)
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
os:=$(shell uname -s)

ifneq ($(findstring BSD,${os}),)
INSTALL=ginstall
SHARE=share
else
INSTALL=install
SHARE=share/apps
endif

INSTALL_PREFIX=/usr

all: snmpb

ifneq ($(findstring CYGWIN,${os}),)
snmpb: libtomcrypt/libtomcrypt.a \
       libsmi/win/libsmi.a \
       qwt/lib/libqwt.a \
       app/snmpb
else
snmpb: libtomcrypt/libtomcrypt.a \
       libsmi/lib/.libs/libsmi.a \
       qwt/lib/libqwt.a \
       app/snmpb
endif

libtomcrypt/libtomcrypt.a:
ifneq ($(findstring CYGWIN,${os}),)
	$(MAKE) -C libtomcrypt CC=mingw32-gcc
else
	$(MAKE) -C libtomcrypt
endif

ifneq ($(findstring CYGWIN,${os}),)
libsmi/win/libsmi.a:
	$(MAKE) -C libsmi/win -f Makefile.mingw CC=mingw32-gcc
else
libsmi/lib/.libs/libsmi.a: libsmi/Makefile
	$(MAKE) -C libsmi
endif

libsmi/Makefile:
ifneq ($(findstring Darwin,${os}),)
	# MacOSX
	cd libsmi; ./configure --disable-shared --with-pathseparator=';' --with-dirseparator='/' --with-smipath='/Applications/SnmpB.app/Contents/MacOS/mibs;/Applications/SnmpB.app/Contents/MacOS/pibs'
else
	# Linux/BSD
	cd libsmi; ./configure --disable-shared --with-pathseparator=';' --with-dirseparator='/' --with-smipath=${INSTALL_PREFIX}'/${SHARE}/snmpb/mibs;'${INSTALL_PREFIX}'/${SHARE}/snmpb/pibs'
endif

qwt/lib/libqwt.a: qwt/Makefile
	$(MAKE) -C qwt

qwt/Makefile:
ifneq ($(findstring CYGWIN,${os}),)
	cd qwt; qmake qwt.pro
else
ifneq ($(findstring Darwin,${os}),)
	# MacOSX
	cd qwt; qmake -spec macx-g++ qwt.pro
else
	# Linux/BSD
	cd qwt; qmake-qt4 qwt.pro
endif
endif

app/snmpb:
	$(MAKE) -C app

clean:
	-$(MAKE) -C libtomcrypt clean
ifneq ($(findstring CYGWIN,${os}),)
	-$(MAKE) -C libsmi/win -f Makefile.mingw clean
else
	-$(MAKE) -C libsmi clean
	rm libsmi/Makefile
endif
	-$(MAKE) -C qwt clean
	-$(MAKE) -C app clean

install:
	$(INSTALL) -v -d ${INSTALL_PREFIX}/bin ${INSTALL_PREFIX}/${SHARE}/snmpb/mibs ${INSTALL_PREFIX}/${SHARE}/snmpb/pibs
	$(INSTALL) -v -m 4755 -D -s -o root app/snmpb ${INSTALL_PREFIX}/bin
	$(INSTALL) -v -m 444 -o root libsmi/mibs/iana/* ${INSTALL_PREFIX}/${SHARE}/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/mibs/ietf/* ${INSTALL_PREFIX}/${SHARE}/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/mibs/tubs/* ${INSTALL_PREFIX}/${SHARE}/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/pibs/ietf/* ${INSTALL_PREFIX}/${SHARE}/snmpb/pibs
	$(INSTALL) -v -m 444 -o root libsmi/pibs/tubs/* ${INSTALL_PREFIX}/${SHARE}/snmpb/pibs
	rm -f ${INSTALL_PREFIX}/${SHARE}/snmpb/mibs/Makefile* ${INSTALL_PREFIX}/${SHARE}/snmpb/pibs/Makefile*
	$(INSTALL) -v -d ${INSTALL_PREFIX}/share/applications ${INSTALL_PREFIX}/share/mime/packages
	$(INSTALL) -v -m 444 -o root app/snmpb.desktop ${INSTALL_PREFIX}/share/applications
	$(INSTALL) -v -m 444 -o root app/snmpb.xml ${INSTALL_PREFIX}/share/mime/packages
	cat ${INSTALL_PREFIX}/share/applications/defaults.list | grep -v "text\/x-mib=" | grep -v "text\/x-mib2=" | grep -v "text\/x-pib=" | grep -v "text\/x-pib2=" | grep -v "text\/x-smi=" | grep -v "text\/x-smi2=" | grep -v "text\/x-smi3=" | grep -v "text\/x-smi4=" | grep -v "text\/x-smi5=" > /tmp/snmpb-assoc; cat app/defaults.list >> /tmp/snmpb-assoc;
	$(INSTALL) -v -m 444 -o root /tmp/snmpb-assoc ${INSTALL_PREFIX}/share/applications/defaults.list
	rm -f /tmp/snmpb-assoc
	update-mime-database ${INSTALL_PREFIX}/share/mime
	$(INSTALL) -v -d ${INSTALL_PREFIX}/share/icons/hicolor/128x128/apps ${INSTALL_PREFIX}/share/pixmaps ${INSTALL_PREFIX}/share/icons/hicolor/scalable/apps
	$(INSTALL) -v -m 444 -o root app/images/snmpb.png ${INSTALL_PREFIX}/share/icons/hicolor/128x128/apps
	$(INSTALL) -v -m 444 -o root app/images/snmpb.png ${INSTALL_PREFIX}/share/pixmaps
	$(INSTALL) -v -m 444 -o root app/images/snmpb.svg ${INSTALL_PREFIX}/share/icons/hicolor/scalable/apps

