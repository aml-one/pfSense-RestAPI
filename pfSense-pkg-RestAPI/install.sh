#!/bin/sh

PORTNAME=pfSense-pkg-RestAPI
FILESDIR=files/
PREFIX=/usr/local
DATADIR=${PREFIX}/share/${PORTNAME}

mkdir -p /${DATADIR}; \
mkdir -p /${PREFIX}/pkg; \
mkdir -p /etc/inc/priv; \
mkdir -p /etc/restapi; \
mkdir -p /etc/inc/restapi; \
mkdir -p /${PREFIX}/www/restapi/v1; \
mkdir -p /${PREFIX}/www/restapi/admin; \


cp ${FILESDIR}${PREFIX}/pkg/restapi.xml \
                ${PREFIX}/pkg

cp ${FILESDIR}/etc/inc/priv/restapi.priv.inc \
                /etc/inc/priv

cp ${FILESDIR}/etc/restapi/credentials.sample.ini \
                /etc/restapi

cp ${FILESDIR}${PREFIX}/www/restapi/v1/index.php \
                ${PREFIX}/www/restapi/v1

cp ${FILESDIR}${PREFIX}/www/restapi/admin/about.php \
                ${PREFIX}/www/restapi/admin

cp ${FILESDIR}${PREFIX}/www/restapi/admin/credentials.php \
                ${PREFIX}/www/restapi/admin

cp ${FILESDIR}${PREFIX}/www/restapi/admin/logs.php \
                ${PREFIX}/www/restapi/admin

cp ${FILESDIR}/etc/inc/restapi/restapi.inc \
                /etc/inc/restapi

cp ${FILESDIR}/etc/inc/restapi/restapi_actions.inc \
                /etc/inc/restapi

cp ${FILESDIR}/etc/inc/restapi/restapi_auth.inc \
                /etc/inc/restapi

cp ${FILESDIR}/etc/inc/restapi/restapi_logger.inc \
                /etc/inc/restapi

cp ${FILESDIR}/etc/inc/restapi/restapi_pfsense_interface.inc \
                /etc/inc/restapi

cp ${FILESDIR}/etc/inc/restapi/restapi_utils.inc \
                /etc/inc/restapi

cp ${FILESDIR}${DATADIR}/info.xml \
		${DATADIR}

/usr/local/bin/php -f /etc/rc.packages ${PORTNAME} POST-INSTALL
