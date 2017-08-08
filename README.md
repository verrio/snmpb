snmpb client
============

snmpb is a graphical SNMP client using the SNMP++ library,
libsmi parser and Qt framework.

![screenshot](/screenshot.png "screenshot")

This client can be used for querying the MIB of SNMPv1/v2c/v3 agents, receiving traps, discovering devices and editing MIB description files.  The current implementation has quite some memory leaks, but you won't be bothered by them, because by the time they'll fill up your memory the application will have segfaulted and crashed.

## Dependencies

Build time dependencies:
- bison & flex
- autoconf and automake
- GNU make
- QT5 development package
- GNU install
- gcc and g++

Runtime dependencies:
- [OpenSSL](https://www.openssl.org) v1.1.0 
- [libsmi](https://www.ibr.cs.tu-bs.de/projects/libsmi/) v0.5.0
- [Qt](https://www.qt.io) v5
- [QWT](http://qwt.sourceforge.net) v6.1.2

## Compilation

- make (linux, unix, Windows/MSYS2)
- gmake (*BSD)

To install in places other than /usr, add INSTALL_PREFIX=<prefix> to the make command.

Tested & compiles on MSYS2/Windows, Linux and NetBSD

### Instructions for windows build

snmpb builds on windows using MSYS2 
1. Download and install MSYS2 from https://sourceforge.net/projects/msys2
2. Start the 'MINGW64' flavor of MSYS2 by executing the script 'mingw64_shell.bat' from the root of the MSYS2 installation folder
3. Install packages for QT build on MSYS2 (taken from https://wiki.qt.io/MSYS2), specifically:
   pacman -Sy
   pacman --needed -S pacman pacman-mirrors msys2-runtime
   (restart shell)
   pacman -Su
   pacman -S base-devel git mingw-w64-x86_64-toolchain mingw-w64-x86_64-qt5-static mingw-w64-x86_64-qt-creator  
4. Download SnmpB source and compile with "make"
5. Download and install the NSIS installer from http://nsis.sourceforge.net/ (3.0 works)
6. Using windows explorer, go in snmpb/installer/win32 and right-click on the .nsi, 
   then "Compile NSIS Script"
7. Voilà, you have the full SnmpB windows installer .exe

## Installation

To install snmpb, type make install (as root) with the install prefix in argument:

```
make INSTALL_PREFIX=/usr install
```

default is /usr

## Licensing

This application is distributed under the GNU General Public License v2.  See the LICENSE file for more details.

