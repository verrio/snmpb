#
# snmpb project top-level makefile. Supports Linux, MacOSX, NetBSD & Cygwin/Windows
#
#    Copyright (C) 2004-2008  Martin Jolicoeur (snmpb1@gmail.com) 
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
else
INSTALL=install
endif

INSTALL_PREFIX=/usr

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
	export CFLAGS="-mno-cygwin"; $(MAKE) -C libtomcrypt
else
	$(MAKE) -C libtomcrypt
endif

ifneq ($(findstring CYGWIN,${os}),)
libsmi/win/libsmi.a:
	$(MAKE) -C libsmi/win -f Makefile.mingw
else
libsmi/lib/.libs/libsmi.a: libsmi/Makefile
	$(MAKE) -C libsmi
endif

libsmi/Makefile:
	# Linux/BSD
	cd libsmi; ./configure --disable-shared --with-pathseparator=';' --with-dirseparator='/' --with-smipath=${INSTALL_PREFIX}'/share/apps/snmpb/mibs;'${INSTALL_PREFIX}'/share/apps/snmpb/pibs'

qwt/lib/libqwt.a: qwt/Makefile
	$(MAKE) -C qwt

qwt/Makefile:
ifneq ($(findstring CYGWIN,${os}),)
	cd qwt; export DIR_SEPARATOR="/"; qmake qwt.pro
	sed -e 's/c:\//\/cygdrive\/c\//g;s/C:.*moc.exe/moc.exe/g' qwt/Makefile > qwt/Makefile.tmp
	mv qwt/Makefile.tmp qwt/Makefile
	cd qwt/src; export DIR_SEPARATOR="/"; qmake src.pro -win32 -o Makefile
	sed -e 's/c:\//\/cygdrive\/c\//g;s/C:.*moc.exe/moc.exe/g' qwt/src/Makefile > qwt/src/Makefile.tmp
	mv qwt/src/Makefile.tmp qwt/src/Makefile
	sed -e 's/c:\//\/cygdrive\/c\//g;s/C:.*moc.exe/moc.exe/g' qwt/src/Makefile.Debug > qwt/src/Makefile.tmp
	mv qwt/src/Makefile.tmp qwt/src/Makefile.Debug
	sed -e 's/c:\//\/cygdrive\/c\//g;s/C:.*moc.exe/moc.exe/g' qwt/src/Makefile.Release > qwt/src/Makefile.tmp
	mv qwt/src/Makefile.tmp qwt/src/Makefile.Release
	cd qwt/textengines; export DIR_SEPARATOR="/"; qmake textengines.pro -win32 -o Makefile
	sed -e 's/c:\//\/cygdrive\/c\//g;s/C:.*moc.exe/moc.exe/g' qwt/textengines/Makefile > qwt/textengines/Makefile.tmp
	mv qwt/textengines/Makefile.tmp qwt/textengines/Makefile
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
	$(INSTALL) -v -d ${INSTALL_PREFIX}/bin ${INSTALL_PREFIX}/share/apps/snmpb/mibs ${INSTALL_PREFIX}/share/apps/snmpb/pibs
	$(INSTALL) -v -m 4755 -D -s -o root app/snmpb ${INSTALL_PREFIX}/bin
	$(INSTALL) -v -m 444 -o root libsmi/mibs/iana/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/mibs/ietf/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/mibs/irtf/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/mibs/tubs/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/pibs/ietf/* ${INSTALL_PREFIX}/share/apps/snmpb/pibs
	$(INSTALL) -v -m 444 -o root libsmi/pibs/tubs/* ${INSTALL_PREFIX}/share/apps/snmpb/pibs
	rm ${INSTALL_PREFIX}/share/apps/snmpb/mibs/Makefile* ${INSTALL_PREFIX}/share/apps/snmpb/pibs/Makefile*

