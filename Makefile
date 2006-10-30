#
# snmpb project top-level makefile. Supports Linux & Windows(mingwin)
#
os:=$(shell uname -o)

INSTALL_PREFIX=/usr

ifeq (${os}, Cygwin)
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
ifeq (${os}, Cygwin)
	export CFLAGS="-mno-cygwin"; make -C libtomcrypt
else
	make -C libtomcrypt
endif

ifeq (${os}, Cygwin)
libsmi/win/libsmi.a:
	make -C libsmi/win -f Makefile.mingw
else
libsmi/lib/.libs/libsmi.a: libsmi/Makefile
	make -C libsmi
endif

libsmi/Makefile:
	# Linux FC5
	cd libsmi; automake-1.4; ./configure --disable-shared --with-pathseparator=';' --with-dirseparator='/' --with-smipath=${INSTALL_PREFIX}'/share/apps/snmpb/mibs;'${INSTALL_PREFIX}'/share/apps/snmpb/pibs'

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
ifeq (${os}, Cygwin)
	-make -C libsmi/win -f Makefile.mingw clean
else
	-make -C libsmi clean
	rm libsmi/Makefile
endif
	-make -C qwt clean
	-make -C app clean

install:
	install -v -d ${INSTALL_PREFIX}/bin ${INSTALL_PREFIX}/share/apps/snmpb/mibs ${INSTALL_PREFIX}/share/apps/snmpb/pibs
	install -v -D -s -o root app/snmpb ${INSTALL_PREFIX}/bin
	install -v -m 444 -o root libsmi/mibs/iana/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	install -v -m 444 -o root libsmi/mibs/ietf/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	install -v -m 444 -o root libsmi/mibs/irtf/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	install -v -m 444 -o root libsmi/mibs/tubs/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	install -v -m 444 -o root libsmi/pibs/ietf/* ${INSTALL_PREFIX}/share/apps/snmpb/pibs
	install -v -m 444 -o root libsmi/pibs/tubs/* ${INSTALL_PREFIX}/share/apps/snmpb/pibs
	rm ${INSTALL_PREFIX}/share/apps/snmpb/mibs/Makefile* ${INSTALL_PREFIX}/share/apps/snmpb/pibs/Makefile*

