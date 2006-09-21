all: libtomcrypt/libtomcrypt.a snmpb

libtomcrypt/libtomcrypt.a:
	make -C libtomcrypt

snmpb: makefile.snmpb
	make -f makefile.snmpb

makefile.snmpb:
	qmake-qt4 -makefile -o makefile.snmpb snmpb.pro

clean:
	make -C libtomcrypt clean
	make -f makefile.snmpb clean
	rm -rf .moc .obj .ui
	rm -f makefile.snmpb
	rm snmpb
