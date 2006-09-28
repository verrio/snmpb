#
# WIN32: Dont forget to adjust the PATH env. variable to put the mingw32 
# toolchain in front of the cygwin toolchain ....
#
# export PATH=/cygdrive/c/mingw/bin:${PATH}
#

os:=$(shell uname -o)

snmpb: libtomcrypt/libtomcrypt.a libsmi/lib/.libs/libsmi.a qwt/lib/libqwt.a makefile.snmpb
	make -f makefile.snmpb

libtomcrypt/libtomcrypt.a:
	make -C libtomcrypt

libsmi/lib/.libs/libsmi.a: libsmi/Makefile
	make -C libsmi

libsmi/Makefile:
	cd libsmi; automake-1.4; ./configure --disable-shared --with-pathseparator=';' --with-dirseparator='/'

qwt/lib/libqwt.a: qwt/Makefile
	make -C qwt

qwt/Makefile:
ifeq (${os}, Cygwin)
	cd qwt; export MINGW_IN_SHELL=1; qmake qwt.pro
else
	# Linux FC5
	cd qwt; qmake-qt4 qwt.pro
endif

makefile.snmpb:
ifeq (${os}, Cygwin)
	export MINGW_IN_SHELL=1; qmake -makefile -o makefile.snmpb snmpb.pro; cat makefile.snmpb| sed -e 's/.*qtmain.prl/Makefile/g' >> makefile.temp; mv makefile.temp makefile.snmpb
else
	qmake-qt4 -makefile -o makefile.snmpb snmpb.pro
endif

clean:
	-make -C libtomcrypt clean
	-make -C libsmi clean
	-make -C qwt clean
	-make -f makefile.snmpb clean
	-rm -rf .moc .obj .ui
	-rm -f makefile.snmpb
	-rm snmpb
