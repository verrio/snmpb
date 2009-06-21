To compile:

# make (or gmake)

To install in places other than /usr, add INSTALL_PREFIX=<prefix> to the make command.

Tested & compiles on Cygwin/Windows, Linux, MacOSX (Leopard) and NetBSD

--------------------------------

Required installed packages for compilation:
- bison & flex
- GNU make
- QT4 development package
- GNU install
- gcc

External packages
------------------

Snmp++ is taken from http://www.agentpp.com
Version: 3.2.23

Modified file(s):
config_snmp_pp.h
src/notifyqueue.cpp "Fix MacOSX build error with cont fd_set"
src/msgqueue.cpp "Fix MacOSX build error with cont fd_set"
v3.h "Changed debugprintf() macro to log debug messages"

Libtomcrypt is taken from http://libtomcrypt.com
Version: 1.16

Modified file(s):
makefile
src/headers/tomcrypt_pk.h "Added defined(BSD) around wchar_t definition to fix broken build on *BSD"

Libsmi is taken from http://www.ibr.cs.tu-bs.de/projects/libsmi
Version: 0.4.8

Modified file(s):
win/Makefile.mingw
win/config.h
lib/smi.h.in "Changed parameter named 'class' to allow compilation on C++"
lib/smi.c "Fixed a bug on paths with spaces in smiReadConfig()"
lib/data.c, loadModule() -> changed allowed mib files extensions
lib/util.c, smiIsPath() -> removed condition for "." in filename
lib/snprintf.c, "Upgraded snprintf.c from source to support 64 bits printing on Windows"

QWT is taken from http://qwt.sourceforge.net
Version: 5.1.0

Modified file(s):
qwtconfig.pri
+ do not import the doc sub-directory for space's sake

--------------------------------

Martin Jolicoeur, April 2008
