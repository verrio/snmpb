To compile:

# make (linux, macosx, unix)
# gmake (*BSD)
> mingw32-make (Windows)

To install in places other than /usr, add INSTALL_PREFIX=<prefix> to the make command.

Tested & compiles on Cygwin/Windows, Linux, MacOSX (Leopard) and NetBSD

--------------------------------

Required installed packages for compilation:
- bison & flex
- autoconf and automake
- GNU make
- QT4 development package
- GNU install
- gcc and g++

External packages
------------------

Snmp++ is taken from http://www.agentpp.com
Version: 3.2.25

Modified file(s):
config_snmp_pp.h
v3.h "Changed debugprintf() macro to log debug messages"
notifyqueue.cpp "Added missing ipv6 include required for compilation on WIN32"
address.cpp "Replaced custom get_printable for IPv6 addresses by inet_ntop()"
notifyqueue.h uxsnmp.h notifyqueue.cpp uxsnmp.cpp 
"Modified snmp++ to allow binding trap port on both ipv4 and ipv6 for all interfaces"
uxsnmp.h notifyqueue.cpp "Added set/get_notify_callback_fd() to access fd when replying to INFORMS"

Libtomcrypt is taken from http://libtom.org
Version: 1.17

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
lib/smi.c  "Fixed stack overwrite in smiRenderValue due to uint64 format"
lib/smi.c  "Fixed display for non-printable octet-strings -tracker 3096221"
lib/smi.c  "Allows OCTET STRING to be printable with \v, \t , \r, \n, \f"

QWT is taken from http://qwt.sourceforge.net
Version: 6.0.0

Modified file(s):
qwtconfig.pri
+ do not import the doc and examples sub-directories for space's sake

--------------------------------

Martin Jolicoeur, April 2012
