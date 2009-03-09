The NetBSD installer needs to created from the NetBSD
pkgsrc system. Download it (for instance) from here: 
ftp://ftp.netbsd.org/pub/pkgsrc/pkgsrc-2008Q4/pkgsrc-2008Q4.tar.bz2
and install it in the /usr/pkgsrc directory.

Create a softlink from the 'net' directory in the pkgsrc folder to
the snmpb folder in the current directory, like this:

ln -s snmpb /usr/pkgsrc/net/snmpb

To create the package, cd in /usr/pkgsrc/net/snmpb and type 'make'

Martin Jolicoeur, 2009

