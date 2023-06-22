<?php
include_once('util.inc');
include_once('guiconfig.inc');

$restapi_credentials_ini_file = '/etc/restapi/credentials.ini';

$pgtitle = array(gettext('System'), gettext('RestAPI'), gettext('Credentials'));
include_once('head.inc');

$tab_array   = array();
$tab_array[] = array(gettext("Credentials"), true, "/restapi/admin/credentials.php");
$tab_array[] = array(gettext("Logs"), false, "/restapi/admin/logs.php");
$tab_array[] = array(gettext("About"), false, "/restapi/admin/about.php");
display_top_tabs($tab_array, true);

function restapi_load_credentials_ini($filename) {
    $ini_credentials = parse_ini_file($filename, TRUE);
    $credentials = array();
    foreach($ini_credentials as $ini_section => $ini_section_items) {
        if(isset($ini_section_items['secret'])) {
            if(!isset($ini_section_items['permit'])) {
                $ini_section_items['permit'] = '&lt;none&gt;';
            }
            
            if(array_key_exists('comment', $ini_section_items)) {
                $comment = $ini_section_items['comment'];
            }
            elseif(array_key_exists('owner', $ini_section_items)) {
                // legacy 
                $comment = $ini_section_items['owner'];
            }
            else {
                $comment = '-';
            }
            
            $credentials[] = array(
                'apikey' => $ini_section,
                'permits' => explode(',',str_replace(' ', '', $ini_section_items['permit'])),
                'comment' => $comment
            );
        }
    }
    return $credentials;
}

?>

<div class="panel panel-default">
    <div class="panel-heading">
        <h2 class="panel-title">RestAPI credentials</h2>
    </div>
    <div class="panel-body">
        <div class="table-responsive">
            <table class="table table-striped table-hover table-compact sortable-theme-bootstrap" data-sortable>
                <thead>
                    <tr>
                        <th>key</th>
                        <th>secret</th>
                        <th>permits</th>
                        <th>comment</th>
                    </tr>
                </thead>
                <tbody>
                    <?php
                    foreach(restapi_load_credentials_ini($restapi_credentials_ini_file) as $credential) {
                        print '<tr>';
                        print '<td><div style="font-family:monospace;">'.$credential['apikey'].'</div></td>';
                        print '<td><div style="font-family:monospace;">[hidden]</div></td>';
                        print '<td><div style="font-family:monospace;">';
                        foreach($credential['permits'] as $permit){
                            print $permit.'<br />';
                        } 
                        print '</div></td>';
                        print '<td><div style="font-family:monospace;">'.$credential['comment'].'</div></td>';
                        print '</tr>';
                    }
                    ?>
                </tbody>
            </table>
        </div>
    </div>
</div>

<?php 
    include('foot.inc');
?>