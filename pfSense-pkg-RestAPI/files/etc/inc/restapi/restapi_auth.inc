<?php
namespace restapi\v1;
if (!defined('RESTAPI_CALLID')) { echo 'RESTAPI_CALLID missing'; exit; };

class restApiAuth {

    private $credentials = array();
    private $api_credentials_file = '/etc/restapi/credentials.ini';
    private $api_timestamp_delta_max = 60;
    private $api_demo_credentials = array(
        'RESTexample01' => 'abcdefghijklmnopqrstuvwxyz0123456789abcd',
        'RESTexample02' => 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCD',
    );

    /**
     * is_authenticated()
     * 
     * @return boolean
     */
    public function is_authenticated() {
        restApiLogger::debug(__METHOD__);

        if (!isset($_SERVER['HTTP_RESTAPI_AUTH'])) {
            restApiLogger::error('request missing RESTAPI_AUTH header value');
            return FALSE;
        }

        // make sure the AUTH is well formed and has expected input
        $elements = explode(':', restApiUtils::sanitize($_SERVER['HTTP_RESTAPI_AUTH'], array(':')));
        if (4 !== count($elements)) {
            restApiLogger::error('unexpected number of RESTAPI_AUTH elements supplied', array(
                $elements
            ));
            return FALSE;
        }

        $auth = array(
            'apikey' => $elements[0],
            'timestamp' => $elements[1],
            'nonce' => $elements[2],
            'hash' => $elements[3],
        );
        
        if(!$this->load_credentials($this->api_credentials_file, $auth['apikey'])) {
            return FALSE;
        }
        
        // make sure demo credentials are not in use
        foreach($this->api_demo_credentials as $apidemo_key => $apidemo_secret) {
            if($apidemo_key === $auth['apikey'] || $apidemo_secret === $this->credentials[RESTAPI_CALLID]['secret']) {
                restApiLogger::error('demo api credential value in use, these MUST be changed!');
                return FALSE;
            }
        }

        // confirm the timestamp is valid
        $caller_ts = strtotime($auth['timestamp']);
        $system_ts = time();
        
        if ($system_ts < ($caller_ts - $this->api_timestamp_delta_max) || 
                $system_ts > ($caller_ts + $this->api_timestamp_delta_max)) {
            restApiLogger::error('timestamp provided is out-of-bounds', array(
                'caller_time' => gmdate('Ymd\ZHis', $caller_ts), 
                'system_time' => gmdate('Ymd\ZHis', $system_ts)
            ));
            return FALSE;
        }

        // TODO - nonce
        // these nonce values really should be checked to ensure requests are not
        // replayed by a third party, should be easy enough to do with files
        // contained in a /tmp/restapi/nonce path for example

        // confirm the nonce is valid
        if (strlen($auth['nonce']) < 8 || strlen($auth['nonce']) > 40) {
            restApiLogger::error('length of nonce value is out-of-bounds');
            return FALSE;
        }
      
        // confirm the api credentials now in use meet the rules:-
        //  - NB1: refer to restApiUtils::sanitize() above, hence alpha numeric only
        //  - NB2: <apikey_value> MUST start with the prefix REST (pfSense Faux API)
        //  - NB3: <apikey_value> MUST be >= 12 chars AND <= 40 chars in total length
        //  - NB4: <apisecret_value> MUST be >= 40 chars AND <= 128 chars in length
        if('REST' !== substr($auth['apikey'], 0, 4) || strlen($auth['apikey']) < 12 || strlen($auth['apikey']) > 40) {
            restApiLogger::error('apikey is out-of-bounds, refer to documentation');
            return FALSE;
        }
        if(strlen($this->credentials[RESTAPI_CALLID]['secret']) < 40 || strlen($this->credentials[RESTAPI_CALLID]['secret']) > 128) {
            restApiLogger::error('apisecret is out-of-bounds, refer to documentation');
            return FALSE;
        }

        // confirm the hash is valid
        $auth_hash_input = $this->credentials[RESTAPI_CALLID]['secret'] . $auth['timestamp'] . $auth['nonce'];
        if (hash('sha256', $auth_hash_input) !== $auth['hash']) {
            restApiLogger::error('invalid hash value provided');
            return FALSE;
        }
        
        define('RESTAPI_APIKEY', $auth['apikey']);
        restApiLogger::debug('valid auth for call', array(
            'apikey' => RESTAPI_APIKEY,
            'callid' => RESTAPI_CALLID,
            'client_ip' => restApiUtils::get_client_ipaddr()
        ));
        return TRUE;
    }
    
    /**
     * is_authorized()
     * 
     * @param string $action
     * @return bool
     */
    public function is_authorized($action) {
        restApiLogger::debug(__METHOD__);
        
        if (!array_key_exists(RESTAPI_CALLID, $this->credentials)) {
            restApiLogger::error('credentials for this action have not been established');
            return FALSE;
        }
        
        $permits = explode(',',str_replace(' ', '', $this->credentials[RESTAPI_CALLID]['permit']));
        foreach($permits as $permit){
            if(fnmatch($permit, $action)) {
                restApiLogger::debug('permit allows action', array(
                    'action' => $action,
                    'permit' => $permit,
                    'permits' => $permits,
                ));
                return TRUE;
            }
        }
        restApiLogger::debug('permits do NOT allow action!', array(
            'action' => $action,
            'permits' => $permits,
        ));
        return FALSE;
    }
    
    /**
     * load_credentials()
     * 
     * @param string $filename
     * @return mixed
     */
    private function load_credentials($filename, $apikey) {
        restApiLogger::debug(__METHOD__);
        
        if(!is_file($filename)) {
            restApiLogger::error('unable to find restapi credentials file', array(
                'filename' => $filename
            ));
            return FALSE;
        }
        
        $ini_credentials = parse_ini_file($filename, TRUE);
        
        $credentials = array();
        foreach($ini_credentials as $key => $ini) {
            if(isset($ini['secret']) && !array_key_exists($key, $this->api_demo_credentials)) {
                $credentials[$key] = array(
                    'secret' => $ini['secret'],
                    'permit' => array_key_exists('permit', $ini) ? $ini['permit'] : ''
                );
            }
        }
        
        if (array_key_exists($apikey, $credentials)) {
            $this->credentials[RESTAPI_CALLID] = $credentials[$apikey];
            return TRUE;
        }
        
        restApiLogger::error('apikey not defined in credential file', array(
            'apikey' => $apikey,
            'filename' => $filename
        ));
        return FALSE;
    }

}
