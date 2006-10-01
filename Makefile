#
# snmpb project top-level makefile. Supports Linux & Windows(cygwin/mingwin)
#
os:=$(shell uname -o)

snmpb: libtomcrypt/libtomcrypt.a \
       libsmi/lib/.libs/libsmi.a \
       qwt/lib/libqwt.a \
       app/snmpb

libtomcrypt/libtomcrypt.a:
	export CFLAGS="-mno-cygwin"; make -C libtomcrypt

libsmi/lib/.libs/libsmi.a: libsmi/Makefile
	make -C libsmi

libsmi/Makefile:
ifeq (${os}, Cygwin)
	cd libsmi; export CPPFLAGS="-mno-cygwin"; automake-1.4; ./configure --disable-shared --with-pathseparator=';' --with-dirseparator='/'
else
	# Linux FC5
	cd libsmi; automake-1.4; ./configure --disable-shared --with-pathseparator=';' --with-dirseparator='/'
endif

qwt/lib/libqwt.a: qwt/Makefile
	make -C qwt

qwt/Makefile:
ifeq (${os}, Cygwin)
	cd qwt; export MINGW_IN_SHELL=1; qmake qwt.pro
else
	# Linux FC5
	cd qwt; qmake-qt4 qwt.pro
endif

app/snmpb:
	make -C app

clean:
	-make -C libtomcrypt clean
	-make -C libsmi clean
	-make -C qwt clean
	-make -C app clean
