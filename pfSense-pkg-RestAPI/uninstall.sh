#!/bin/sh

PORTNAME=pfSense-pkg-RestAPI
PREFIX=/usr/local
DATADIR=${PREFIX}/share/${PORTNAME}

/usr/local/bin/php -f /etc/rc.packages ${PORTNAME} DEINSTALL

rm -Rf /${DATADIR}
rm -Rf /${PREFIX}/pkg/restapi.xml
rm -Rf /etc/inc/priv/restapi.priv.inc
rm -Rf /etc/restapi
rm -Rf /etc/inc/restapi
rm -Rf /cf/conf/restapi
rm -Rf /${PREFIX}/www/restapi

/usr/local/bin/php -f /etc/rc.packages ${PORTNAME} POST-DEINSTALL
