To compile:

# make (linux, macosx, unix)
# gmake (*BSD)
> make (Windows/cygwin)

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

Instructions for windows build
-------------------------------
SnmpB builds on windows using cygwin and QT from Qt64-NG
1- Download and install cygwin from https://www.cygwin.com/
   -> Make sure not to install the toolchain "x86_64-w64-mingw32" as it will conflict with the toolchain from QT
   -> make sure to install packages listed above i.e: bison, flex, autoconf, ...
2- Download and install Qt64-NG from https://sourceforge.net/projects/qt64ng/
   -> Choose the QT x86-64 for mingw, "SJLJ" version, autoextract .exe.
      For instance, at the moment of this writing the latest QT version is 5.4.1 and the package to download is:
      http://sourceforge.net/projects/qt64ng/files/qt/x86-64/5.4.1/mingw-4.9/sjlj/qt-5.4.1-x64-mingw492r1-sjlj-rev1.exe
3- Create 2 env. variables in Windows: QTLOC and MINGWLOC. 
   - QTLOC points to the QT bin folder in the Qt64-NG install, ex.: C:\Qt\qt-5.4.1-x64-mingw492r1-sjlj-rev1\qt-5.4.1-x64-mingw492r1-sjlj-rev1\bin
   - MINGWLOC points to the mingw bin folder in the Qt64-NG install, ex.: C:\Qt\qt-5.4.1-x64-mingw492r1-sjlj-rev1\mingw64\bin 
4- Add %QTLOC% and %MINGWLOC% and the end of yout PATH env. variable, restart your shell
5- Download SnmpB source and compile with "make"
6- Download and install the NSIS installer from http://nsis.sourceforge.net/ (3.0 works)
7- Using windows explorer, go in snmpb/installer/win32 and right-click on the .nsi, then "Compile NSIS Script"
8- Voila, you have the full SnmpB windows installer .exe

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
lib/error.c "Changed the definition of yyerror for bison 3.2 (from libsmi 5.0)"
lib/parser-smi.y "YYPARSE_PARAM,YYLEX_PARAM->parse-param,lex-param for bison 3.2 (from libsmi 5.0)"
lib/scanner-smi.h "Commented yyleng for bison 3.x (from libsmi 5.0)"

Deleted mibs/ietf/IANA-ITU-ALARM-TC-MIB: redundant with one in iana folder (prevented proper copy in win32 installer)

QWT is taken from http://qwt.sourceforge.net
Version: 6.1.2

Modified file(s):
qwtconfig.pri
qwtbuild.pri "Removed silent flag, prevented build on win32"
qwt.pro

+ do not import the doc,examples,playground & admin sub-directories for space's sake

--------------------------------

Martin Jolicoeur, May 2015
