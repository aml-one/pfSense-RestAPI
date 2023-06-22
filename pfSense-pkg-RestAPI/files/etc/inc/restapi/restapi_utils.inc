<?php
namespace restapi\v1;
if (!defined('RESTAPI_CALLID')) { echo 'RESTAPI_CALLID missing'; exit; };

class restApiUtils {

    /**
     * sanitize()
     * 
     * @param mixed $input
     * @param array $allowed
     * @param int $__recurse_count
     * @param int $__recurse_limit
     * @return mixed
     * @throws Exception
     * 
     * Based on CakePHP 2.1 paranoid mode sanitize function:-
     * http://api.cakephp.org/2.1/source-class-Sanitize.html#24-264
     */
    public static function sanitize($input, $allowed = array(), $__recurse_count = 0, $__recurse_limit = 10) {
        
        if ($__recurse_count > $__recurse_limit) {
            throw new \Exception('FATAL: recusion limit reached in sanitize()');
        }

        $allow = null;
        if (!empty($allowed)) {
            foreach ($allowed as $value) {
                $allow .= "\\$value";
            }
        }

        if (is_array($input)) {
            $cleaned = array();
            foreach ($input as $key => $clean) {
                $cleaned[$key] = restApiUtils::sanitize($clean, $allowed, $__recurse_count + 1);
            }
        } else {
            $cleaned = preg_replace("/[^{$allow}a-zA-Z0-9]/", '', $input);
        }
        return $cleaned;
    }
    
    /**
     * get_client_ipaddr()
     * @return string
     */
    public static function get_client_ipaddr() {
        $ipaddress = NULL;
        
        if(getenv('HTTP_CLIENT_IP')) {
            $ipaddress = getenv('HTTP_CLIENT_IP');
        }
        elseif(getenv('HTTP_X_FORWARDED_FOR')){
            $ipaddress = getenv('HTTP_X_FORWARDED_FOR');
        }
        elseif(getenv('HTTP_X_FORWARDED')) {
            $ipaddress = getenv('HTTP_X_FORWARDED');
        }
        elseif(getenv('HTTP_FORWARDED_FOR')) {
            $ipaddress = getenv('HTTP_FORWARDED_FOR');
        }
        elseif(getenv('HTTP_FORWARDED')) {
            $ipaddress = getenv('HTTP_FORWARDED');
        }
        elseif(getenv('REMOTE_ADDR')) {
            $ipaddress = getenv('REMOTE_ADDR');
        } else {
            $ipaddress = 'UNKNOWN';
        }
        return $ipaddress;
    }
    
    /**
     * get_request_scheme()
     * @return string
     */
    public static function get_request_scheme() {
        $scheme = NULL;
        
        if(getenv('REQUEST_SCHEME')) {
            $scheme = strtolower(getenv('REQUEST_SCHEME'));
        }
        elseif(getenv('HTTPS') && strtolower(getenv('HTTPS')) !== 'off') {
            $scheme = 'https';
        }
        elseif(getenv('SERVER_PORT') && (int)getenv('SERVER_PORT') === 443) {
            $scheme = 'https';
        } else {
            $scheme = 'http';
        }
        return $scheme;
    }

    /**
     * get_request_port()
     * @return int
     */
    public static function get_request_port() {
        return (int) getenv('SERVER_PORT');
    }
    
    /**
     * get_request_ipaddr()
     * @return string
     */
    public static function get_request_ipaddr() {
        $ipaddress = NULL;
            
        if(getenv('SERVER_ADDR')) {
            $ipaddress = getenv('SERVER_ADDR');
        } else {
            $ipaddress = 'UNKNOWN';
        }
        return $ipaddress;
    }
}
