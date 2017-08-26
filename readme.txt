To compile:

# make (linux, macosx, unix)
# gmake (*BSD)
> make (Windows/MSYS2)

To install in places other than /usr, add INSTALL_PREFIX=<prefix> to the make command.

Tested & compiles on MSYS2/Windows, Linux, MacOSX (Leopard) and NetBSD

--------------------------------

Required installed packages for compilation:
- bison & flex
- autoconf and automake
- GNU make
- QT5 development package
- GNU install
- gcc and g++

Instructions for windows build
-------------------------------
SnmpB builds on windows using MSYS2 
1- Download and install MSYS2 from https://sourceforge.net/projects/msys2
2- Start the 'MINGW64' flavor of MSYS2 by executing the script 'mingw64_shell.bat' from the root of the MSYS2 installation folder
3- Install packages for QT build on MSYS2 (taken from https://wiki.qt.io/MSYS2), specifically:
   pacman -Sy
   pacman --needed -S pacman pacman-mirrors msys2-runtime
   (restart shell)
   pacman -Su
   pacman -S base-devel git mingw-w64-x86_64-toolchain mingw-w64-x86_64-qt5-static mingw-w64-x86_64-qt-creator  
4- Download SnmpB source and compile with "make"
5- Download and install the NSIS installer from http://nsis.sourceforge.net/ (3.0 works)
6- Using windows explorer, go in snmpb/installer/win32 and right-click on the .nsi, 
   then "Compile NSIS Script"
7- Voila, you have the full SnmpB windows installer .exe

External packages
------------------

Snmp++ is taken from http://www.agentpp.com
Version: 3.3.9

Modified file(s):
config_snmp_pp.h libsnmp.h config.h (new) "Hard-coded values instead of using package's autoconf"
notifyqueue.cpp "Added missing ipv6 include required for compilation on WIN32"
address.cpp "Replaced custom get_printable for IPv6 addresses by inet_ntop()"
notifyqueue.h uxsnmp.h notifyqueue.cpp uxsnmp.cpp 
"Modified snmp++ to allow binding trap port on both ipv4 and ipv6 for all interfaces"
uxsnmp.h notifyqueue.cpp "Added set/get_notify_callback_fd() to access fd when replying to INFORMS"
IPv6Utility.cpp IPv6Utility.h "Re-enabled local inet_pton unavailable on MSYS2/MINGW"
uxsnmp.h "Redefined INVALID_SOCKET to get rid of compiler warnings"

Libtomcrypt is taken from http://libtom.org
Version: 1.17

Modified file(s):
makefile
src/headers/tomcrypt_pk.h "Added defined(BSD) around wchar_t definition to fix broken build on *BSD"
src/headers/tomcrypt_custom.h "Defined LTC_NO_ROLC for MacOSX using LLVM"
src/headers/tomcrypt_macros.h "Disabled ROL64/ROR64 assembly routines on Windows 64 bits"

Libsmi is taken from http://www.ibr.cs.tu-bs.de/projects/libsmi
Version: 0.5.0

Modified file(s):
win/Makefile.mingw
win/config.h
Makefile.am     "Removed compilation of all subdirs except lib"
lib/Makefile.am "Fixed file generation rules: broke parallel build"
lib/util.c      "smiIsPath(): removed condition for "." in filename"
lib/snprintf.c  "Upgraded snprintf.c from source to support 64 bits printing on Windows"
lib/smi.c       "Fixed a bug on paths with spaces in smiReadConfig()"
lib/smi.c       "Fixed stack overwrite in smiRenderValue due to uint64 format"
lib/smi.c       "Fixed display for non-printable octet-strings -tracker 3096221"
lib/smi.c       "Allows OCTET STRING to be printable with \v, \t , \r, \n, \f"
Deleted mibs/ietf/IANA-ITU-ALARM-TC-MIB: redundant with one in iana folder 
(prevented proper copy in win32 installer)
Ran 'autoreconf -i' to generate Makefiles for 1.15 libtools

QWT is taken from http://qwt.sourceforge.net
Version: 6.1.2

Modified file(s):
qwtconfig.pri
qwtbuild.pri "Removed silent flag, prevented build on win32"
qwt.pro

+ do not import the doc,examples,playground & admin sub-directories for space's sake

--------------------------------

Martin Jolicoeur, August 2017
