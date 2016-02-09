The NetBSD installer needs to be created from the NetBSD
pkgsrc system. Download it (for instance, the 2015 4th quarter) from here: 
ftp://ftp.netbsd.org/pub/pkgsrc/pkgsrc-2015Q4/pkgsrc.tar.bz2
and install it in the /usr directory.

Copy the snmpb folder in the current directory to the 'net'
folder in the pkgsrc directory, like this:

(As root)
cp -R snmpb /usr/pkgsrc/net/snmpb

To create the package, cd in /usr/pkgsrc/net/snmpb and (as root) type 'make NO_CHECKSUM=yes'

then type 'make package' to create the package.
It will be put in ../../packages/All/

Tested on NetBSD 7.0 x86 64 bits

Martin Jolicoeur, 2016

