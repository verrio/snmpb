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

Libtomcrypt is taken from http://libtomcrypt.com
Version: 1.14

Modified file(s):
libtomcrypt/makefile

Libsmi is taken from http://www.ibr.cs.tu-bs.de/projects/libsmi
Version: 0.4.8

Modified file(s):
win/Makefile.mingw
win/config.h
lib/smi.h.in "Changed parameter named 'class' to allow compilation on C++"
lib/smi.c "Fixed a bug on paths with spaces in smiReadConfig()"

QWT is taken from http://qwt.sourceforge.net
Version: 5.1.0

Modified file(s):
qwtconfig.pri
+ do not import the doc sub-directory for space's sake

--------------------------------

Martin Jolicoeur, April 2008
