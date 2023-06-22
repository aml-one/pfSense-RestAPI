<?php
namespace restapi\v1;
if (!defined('RESTAPI_CALLID')) { echo 'RESTAPI_CALLID missing'; exit; };

/**
 * quasi-private global $__restapi_logs variable to store log messages
 */
$__restapi_logs = array();

class restApiLogger {
    
    public static $log_count_limit = 100;
    public static $syslog_filename = 'restapi.log';

    public static function debug($message, $data=NULL) {
        restApiLogger::write_syslog(
            LOG_DEBUG,
            restApiLogger::append_restapi_logs_entry('debug', $message, $data)
        );
    }

    public static function info($message, $data=NULL) {
        restApiLogger::write_syslog(
            LOG_INFO,
            restApiLogger::append_restapi_logs_entry('info', $message, $data)
        );
    }

    public static function warn($message, $data=NULL) {
        restApiLogger::write_syslog(
            LOG_WARNING,
            restApiLogger::append_restapi_logs_entry('warn', $message, $data)
        );
    }

    public static function error($message, $data=NULL) {
        restApiLogger::write_syslog(
            LOG_ERR,
            restApiLogger::append_restapi_logs_entry('error', $message, $data)
        );
    }
    
    public static function get_logs($include_debug=FALSE) {
        global $__restapi_logs;
        $logs = array();
        foreach($__restapi_logs as $log) {
            if(array_key_exists('DEBUG', $log) && TRUE === $include_debug) {
                $logs[] = $log;
            }
            elseif(!array_key_exists('DEBUG', $log)) {
                $logs[] = $log;
            }
        }
        return $logs;
    }
    
    private static function write_syslog($level, $log, $inject_source=TRUE) {
        
        #openlog(restApiLogger::$syslog_filename, LOG_PID | LOG_LOCAL0);
        #closelog();
        
        if($inject_source) {
            $log['source'] = 'restapi';
        }
        syslog($level, json_encode($log));
    }
    
    private static function append_restapi_logs_entry($type, $message, $data) {
        global $__restapi_logs;
        $ts = date('Ymd\ZHis', time()); // zulu time
        if(is_null($data)){
            $log[strtoupper($type)] = $ts.' :: '.$message;
        } else {
            $log[strtoupper($type)] = $ts.' :: '.$message;
            $log['DATA'] = $data;
        }
        $__restapi_logs[] = $log;
        
        if(count($__restapi_logs) > restApiLogger::$log_count_limit) {
            array_shift($__restapi_logs);
        }
        return $log;
    }

}
