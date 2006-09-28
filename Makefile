#
# WIN32: Dont forget to adjust the PATH env. variable to put the mingw32 
# toolchain in front of the cygwin toolchain ....
#
# export PATH=/cygdrive/c/mingw/bin:${PATH}
#

os:=$(shell uname -o)

snmpb: libtomcrypt/libtomcrypt.a \
       libsmi/lib/.libs/libsmi.a \
       qwt/lib/libqwt.a \
       app/snmpb

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

app/snmpb:
	make -C app

clean:
	-make -C libtomcrypt clean
	-make -C libsmi clean
	-make -C qwt clean
	-make -C app clean
