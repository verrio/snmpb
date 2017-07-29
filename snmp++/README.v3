
README.v3 (SNMP++ V3.3)
=======================

SNMP++ up to version 2.8 was designed by Peter Mellquist
(Hewlett Packard Co.) and was available from
http://rosegarden.external.hp.com/snmp++. This link no longer works
because HP discontinued SNMP++. You can download the patched version 2.8a
still from http://www.agentpp.com/snmp++v2.8a.tar.gz.

SNMP++v3.3 adds SNMPv3 support and many other features. It can be used at 
least on the following platforms:

Linux, FreeBSD, Mac OS X (10.8), Solaris 11, Windows XP/7/8 (VS 2012), 
and CygWin.
Many others, including embedded systems, are supported as well
but compatibility is not guaranteed.
You are welcome to send fixes for your platform to
support@agentpp.com.

If you are NOT using autoconf (see README.autoconf), then edit
the file include/snmp_pp/config_snmp_pp.h before compiling
the library. 

In order to use SNMPv3 you need at least one of the following
crypto libraries:
- OpenSSL: This is the default library, so no configuration
  changes for snmp++ are needed. The define HAVE_LIBSSL must
  be kept enabled in config_snmp_pp.h. SHA1, MD5, DES and AES
  will be used from OpenSSL.
  Autoconf will use OpenSSL by default, if it is detected.
- libdes: The define HAVE_LIBDES must be enabled in config_snmp_pp.h
  and HAVE_LIBSSL and HAVE_LIBTOMCRYPT must be disabled. In this case
  the MD5 and SHA1 algorithms that are included in snmp++ are used.
  AES encryption is not available.
- libtomcrypt: The define HAVE_LIBTOMCRYPT must be enabled in
  config_snmp_pp.h and HAVE_LIBDES must be disabled. SHA1, MD5, DES
  and AES will be used from libtomcrypt.

IDEA is protected by International copyright law and in 
addition has been patented in several countries. The 
non-commercial use of the IDEA algorithm is free. However, 
whenever you or your company sells any products including 
the IDEA algorithm it needs a license granted by MediaCrypt
for many European countries, the U.S. and Japan.

Please visit the following site for more information:
http://www.mediacrypt.com/engl/Content/patent_info.htm
