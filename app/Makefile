#
# Makefile for snmpb app
#

snmpb: makefile.snmpb
	make -f makefile.snmpb

makefile.snmpb:
ifeq (${os}, Cygwin)
	export MINGW_IN_SHELL=1; qmake -makefile -o makefile.snmpb snmpb.pro
	sed -e 's/.*qtmain.prl/Makefile/g' makefile.snmpb >> makefile.temp
	mv makefile.temp makefile.snmpb
else
	# Linux/FC5
	qmake-qt4 -makefile -o makefile.snmpb snmpb.pro
endif

clean:
	-make -f makefile.snmpb clean
	-rm -rf .moc .obj .ui
	-rm -f makefile.snmpb
	-rm snmpb
