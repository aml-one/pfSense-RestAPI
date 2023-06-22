<?php
namespace restapi\v1;
define('RESTAPI_CALLID', uniqid());

include_once('/etc/inc/restapi/restapi.inc');

$action = (string)filter_input(INPUT_GET, 'action');
if(empty($action)) { 
    $action = 'undefined'; 
}

$restapi = new restApi();
$response = $restapi->$action($_GET, file_get_contents("php://input"));

http_response_code($response->http_code);
if(!empty($response->action)) {
    header('restapi-callid: ' . RESTAPI_CALLID);
}
header('Content-Type: application/json');

unset($response->http_code);
echo json_encode($response);
