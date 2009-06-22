The NetBSD installer needs to be created from the NetBSD
pkgsrc system. Download it (for instance, the 2008 4th quarter) from here: 
ftp://ftp.netbsd.org/pub/pkgsrc/pkgsrc-2009Q1/pkgsrc.tar.bz2
and install it in the /usr directory.

First, make sure to update the file snmpb/distinfo from the SVN HEAD
to get the correct SHA1 checksum or download the source tarball manually,
compute it using the sha1sum tool and update the snmpb/distinfo file.

Copy the snmpb folder in the current directory to the 'net'
folder in the pkgsrc directory, like this:

(As root)
cp -R snmpb /usr/pkgsrc/net/snmpb

To create the package, cd in /usr/pkgsrc/net/snmpb and (as root) type 'make'

then type 'make package' to create the package.
It will be put in ../../packages/All/

Martin Jolicoeur, 2009

