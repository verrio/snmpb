#
# Makefile for snmpb app
#

os:=$(shell uname -o)

snmpb_app: makefile.snmpb
	make -f makefile.snmpb

makefile.snmpb:
ifeq (${os}, Cygwin)
	export DIR_SEPARATOR="/"; qmake -makefile -o makefile.snmpb snmpb.pro
	sed -e 's/.*qtmain.prl/Makefile/g;s/c:/\/cygdrive\/c/g;s/C:.*moc.exe/moc.exe/g' makefile.snmpb > makefile.tmp
	mv makefile.tmp makefile.snmpb
	sed -e 's/c:/\/cygdrive\/c/g;s/C:.*moc.exe/moc.exe/g' makefile.snmpb.Debug > makefile.tmp
	mv makefile.tmp makefile.snmpb.Debug
	sed -e 's/c:/\/cygdrive\/c/g;s/C:.*moc.exe/moc.exe/g' makefile.snmpb.Release > makefile.tmp
	mv makefile.tmp makefile.snmpb.Release
else
	# Linux
	qmake-qt4 -makefile -o makefile.snmpb snmpb.pro
endif

clean:
	-make -f makefile.snmpb clean
	-rm -rf .moc .obj .ui
	-rm -f makefile.snmpb
	-rm snmpb
	-rm -rf release
	-rm -rf debug
