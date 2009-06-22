DISTNAME=	snmpb0.8
PKGREVISION=	1
CATEGORIES=	x11 net 
MASTER_SITES=	http://downloads.sf.net/snmpb/
EXTRACT_SUFX=   .tar.bz2

MAINTAINER=	snmpb1@gmail.com
HOMEPAGE=	http://sourceforge.net/projects/snmpb
COMMENT=	Graphical SNMP MIB browser written in QT.

USE_TOOLS+=	gmake
USE_LANGUAGES=	c c++
USE_LIBTOOL=	NO
WRKSRC     = /usr/pkgsrc/net/snmpb/work/snmpb0.8
CHECK_PORTABILITY_SKIP+=libsmi/*
MAKE_FLAGS +=   INSTALL_PREFIX=${PREFIX}

.include "../../x11/qt4-libs/buildlink3.mk"
.include "../../x11/qt4-tools/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
