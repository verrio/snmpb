#
# snmpb project top-level makefile. Supports Linux, NetBSD & Cygwin/Windows
#
os:=$(shell uname -s)

ifneq ($(findstring BSD,${os}),)
INSTALL=ginstall
else
INSTALL=install
endif

INSTALL_PREFIX=/usr

ifneq ($(findstring CYGWIN,${os}),)
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
ifneq ($(findstring CYGWIN,${os}),)
	export CFLAGS="-mno-cygwin"; $(MAKE) -C libtomcrypt
else
	$(MAKE) -C libtomcrypt
endif

ifneq ($(findstring CYGWIN,${os}),)
libsmi/win/libsmi.a:
	$(MAKE) -C libsmi/win -f Makefile.mingw
else
libsmi/lib/.libs/libsmi.a: libsmi/Makefile
	$(MAKE) -C libsmi
endif

libsmi/Makefile:
	# Linux/BSD
	cd libsmi; automake-1.4; ./configure --disable-shared --with-pathseparator=';' --with-dirseparator='/' --with-smipath=${INSTALL_PREFIX}'/share/apps/snmpb/mibs;'${INSTALL_PREFIX}'/share/apps/snmpb/pibs'

qwt/lib/libqwt.a: qwt/Makefile
	$(MAKE) -C qwt

qwt/Makefile:
ifneq ($(findstring CYGWIN,${os}),)
	cd qwt; export DIR_SEPARATOR="/"; qmake qwt.pro
	sed -e 's/c:/\/cygdrive\/c/g;s/C:.*moc.exe/moc.exe/g' qwt/Makefile > qwt/Makefile.tmp
	mv qwt/Makefile.tmp qwt/Makefile
	sed -e 's/c:/\/cygdrive\/c/g;s/C:.*moc.exe/moc.exe/g' qwt/Makefile.Debug > qwt/Makefile.tmp
	mv qwt/Makefile.tmp qwt/Makefile.Debug
	sed -e 's/c:/\/cygdrive\/c/g;s/C:.*moc.exe/moc.exe/g' qwt/Makefile.Release > qwt/Makefile.tmp
	mv qwt/Makefile.tmp qwt/Makefile.Release
else
	# Linux/BSD
	cd qwt; qmake-qt4 qwt.pro
endif

app/snmpb:
	$(MAKE) -C app

clean:
	-$(MAKE) -C libtomcrypt clean
ifneq ($(findstring CYGWIN,${os}),)
	-$(MAKE) -C libsmi/win -f Makefile.mingw clean
else
	-$(MAKE) -C libsmi clean
	rm libsmi/Makefile
endif
	-$(MAKE) -C qwt clean
	-$(MAKE) -C app clean

install:
	$(INSTALL) -v -d ${INSTALL_PREFIX}/bin ${INSTALL_PREFIX}/share/apps/snmpb/mibs ${INSTALL_PREFIX}/share/apps/snmpb/pibs
	$(INSTALL) -v -D -s -o root app/snmpb ${INSTALL_PREFIX}/bin
	$(INSTALL) -v -m 444 -o root libsmi/mibs/iana/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/mibs/ietf/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/mibs/irtf/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/mibs/tubs/* ${INSTALL_PREFIX}/share/apps/snmpb/mibs
	$(INSTALL) -v -m 444 -o root libsmi/pibs/ietf/* ${INSTALL_PREFIX}/share/apps/snmpb/pibs
	$(INSTALL) -v -m 444 -o root libsmi/pibs/tubs/* ${INSTALL_PREFIX}/share/apps/snmpb/pibs
	rm ${INSTALL_PREFIX}/share/apps/snmpb/mibs/Makefile* ${INSTALL_PREFIX}/share/apps/snmpb/pibs/Makefile*

