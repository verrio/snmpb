snmpb client
============

snmpb is a graphical SNMP client using the SNMP++ library,
libsmi parser and Qt framework.

![screenshot](/screenshot.png "screenshot")

This client can be used for querying the MIB of SNMPv1/v2c/v3 agents, receiving traps, discovering devices and editing MIB description files.  The current implementation has quite some memory leaks, but you won't be bothered by them, because by the time they'll fill up your memory the application will have segfaulted and crashed.

## Dependencies

Build time dependencies:
- autoconf and automake
- GNU make
- Qt5 development package
- GNU install
- gcc and g++

Runtime dependencies:
- [OpenSSL](https://www.openssl.org) v1.1.0 
- [libsmi](https://www.ibr.cs.tu-bs.de/projects/libsmi/) v0.5.0
- [Qt](https://www.qt.io) v5
- [QWT](http://qwt.sourceforge.net) v6.1.2

## Compilation

```sh
autoreconf -i
./configure
make
make install
etc.
```

Make sure to use the GNU make version (``gmake'' on *BSD).
To install in places other than /usr, add INSTALL_PREFIX=<prefix> to the make command.

## Installation

To install snmpb, type make install (as root) with the install prefix in argument:

```
make INSTALL_PREFIX=/usr install
```

default is /usr

## Licensing

This application is distributed under the GNU General Public License v2.  See the LICENSE file for more details.

