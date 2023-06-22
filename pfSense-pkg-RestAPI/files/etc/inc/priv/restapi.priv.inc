<?php
global $priv_list;

$priv_list['page-system-restapi'] = array();
$priv_list['page-system-restapi']['name'] = "WebCfg - System: RestAPI";
$priv_list['page-system-restapi']['descr'] = "Allow access to RestAPI configuration";
$priv_list['page-system-restapi']['match'] = array();
$priv_list['page-system-restapi']['match'][] = "/restapi/admin/credentials.php";
$priv_list['page-system-restapi']['match'][] = "/restapi/admin/logs.php";
$priv_list['page-system-restapi']['match'][] = "/restapi/admin/about.php";

?>
