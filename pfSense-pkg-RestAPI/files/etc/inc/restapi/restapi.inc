<?php
namespace restapi\v1;
if (!defined('RESTAPI_CALLID')) { echo 'RESTAPI_CALLID missing'; exit; };

include_once('restapi_logger.inc');
include_once('restapi_utils.inc');
include_once('restapi_auth.inc');
include_once('restapi_actions.inc');
include_once('restapi_pfsense_interface.inc');

class restApi {
    
    // NB: setting an input URL parameter __debug=true will cause this $debug 
    // to be set true within the __check_user_action_call() function
    public $debug = NULL;
    
    private $Auth = NULL;

    /**
     * __construct()
     */
    public function __construct($debug=FALSE) {
        $this->debug = $debug;
        $this->Auth = new restApiAuth();
    }

    /**
     * __call
     * @param string $user_action
     * @param array $user_args
     * @throws Exception
     */
    public function __call($user_action, $call_args) {
        
        // info-level log only if a non GET request
        if('GET' !== getenv('REQUEST_METHOD')) {
            $logger_function = 'info';
        } else {
            $logger_function = 'debug';
        }
        
        restApiLogger::$logger_function(__METHOD__, array(
            'user_action' => $user_action,
            'callid' => RESTAPI_CALLID,
            'client_ip' => restApiUtils::get_client_ipaddr()
        ));
        
        $Actions = new restApiActions();
        
        // check this user action request
        $checked = $this->__check_user_action_call($Actions, $user_action, $call_args[0], $call_args[1]);
        
        // create a $response object to work with below
        $response = (object) array(
            'callid' => RESTAPI_CALLID,
            'http_code' => NULL, 
            'action' => NULL,
            'message' => NULL,
            'data' => NULL,
            'logs' => NULL);
        
        if(!empty($checked->http_code)) {
            $response->message = $checked->message;
            $response->http_code = $checked->http_code;
            
        } else {
            $Actions->response = &$response;
            $Actions->action_input_data = &$checked->data;
            
            $response->action = $checked_action = $checked->action;
            $Actions->$checked_action($checked->args);
            
            if(empty($response->http_code)) {
                $response->http_code = 500;
                $response->message = 'unexpected call response status';
            }
        }
        
        if (200 !== $response->http_code || TRUE === $this->debug) {
            if(empty($response->action)) {
                // feels prudent to provide limited return data in this case, also feels messy
                unset($response->logs);
                unset($response->action);
            } else {
                $response->logs = restApiLogger::get_logs($this->debug);
            }
        } else {
            unset($response->logs);
        }
        
        if(is_null($response->data)) {
            unset($response->data);
        }
        
        return $response;
    }
    
    /**
     * __check_user_action_call()
     * 
     * @param string $user_action
     * @param array $user_args
     * @param string $user_data
     * @return object
     */
    private function __check_user_action_call($Actions, $user_action, $user_args, $user_data) {
        restApiLogger::debug(__METHOD__);
        
        // create a $response object to work with below
        $checked = (object) array(
            'http_code' => NULL, 
            'message' => NULL, 
            'action'=> NULL, 
            'args' => array(),
            'data' => NULL
        );
        
        // confirm request is authenticated before we get too involved with it
        if (TRUE !== $this->Auth->is_authenticated()) {
            $checked->http_code = 401;
            $checked->message = 'authentication failed';
            restApiLogger::error($checked->message);
            return $checked;
        }
        
        // confirm the user action data does not need to be scrubbed
        $permitted = array('_');
        $checked->action = restApiUtils::sanitize((string)$user_action, $permitted);
        if($user_action !== $checked->action) {
            $checked->http_code = 400;
            $checked->message = 'user action name contains non-permitted values';
            restApiLogger::error($checked->message, array(
                'user_action' => $user_action,
                'checked_action' => $checked->action
            ));
            return $checked;
        }

        // confirm the method being called actually exists
        if (!method_exists($Actions, $checked->action)) {
            $checked->http_code = 404;
            $checked->message = 'api action is not defined';
            restApiLogger::error($checked->message, array(
                'action' => $checked->action
            ));
            return $checked;
        }
        
        // check the request is authorized to be performed
        if (TRUE !== $this->Auth->is_authorized($checked->action)) {
            $checked->http_code = 401;
            $checked->message = 'action authorization failed';
            restApiLogger::error($checked->message, array(
                'action' => $checked->action
            ));
            return $checked;
        }
        
        // confirm any user_data provided is valid JSON
        if(!empty($user_data)) {
            $checked->data = @json_decode($user_data, TRUE);
            $json_last_error = json_last_error();
            if(NULL === $checked->data && $json_last_error !== JSON_ERROR_NONE) {
                $checked->http_code = 400;
                $checked->message = 'user action data is not valid JSON';
                restApiLogger::error($checked->message, array(
                    'json_last_error' => $json_last_error
                ));
                return $checked;
            }
        }
        
        // sanity check and clean up the user args provided direct from $_GET
        foreach($user_args as $arg_key=>$arg_value) {
            $permitted = array(' ','_','-','.','/');
            if($arg_key !== restApiUtils::sanitize($arg_key, $permitted) || $arg_value !== restApiUtils::sanitize($arg_value, $permitted)) {
                $checked->http_code = 400;
                $checked->message = 'user action arguments contain non-permitted values';
                restApiLogger::error($checked->message, array(
                    'arg_key' => $arg_key,
                    'arg_value' => $arg_value,
                ));
                return $checked;
            }
            if($arg_key !== 'action') {
                if('true'=== strtolower($arg_value)) {
                    $arg_value = TRUE;
                }
                if('false' === strtolower($arg_value)) {
                    $arg_value = FALSE;
                }
                if('__debug' === $arg_key) {
                    $this->debug = $arg_value;
                }
                else {
                    $checked->args[$arg_key] = $arg_value;
                }
            }
        }
        
        restApiLogger::debug(__METHOD__.'() checks all passed');
        return $checked;
    }
}
