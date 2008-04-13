#
# Makefile for snmpb app
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

os:=$(shell uname -s)

snmpb_app: makefile.snmpb
	$(MAKE) -f makefile.snmpb

makefile.snmpb:
ifneq ($(findstring CYGWIN,${os}),)
	export DIR_SEPARATOR="/"; qmake -makefile -o makefile.snmpb snmpb.pro
	sed -e 's/.*qtmain.prl/Makefile/g;s/c:/\/cygdrive\/c/g;s/C:.*moc.exe/moc.exe/g' makefile.snmpb > makefile.tmp
	mv makefile.tmp makefile.snmpb
	sed -e 's/c:/\/cygdrive\/c/g;s/C:.*moc.exe/moc.exe/g' makefile.snmpb.Debug > makefile.tmp
	mv makefile.tmp makefile.snmpb.Debug
	sed -e 's/c:/\/cygdrive\/c/g;s/C:.*moc.exe/moc.exe/g' makefile.snmpb.Release > makefile.tmp
	mv makefile.tmp makefile.snmpb.Release
else
	# Linux/BSD
	qmake-qt4 -makefile -o makefile.snmpb snmpb.pro
endif

clean:
	-$(MAKE) -f makefile.snmpb clean
	-rm -rf .moc .obj .ui
	-rm -f makefile.snmpb
	-rm snmpb
	-rm -rf release
	-rm -rf debug
