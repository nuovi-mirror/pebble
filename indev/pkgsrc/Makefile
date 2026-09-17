DISTNAME = 20260915
EXTRACT_SUFX = .tar
PKGNAME = cpebble-snapshot-20260915
CATEGORIES = lang
MAINTAINER = vantheman@thenuoviorizzonticomapny.org
HOMEPAGE = http://pebblevm.org
COMMENT = A small, portable, and ultralight virtual machine and language runtime
LICENSE = nuovi-openlicense-v3
#MASTER_SITES = http://pebblevm.org/downloads/cpebble/
MASTER_SITES = http://pebblevm.org/downloads/cpebble/snapshot/
USE_LANGUAGES = c99
WRKSRC = ${WRKDIR}
INSTALLATION_DIRS = bin

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/bin/vm \
		${DESTDIR}${PREFIX}/bin/pebble

.include "../../mk/bsd.pkg.mk"
