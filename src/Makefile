#
# Makefile for the snmpb Qt application
#
#    Copyright (c) 2004-2017 Martin Jolicoeur and contributors
#
#    This file is part of the snmpb project and is free software: you can
#    redistribute it and/or modify it under the terms of the GNU General
#    Public License as published by the Free Software Foundation, either
#    version 3 of the License, or (at your option) any later version.
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
	$(MAKE) -C ../ app/makefile.snmpb

clean:
	-$(MAKE) -f makefile.snmpb clean
	-rm -rf .moc .obj .ui
	-rm -f makefile.snmpb
	-rm snmpb
	-rm -rf release
	-rm -rf debug
	-rm -rf snmpb.app 
