snmpb: libtomcrypt/libtomcrypt.a libsmi/lib/.libs/libsmi.a makefile.snmpb
	make -f makefile.snmpb

libtomcrypt/libtomcrypt.a:
	make -C libtomcrypt

libsmi/lib/.libs/libsmi.a: libsmi/Makefile
	make -C libsmi

libsmi/Makefile:
	cd libsmi;./configure --disable-shared

makefile.snmpb:
	qmake-qt4 -makefile -o makefile.snmpb snmpb.pro

clean:
	-make -C libtomcrypt clean
	-make -C libsmi clean
	-make -f makefile.snmpb clean
	-rm -rf .moc .obj .ui
	-rm -f makefile.snmpb
	-rm snmpb
