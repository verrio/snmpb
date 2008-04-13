To compile:

# make (or gmake)

To install in places other than /usr, add INSTALL_PREFIX=<prefix> to the make command.

Tested & compiles on Cygwin/Windows, Linux and NetBSD

--------------------------------

Required installed packages for compilation:
- bison & flex
- GNU make
- QT4 development package
- GNU install
- automake 1.4 (won't work with later versions)
- gcc

External packages
------------------

Snmp++ is taken from http://www.agentpp.com
Version: 3.2.21

Modified file(s):
config_snmp_pp.h

Libtomcrypt is taken from http://libtomcrypt.com
Version: 1.14

Modified file(s):
libtomcrypt/makefile

Libsmi is taken from http://www.ibr.cs.tu-bs.de/projects/libsmi
Version: 0.4.5

Modified file(s):
win/Makefile.mingw
win/config.h
lib/smi.c "Fixed a bug on paths with spaces in smiReadConfig()"

QWT is taken from http://qwt.sourceforge.net
Version: 5.0.0rc0 (Release Candidate 0)

Modified file(s):
qwt.pro

--------------------------------

Martin Jolicoeur, April 2008
