<div>
<!--READMESTART-->
<h1>
<a id="user-content-RestAPI---v10" class="anchor" href="#RestAPI---v10" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>pfSense RestAPI - v1.0</h1>
<p>A REST API interface for pfSense 2.3.x, 2.4.x, 2.5.x, 2.6.x to facilitate devops:-</p>
<ul>
<li>Based on: <a href="https://github.com/ndejong/pfsense_FauxAPI">FauxAPI</a> by Nicholas de Jong &copy; 2016</li>
</ul>

<h2>
<a id="user-content-api-action-summary" class="anchor" href="#api-action-summary" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>API Action Summary</h2>
<ul>
<li>
<a href="#user-content-config_backup">config_backup</a> - Causes the system to take a configuration backup and add it to the regular set of system change backups.</li>
<li>
<a href="#user-content-config_backup_list">config_backup_list</a> - Returns a list of the currently available system configuration backups.</li>
<li>
<a href="#user-content-config_get">config_get</a> - Returns the full system configuration as a JSON formatted string.</li>
<li>
<a href="#user-content-config_patch">config_patch</a> - Patch the system config with a granular piece of new configuration.</li>
<li>
<a href="#user-content-config_reload">config_reload</a> - Causes the pfSense system to perform an internal reload of the <code>config.xml</code> file.</li>
<li>
<a href="#user-content-config_restore">config_restore</a> - Restores the pfSense system to the named backup configuration.</li>
<li>
<a href="#user-content-config_set">config_set</a> - Sets a full system configuration and (by default) reloads once successfully written and tested.</li>
<li>
<a href="#user-content-gateway_status">gateway_status</a> - Returns gateway status data.</li>
<li>
<a href="#user-content-interface_stats">interface_stats</a> - Returns statistics and information about an interface.</li>
<li>
<a href="#user-content-rule_get">rule_get</a> - Returns the numbered list of loaded pf rules from a <code>pfctl -sr -vv</code> command on the pfSense host.</li>
<li>
<a href="#user-content-send_event">send_event</a> - Performs a pfSense "send_event" command to cause various pfSense system actions.</li>
<li>
<a href="#user-content-system_reboot">system_reboot</a> - Reboots the pfSense system.</li>
<li>
<a href="#user-content-system_stats">system_stats</a> - Returns various useful system stats.</li>
<li>
<a href="#user-content-system_info">system_info</a> - Returns various useful system info.</li>
<li>
<a href="#user-content-php-example">PHP Example Code</a> - quick example how to read / write a firewall / NAT rule</li>
</ul>
<h2>
<a id="user-content-approach" class="anchor" href="#approach" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>Approach</h2>
<p>At its core RestAPI simply reads the core pfSense <code>config.xml</code> file, converts it
to JSON and returns to the API caller.  Similarly it can take a JSON formatted
configuration and write it to the pfSense <code>config.xml</code> and handles the required
reload operations.  The ability to programmatically interface with a running
pfSense host(s) is enormously useful however it should also be obvious that this
provides the API user the ability to create configurations that can break your
pfSense system.</p>
<p>RestAPI provides easy backup and restore API interfaces that by default store
configuration backups on all configuration write operations thus it is very easy
to roll-back even if the API user manages to deploy a "very broken" configuration.</p>
<p>Multiple sanity checks take place to make sure a user provided JSON config will
correctly convert into the (slightly quirky) pfSense XML <code>config.xml</code> format and
then reload as expected in the same way.  However, because it is not a real
per-action application-layer interface it is still possible for the API caller
to create configuration changes that make no sense and can potentially disrupt
your pfSense system.</p>
<p>Because RestAPI is a utility that interfaces with the pfSense <code>config.xml</code> there
are some cases where reloading the configuration file is not enough and you
may need to "tickle" pfSense a little more to do what you want.  This is not
common however a good example is getting newly defined network interfaces or
VLANs to be recognized.  These situations are easily handled by calling the
<strong>send_event</strong> action with the payload <strong>interface reload all</strong> - see the example
included below</p>

<h2>
<a id="user-content-installation" class="anchor" href="#installation" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>Installation</h2>

<div class="highlight highlight-source-shell"><pre>fetch https://github.com/aml-one/pfSense-RestAPI/raw/main/releases/restapi_latest.tar.xz
tar -xf restapi_latest.tar.xz
cd pfSense-pkg-RestAPI
./install.sh
</pre></div>

<p><strong>NB:</strong> you MUST at least setup your <code>/etc/restapi/credentials.ini</code> file on the
pfSense host before you continue, see the API Authentication section below.</p>




<h2>
<a id="user-content-api-authentication" class="anchor" href="#api-authentication" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>API Authentication</h2>
<p>A deliberate design decision to decouple RestAPI authentication from both the
pfSense user authentication and the pfSense <code>config.xml</code> system.  This was done
to limit the possibility of an accidental API change that removes access to the
host.  It also seems more prudent to only establish API user(s) manually via the
RestAPI <code>/etc/restapi/credentials.ini</code> file.</p>
<p>The two sample RestAPI keys (RESTexample01 and RESTexample02) and their
associated secrets in the sample <code>credentials.sample.ini</code> file are hard-coded to
be inoperative, you must create entirely new values before your client scripts
will be able to issue commands to RestAPI.</p>
<p>You can start your own <code>/etc/restapi/credentials.ini</code> file by copying the sample
file provided in <code>credentials.sample.ini</code></p>
<p>API authentication itself is performed on a per-call basis with the auth value
inserted as an additional <strong>restapi-auth</strong> HTTP request header, it can be
calculated as such:-</p>
<pre><code>restapi-auth: &lt;apikey&gt;:&lt;timestamp&gt;:&lt;nonce&gt;:&lt;hash&gt;

For example:-
restapi-auth: REST4797d073:20161119Z144328:833a45d8:9c4f96ab042f5140386178618be1ae40adc68dd9fd6b158fb82c99f3aaa2bb55
</code></pre>
<p>Where the &lt;hash&gt; value is calculated like so:-</p>
<pre><code>&lt;hash&gt; = sha256(&lt;apisecret&gt;&lt;timestamp&gt;&lt;nonce&gt;)
</code></pre>
<p>NB: that the timestamp value is internally passed to the PHP <code>strtotime</code> function
which can interpret a wide variety of timestamp formats together with a timezone.
A nice tidy timestamp format that the <code>strtotime</code> PHP function is able to process
can be obtained using bash command <code>date --utc +%Y%m%dZ%H%M%S</code> where the <code>Z</code>
date-time seperator hence also specifies the UTC timezone.</p>

<p>Getting the API credentials right seems to be a common source of confusion in
getting started with RestAPI because the rules about valid API keys and secret
values are pedantic to help make ensure poor choices are not made.</p>
<p>The API key + API secret values that you will need to create in <code>/etc/restapi/credentials.ini</code>
have the following rules:-</p>
<ul>
<li>&lt;apikey_value&gt; and &lt;apisecret_value&gt; may have alphanumeric chars ONLY!</li>
<li>&lt;apikey_value&gt; MUST start with the prefix REST</li>
<li>&lt;apikey_value&gt; MUST be &gt;= 12 chars AND &lt;= 40 chars in total length</li>
<li>&lt;apisecret_value&gt; MUST be &gt;= 40 chars AND &lt;= 128 chars in length</li>
<li>you must not use the sample key/secret in the <code>credentials.ini</code> since they
are hard coded to fail.</li>
</ul>
<p>To make things easier consider using the following shell commands to generate
valid values:-</p>
<h4>
<a id="user-content-apikey_value" class="anchor" href="#apikey_value" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>apikey_value</h4>
<div class="highlight highlight-source-shell"><pre><span class="pl-c1">echo</span> REST<span class="pl-s"><span class="pl-pds">`</span>head /dev/urandom <span class="pl-k">|</span> base64 -w0 <span class="pl-k">|</span> tr -d /+= <span class="pl-k">|</span> head -c 20<span class="pl-pds">`</span></span></pre></div>
<h4>
<a id="user-content-apisecret_value" class="anchor" href="#apisecret_value" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>apisecret_value</h4>
<div class="highlight highlight-source-shell"><pre><span class="pl-c1">echo</span> <span class="pl-s"><span class="pl-pds">`</span>head /dev/urandom <span class="pl-k">|</span> base64 -w0 <span class="pl-k">|</span> tr -d /+= <span class="pl-k">|</span> head -c 60<span class="pl-pds">`</span></span></pre></div>
<p>NB: Make sure the client side clock is within 60 seconds of the pfSense host
clock else the auth token values calculated by the client will not be valid - 60
seconds seems tight, however, provided you are using NTP to look after your
system time it's quite unlikely to cause issues.</p>


<h2>
<a id="user-content-api-authorization" class="anchor" href="#api-authorization" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>API Authorization</h2>
<p>The file <code>/etc/restapi/credentials.ini</code> additionally provides a method to restrict
the API actions available to the API key using the <strong>permit</strong> configuration
parameter.  Permits are comma delimited and may contain * wildcards to match more
than one rule as shown in the example below.</p>
<pre><code>[RESTexample01]
secret = abcdefghijklmnopqrstuvwxyz0123456789abcd
permit = alias_*, config_*, gateway_*, rule_*, send_*, system_*, function_*
comment = example key RESTexample01 - hardcoded to be inoperative
</code></pre>
<h2>
<a id="user-content-debugging" class="anchor" href="#debugging" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>Debugging</h2>
<p>RestAPI comes with awesome debug logging capability, simply insert <code>__debug=true</code>
as a URL request parameter and the response data will contain rich debugging log
data about the flow of the request.</p>

<h2>
<a id="user-content-logging" class="anchor" href="#logging" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>Logging</h2>
<p>RestAPI actions are sent to the system syslog via a call to the PHP <code>syslog()</code>
function thus causing all RestAPI actions to be logged and auditable on a per
action (callid) basis which provide the full basis for the call, for example:-</p>
<pre lang="text"><code>Jul  3 04:37:59 pfSense php-fpm[55897]: {"INFO":"20180703Z043759 :: RestAPI\\v1\\RestAPI::__call","DATA":{"user_action":"alias_update_urltables","callid":"5b3afda73e7c9","client_ip":"192.168.1.5"},"source":"RestAPI"}
Jul  3 04:37:59 pfSense php-fpm[55897]: {"INFO":"20180703Z043759 :: valid auth for call","DATA":{"apikey":"RESTdevtrash","callid":"5b3afda73e7c9","client_ip":"192.168.1.5"},"source":"RestAPI"}
</code></pre>
<p>Enabling debugging yields considerably more logging data to assist with tracking
down issues if you encounter them - you may review the logs via the pfSense GUI
as usual under Status-&gt;System Logs-&gt;General or via the console using the <code>clog</code> tool</p>
<div class="highlight highlight-source-shell"><pre>$ clog /var/log/system.log <span class="pl-k">|</span> grep restapi</pre></div>
<h2>
<a id="user-content-configuration-backups" class="anchor" href="#configuration-backups" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>Configuration Backups</h2>
<p>All configuration edits through RestAPI create configuration backups in the
same way as pfSense does with the webapp GUI.</p>
<p>These backups are available in the same way as edits through the pfSense
GUI and are thus able to be reviewed and diff'd in the same way under
Diagnostics-&gt;Backup &amp; Restore-&gt;Config History.</p>
<p>Changes made through the RestAPI carry configuration change descriptions that
name the unique <code>callid</code> which can then be tied to logs if required for full
usage audit and change tracking.</p>
<p>RestAPI functions that cause write operations to the system config <code>config.xml</code>
return reference to a backup file of the configuration immediately previous
to the change.</p>
<h2>
<a id="user-content-api-rest-actions" class="anchor" href="#api-rest-actions" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>API REST Actions</h2>
<p>The following REST based API actions are provided, example cURL call request
examples are provided for each.</p>


<p>NB: the cURL requests below use the '--insecure' switch because many pfSense
deployments do not deploy certificate chain signed SSL certificates.  A reasonable
improvement in this regard might be to implement certificate pinning at the
client side to hence remove scope for man-in-middle concerns.</p>
<hr>

<h3>
<a id="user-content-config_backup" class="anchor" href="#config_backup" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>config_backup</h3>
<ul>
<li>Causes the system to take a configuration backup and add it to the regular
set of pfSense system backups at <code>/cf/conf/backup/</code>
</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params: none</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=config_backup<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"583012fea254f"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"config_backup"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
  <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
    <span class="pl-s">"backup_config_file"</span>: <span class="pl-s">"/cf/conf/backup/config-1479545598.xml"</span>
  <span class="pl-kos">}</span>
<span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-config_backup_list" class="anchor" href="#config_backup_list" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>config_backup_list</h3>
<ul>
<li>Returns a list of the currently available pfSense system configuration backups.</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params: none</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=config_backup_list<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"583065cb670db"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"config_backup_list"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
  <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
    <span class="pl-s">"backup_files"</span>: <span class="pl-kos">[</span>
      <span class="pl-kos">{</span>
        <span class="pl-s">"filename"</span>: <span class="pl-s">"/cf/conf/backup/config-1479545598.xml"</span><span class="pl-kos">,</span>
        <span class="pl-s">"timestamp"</span>: <span class="pl-s">"20161119Z144635"</span><span class="pl-kos">,</span>
        <span class="pl-s">"description"</span>: <span class="pl-s">"RestAPI-REST4797d073@192.168.10.10: update via RestAPI for callid: 583012fea254f"</span><span class="pl-kos">,</span>
        <span class="pl-s">"version"</span>: <span class="pl-s">"15.5"</span><span class="pl-kos">,</span>
        <span class="pl-s">"filesize"</span>: <span class="pl-c1">18535</span>
      <span class="pl-kos">}</span><span class="pl-kos">,</span>
      ...<span class="pl-kos">.</span></pre></div>
<hr>
<h3>
<a id="user-content-config_get" class="anchor" href="#config_get" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>config_get</h3>
<ul>
<li>Returns the system configuration as a JSON formatted string.  Additionally,
using the optional <strong>config_file</strong> parameter it is possible to retrieve backup
configurations by providing the full path to it under the <code>/cf/conf/backup</code>
path.</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params:
<ul>
<li>
<strong>config_file</strong> (optional, default=<code>/cf/config/config.xml</code>)</li>
</ul>
</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=config_get<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
    <span class="pl-s">"callid"</span>: <span class="pl-s">"583012fe39f79"</span><span class="pl-kos">,</span>
    <span class="pl-s">"action"</span>: <span class="pl-s">"config_get"</span><span class="pl-kos">,</span>
    <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
    <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
      <span class="pl-s">"config_file"</span>: <span class="pl-s">"/cf/conf/config.xml"</span><span class="pl-kos">,</span>
      <span class="pl-s">"config"</span>: <span class="pl-kos">{</span>
        <span class="pl-s">"version"</span>: <span class="pl-s">"15.5"</span><span class="pl-kos">,</span>
        <span class="pl-s">"staticroutes"</span>: <span class="pl-s">""</span><span class="pl-kos">,</span>
        <span class="pl-s">"snmpd"</span>: <span class="pl-kos">{</span>
          <span class="pl-s">"syscontact"</span>: <span class="pl-s">""</span><span class="pl-kos">,</span>
          <span class="pl-s">"rocommunity"</span>: <span class="pl-s">"public"</span><span class="pl-kos">,</span>
          <span class="pl-s">"syslocation"</span>: <span class="pl-s">""</span>
        <span class="pl-kos">}</span><span class="pl-kos">,</span>
        <span class="pl-s">"shaper"</span>: <span class="pl-s">""</span><span class="pl-kos">,</span>
        <span class="pl-s">"installedpackages"</span>: <span class="pl-kos">{</span>
          <span class="pl-s">"pfblockerngsouthamerica"</span>: <span class="pl-kos">{</span>
            <span class="pl-s">"config"</span>: <span class="pl-kos">[</span>
             ...<span class="pl-kos">.</span></pre></div>
<p>Hint: use <code>jq</code> to parse the response JSON and obtain the config only, as such:-</p>
<div class="highlight highlight-source-shell"><pre>cat /tmp/rest-config-get-output-from-curl.json <span class="pl-k">|</span> jq .data.config <span class="pl-k">&gt;</span> /tmp/config.json</pre></div>
<hr>
<h3>
<a id="user-content-config_patch" class="anchor" href="#config_patch" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>config_patch</h3>
<ul>
<li>Allows the API user to patch the system configuration with the existing system config</li>
<li>A <strong>config_patch</strong> call allows the API user to supply the partial configuration to be updated
which is quite different to the <strong>config_set</strong> function that requires the full configuration
to be posted.</li>
<li>HTTP: <strong>POST</strong>
</li>
<li>Params:
<ul>
<li>
<strong>do_backup</strong> (optional, default = true)</li>
<li>
<strong>do_reload</strong> (optional, default = true)</li>
</ul>
</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>cat <span class="pl-k">&gt;</span> /tmp/config_patch.json <span class="pl-s"><span class="pl-k">&lt;&lt;</span><span class="pl-k">EOF</span></span>
<span class="pl-s">{</span>
<span class="pl-s">  "system": {</span>
<span class="pl-s">    "dnsserver": [</span>
<span class="pl-s">      "8.8.8.8",</span>
<span class="pl-s">      "8.8.4.4"</span>
<span class="pl-s">    ],</span>
<span class="pl-s">    "hostname": "newhostname"</span>
<span class="pl-s">  }</span>
<span class="pl-s">}</span>
<span class="pl-s"><span class="pl-k">EOF</span></span>

curl \
    -X POST \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    --header <span class="pl-s"><span class="pl-pds">"</span>Content-Type: application/json<span class="pl-pds">"</span></span> \
    --data @/tmp/config_patch.json \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=config_patch<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"5b3b506f72670"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"config_patch"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
  <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
    <span class="pl-s">"do_backup"</span>: <span class="pl-c1">true</span><span class="pl-kos">,</span>
    <span class="pl-s">"do_reload"</span>: <span class="pl-c1">true</span><span class="pl-kos">,</span>
    <span class="pl-s">"previous_config_file"</span>: <span class="pl-s">"/cf/conf/backup/config-1530613871.xml"</span>
  <span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-config_reload" class="anchor" href="#config_reload" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>config_reload</h3>
<ul>
<li>Causes the pfSense system to perform a reload action of the <code>config.xml</code> file, by
default this happens when the <strong>config_set</strong> action occurs hence there is
normally no need to explicitly call this after a <strong>config_set</strong> action.</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params: none</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=config_reload<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"5831226e18326"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"config_reload"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span>
<span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-config_restore" class="anchor" href="#config_restore" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>config_restore</h3>
<ul>
<li>Restores the pfSense system to the named backup configuration.</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params:
<ul>
<li>
<strong>config_file</strong> (required, full path to the backup file to restore)</li>
</ul>
</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=config_restore&amp;config_file=/cf/conf/backup/config-1479545598.xml<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"583126192a789"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"config_restore"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
  <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
    <span class="pl-s">"config_file"</span>: <span class="pl-s">"/cf/conf/backup/config-1479545598.xml"</span>
  <span class="pl-kos">}</span>
<span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-config_set" class="anchor" href="#config_set" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>config_set</h3>
<ul>
<li>Sets a full system configuration and (by default) takes a system config
backup and (by default) causes the system config to be reloaded once
successfully written and tested.</li>
<li>NB1: be sure to pass the <em>FULL</em> system configuration here, not just the piece you
wish to adjust!  Consider the <strong>config_patch</strong> or <strong>config_item_set</strong> functions if
you wish to adjust the configuration in more granular ways.</li>
<li>NB2: if you are pulling down the result of a <code>config_get</code> call, be sure to parse that
response data to obtain the config data only under the key <code>.data.config</code>
</li>
<li>HTTP: <strong>POST</strong>
</li>
<li>Params:
<ul>
<li>
<strong>do_backup</strong> (optional, default = true)</li>
<li>
<strong>do_reload</strong> (optional, default = true)</li>
</ul>
</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X POST \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    --header <span class="pl-s"><span class="pl-pds">"</span>Content-Type: application/json<span class="pl-pds">"</span></span> \
    --data @/tmp/config.json \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=config_set<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"5b3b50e8b1bc6"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"config_set"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
  <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
    <span class="pl-s">"do_backup"</span>: <span class="pl-c1">true</span><span class="pl-kos">,</span>
    <span class="pl-s">"do_reload"</span>: <span class="pl-c1">true</span><span class="pl-kos">,</span>
    <span class="pl-s">"previous_config_file"</span>: <span class="pl-s">"/cf/conf/backup/config-1530613992.xml"</span>
  <span class="pl-kos">}</span>
<span class="pl-kos">}</span></pre></div>
<hr>

<h3>
<a id="user-content-gateway_status" class="anchor" href="#gateway_status" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>gateway_status</h3>
<ul>
<li>Returns gateway status data.</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params: none</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=gateway_status<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"598ecf3e7011e"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"gateway_status"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
  <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
    <span class="pl-s">"gateway_status"</span>: <span class="pl-kos">{</span>
      <span class="pl-s">"10.22.33.1"</span>: <span class="pl-kos">{</span>
        <span class="pl-s">"monitorip"</span>: <span class="pl-s">"8.8.8.8"</span><span class="pl-kos">,</span>
        <span class="pl-s">"srcip"</span>: <span class="pl-s">"10.22.33.100"</span><span class="pl-kos">,</span>
        <span class="pl-s">"name"</span>: <span class="pl-s">"GW_WAN"</span><span class="pl-kos">,</span>
        <span class="pl-s">"delay"</span>: <span class="pl-s">"4.415ms"</span><span class="pl-kos">,</span>
        <span class="pl-s">"stddev"</span>: <span class="pl-s">"3.239ms"</span><span class="pl-kos">,</span>
        <span class="pl-s">"loss"</span>: <span class="pl-s">"0.0%"</span><span class="pl-kos">,</span>
        <span class="pl-s">"status"</span>: <span class="pl-s">"none"</span>
      <span class="pl-kos">}</span>
    <span class="pl-kos">}</span>
  <span class="pl-kos">}</span>
<span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-interface_stats" class="anchor" href="#interface_stats" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>interface_stats</h3>
<ul>
<li>Returns interface statistics data and information - the real interface name must be provided
not an alias of the interface such as "WAN" or "LAN"</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params:
<ul>
<li>
<strong>interface</strong> (required)</li>
</ul>
</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=interface_stats&amp;interface=em0<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"5b3a5bce65d01"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"interface_stats"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
  <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
    <span class="pl-s">"stats"</span>: <span class="pl-kos">{</span>
      <span class="pl-s">"inpkts"</span>: <span class="pl-c1">267017</span><span class="pl-kos">,</span>
      <span class="pl-s">"inbytes"</span>: <span class="pl-c1">21133408</span><span class="pl-kos">,</span>
      <span class="pl-s">"outpkts"</span>: <span class="pl-c1">205860</span><span class="pl-kos">,</span>
      <span class="pl-s">"outbytes"</span>: <span class="pl-c1">8923046</span><span class="pl-kos">,</span>
      <span class="pl-s">"inerrs"</span>: <span class="pl-c1">0</span><span class="pl-kos">,</span>
      <span class="pl-s">"outerrs"</span>: <span class="pl-c1">0</span><span class="pl-kos">,</span>
      <span class="pl-s">"collisions"</span>: <span class="pl-c1">0</span><span class="pl-kos">,</span>
      <span class="pl-s">"inmcasts"</span>: <span class="pl-c1">61618</span><span class="pl-kos">,</span>
      <span class="pl-s">"outmcasts"</span>: <span class="pl-c1">73</span><span class="pl-kos">,</span>
      <span class="pl-s">"unsuppproto"</span>: <span class="pl-c1">0</span><span class="pl-kos">,</span>
      <span class="pl-s">"mtu"</span>: <span class="pl-c1">1500</span>
    <span class="pl-kos">}</span>
  <span class="pl-kos">}</span>
<span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-rule_get" class="anchor" href="#rule_get" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>rule_get</h3>
<ul>
<li>Returns the numbered list of loaded pf rules from a <code>pfctl -sr -vv</code> command
on the pfSense host.  An empty rule_number parameter causes all rules to be
returned.</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params:
<ul>
<li>
<strong>rule_number</strong> (optional, default = null)</li>
</ul>
</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=rule_get&amp;rule_number=5<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"583c279b56958"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"rule_get"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
  <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
    <span class="pl-s">"rules"</span>: <span class="pl-kos">[</span>
      <span class="pl-kos">{</span>
        <span class="pl-s">"number"</span>: <span class="pl-c1">5</span><span class="pl-kos">,</span>
        <span class="pl-s">"rule"</span>: <span class="pl-s">"anchor \"openvpn/*\" all"</span><span class="pl-kos">,</span>
        <span class="pl-s">"evaluations"</span>: <span class="pl-s">"14134"</span><span class="pl-kos">,</span>
        <span class="pl-s">"packets"</span>: <span class="pl-s">"0"</span><span class="pl-kos">,</span>
        <span class="pl-s">"bytes"</span>: <span class="pl-s">"0"</span><span class="pl-kos">,</span>
        <span class="pl-s">"states"</span>: <span class="pl-s">"0"</span><span class="pl-kos">,</span>
        <span class="pl-s">"inserted"</span>: <span class="pl-s">"21188"</span><span class="pl-kos">,</span>
        <span class="pl-s">"statecreations"</span>: <span class="pl-s">"0"</span>
      <span class="pl-kos">}</span>
    <span class="pl-kos">]</span>
  <span class="pl-kos">}</span>
<span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-send_event" class="anchor" href="#send_event" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>send_event</h3>
<ul>
<li>Performs a pfSense "send_event" command to cause various pfSense system
actions as is also available through the pfSense console interface.  The
following standard pfSense send_event combinations are permitted:-
<ul>
<li>filter: reload, sync</li>
<li>interface: all, newip, reconfigure</li>
<li>service: reload, restart, sync</li>
</ul>
</li>
<li>HTTP: <strong>POST</strong>
</li>
<li>Params: none</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X POST \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    --header <span class="pl-s"><span class="pl-pds">"</span>Content-Type: application/json<span class="pl-pds">"</span></span> \
    --data <span class="pl-s"><span class="pl-pds">"</span>[<span class="pl-cce">\"</span>interface reload all<span class="pl-cce">\"</span>]<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=send_event<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"58312bb3398bc"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"send_event"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span>
<span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-system_reboot" class="anchor" href="#system_reboot" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>system_reboot</h3>
<ul>
<li>Just as it says, reboots the system.</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params: none</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=system_reboot<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"58312bb3487ac"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"system_reboot"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span>
<span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-system_stats" class="anchor" href="#system_stats" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>system_stats</h3>
<ul>
<li>Returns various useful system stats.</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params: none</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=system_stats<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
  <span class="pl-s">"callid"</span>: <span class="pl-s">"5b3b511655589"</span><span class="pl-kos">,</span>
  <span class="pl-s">"action"</span>: <span class="pl-s">"system_stats"</span><span class="pl-kos">,</span>
  <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
  <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
    <span class="pl-s">"stats"</span>: <span class="pl-kos">{</span>
      <span class="pl-s">"cpu"</span>: <span class="pl-s">"20770421|20494981"</span><span class="pl-kos">,</span>
      <span class="pl-s">"mem"</span>: <span class="pl-s">"20"</span><span class="pl-kos">,</span>
      <span class="pl-s">"uptime"</span>: <span class="pl-s">"1 Day 21 Hours 25 Minutes 48 Seconds"</span><span class="pl-kos">,</span>
      <span class="pl-s">"pfstate"</span>: <span class="pl-s">"62/98000"</span><span class="pl-kos">,</span>
      <span class="pl-s">"pfstatepercent"</span>: <span class="pl-s">"0"</span><span class="pl-kos">,</span>
      <span class="pl-s">"temp"</span>: <span class="pl-s">""</span><span class="pl-kos">,</span>
      <span class="pl-s">"datetime"</span>: <span class="pl-s">"20180703Z103358"</span><span class="pl-kos">,</span>
      <span class="pl-s">"cpufreq"</span>: <span class="pl-s">""</span><span class="pl-kos">,</span>
      <span class="pl-s">"load_average"</span>: <span class="pl-kos">[</span>
        <span class="pl-s">"0.01"</span><span class="pl-kos">,</span>
        <span class="pl-s">"0.04"</span><span class="pl-kos">,</span>
        <span class="pl-s">"0.01"</span>
      <span class="pl-kos">]</span><span class="pl-kos">,</span>
      <span class="pl-s">"mbuf"</span>: <span class="pl-s">"1016/61600"</span><span class="pl-kos">,</span>
      <span class="pl-s">"mbufpercent"</span>: <span class="pl-s">"2"</span>
    <span class="pl-kos">}</span>
  <span class="pl-kos">}</span>
<span class="pl-kos">}</span></pre></div>
<hr>
<h3>
<a id="user-content-system_info" class="anchor" href="#system_info" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>system_info</h3>
<ul>
<li>Returns various useful system info.</li>
<li>HTTP: <strong>GET</strong>
</li>
<li>Params: none</li>
</ul>
<p><em>Example Request</em></p>
<div class="highlight highlight-source-shell"><pre>curl \
    -X GET \
    --silent \
    --insecure \
    --header <span class="pl-s"><span class="pl-pds">"</span>restapi-auth: &lt;auth-value&gt;<span class="pl-pds">"</span></span> \
    <span class="pl-s"><span class="pl-pds">"</span>https://&lt;host-address&gt;/restapi/v1/?action=system_info<span class="pl-pds">"</span></span></pre></div>
<p><em>Example Response</em></p>
<div class="highlight highlight-source-js"><pre><span class="pl-kos">{</span>
    <span class="pl-s">"callid"</span>: <span class="pl-s">"5e1d8ceb8ff47"</span><span class="pl-kos">,</span>
    <span class="pl-s">"action"</span>: <span class="pl-s">"system_info"</span><span class="pl-kos">,</span>
    <span class="pl-s">"message"</span>: <span class="pl-s">"ok"</span><span class="pl-kos">,</span>
    <span class="pl-s">"data"</span>: <span class="pl-kos">{</span>
        <span class="pl-s">"info"</span>: <span class="pl-kos">{</span>
            <span class="pl-s">"sys"</span>: <span class="pl-kos">{</span>
                <span class="pl-s">"platform"</span>: <span class="pl-kos">{</span>
                    <span class="pl-s">"name"</span>: <span class="pl-s">"VMware"</span><span class="pl-kos">,</span>
                    <span class="pl-s">"descr"</span>: <span class="pl-s">"VMware Virtual Machine"</span>
                <span class="pl-kos">}</span><span class="pl-kos">,</span>
                <span class="pl-s">"serial_no"</span>: <span class="pl-s">""</span><span class="pl-kos">,</span>
                <span class="pl-s">"device_id"</span>: <span class="pl-s">"719e8c91c2c43b820400"</span>
            <span class="pl-kos">}</span><span class="pl-kos">,</span>
            <span class="pl-s">"pfsense_version"</span>: <span class="pl-kos">{</span>
                <span class="pl-s">"product_version_string"</span>: <span class="pl-s">"2.4.5-DEVELOPMENT"</span><span class="pl-kos">,</span>
                <span class="pl-s">"product_version"</span>: <span class="pl-s">"2.4.5-DEVELOPMENT"</span><span class="pl-kos">,</span>
                <span class="pl-s">"product_version_patch"</span>: <span class="pl-s">"0"</span>
            <span class="pl-kos">}</span><span class="pl-kos">,</span>
            <span class="pl-s">"pfsense_remote_version"</span>: <span class="pl-kos">{</span>
                <span class="pl-s">"version"</span>: <span class="pl-s">"2.4.5.a.20200112.1821"</span><span class="pl-kos">,</span>
                <span class="pl-s">"installed_version"</span>: <span class="pl-s">"2.4.5.a.20191218.2354"</span><span class="pl-kos">,</span>
                <span class="pl-s">"pkg_version_compare"</span>: <span class="pl-s">"&lt;"</span>
            <span class="pl-kos">}</span><span class="pl-kos">,</span>
            <span class="pl-s">"os_verison"</span>: <span class="pl-s">"FreeBSD 11.3-STABLE"</span><span class="pl-kos">,</span>
            <span class="pl-s">"cpu_type"</span>: <span class="pl-kos">{</span>
                <span class="pl-s">"cpu_model"</span>: <span class="pl-s">"Intel(R) Core(TM) i7-7700 CPU @ 3.60GHz"</span><span class="pl-kos">,</span>
                <span class="pl-s">"cpu_count"</span>: <span class="pl-s">"4"</span><span class="pl-kos">,</span>
                <span class="pl-s">"logic_cpu_count"</span>: <span class="pl-s">"4 package(s)"</span><span class="pl-kos">,</span>
                <span class="pl-s">"cpu_freq"</span>: <span class="pl-s">""</span>
            <span class="pl-kos">}</span><span class="pl-kos">,</span>
            <span class="pl-s">"kernel_pti_status"</span>: <span class="pl-s">"enabled"</span><span class="pl-kos">,</span>
            <span class="pl-s">"mds_mitigation"</span>: <span class="pl-s">"inactive"</span><span class="pl-kos">,</span>
            <span class="pl-s">"bios"</span>: <span class="pl-kos">{</span>
                <span class="pl-s">"vendor"</span>: <span class="pl-s">"Phoenix Technologies LTD"</span><span class="pl-kos">,</span>
                <span class="pl-s">"version"</span>: <span class="pl-s">"6.00"</span><span class="pl-kos">,</span>
                <span class="pl-s">"date"</span>: <span class="pl-s">"07/29/2019"</span>
            <span class="pl-kos">}</span>
        <span class="pl-kos">}</span>
    <span class="pl-kos">}</span>
<span class="pl-kos">}</span></pre></div>
<hr>

<h2>
<a id="user-content-php-example" class="anchor" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>PHP example</h2>
<hr>

<ul>
<li>Changing config</li>
<li>Read / Write config</li>
</ul>
<p>With this example you can change an incoming IP address on a firewall rule.</p>
<p>Need to send: <strong>ip</strong>, <strong>desc</strong> where ip= the ip we want to set, desc= description of the rule (as a unique identifier)</p>
<p>Link for set ip: example.php<span class="gf">?</span><strong>set</strong><span class="gf">&</span><strong>ip=</strong>172.16.2.2<span class="gf">&</span><strong>desc=</strong>Rule%20for%20my%20friend</p>
<p>Link for set ip: example.php<span class="gf">?</span><strong>get</strong><span class="gf">&</span><strong>desc=</strong>Rule%20for%20my%20friend</p>
<br />
<br />
<p><em>Example code</em></p>
<div class="highlight highlight-source-shell"><pre class="phpExample"><span class="gf">##################################################################################
## Credentials for RestAPI</span>
<span class="lbf">$apiKey</span>      <span class="grf">=</span> <span class="sf">"&lt;APIKEY&gt;"</span>;
<span class="lbf">$secret</span>      <span class="grf">=</span> <span class="sf">"&lt;SECRET&gt;"</span>;
<span class="lbf">$pfsenseHost</span> <span class="grf">=</span> <span class="sf">"&lt;PFSENSE_HOST_IP&gt;"</span>;
<span class="gf">## END Credentials
##################################################################################</span>

<span class="vf">if</span> <span class="yf">(</span><span class="lyf">isset</span><span class="yf">(</span><span class="lbf">$_GET</span><span class="vf">[</span><span class="sf">"ip"</span><span class="vf">]</span><span class="yf">)</span> <span class="grf">&&</span> <span class="lyf">isset</span><span class="yf">(</span><span class="lbf">$_GET</span><span class="vf">[</span><span class="sf">"desc"</span><span class="vf">]</span><span class="yf">)</span> <span class="grf">&&</span> <span class="lyf">isset</span><span class="yf">(</span><span class="lbf">$_GET</span><span class="vf">[</span><span class="sf">"set"</span><span class="vf">]</span><span class="yf">)</span><span class="yf">)</span> <span class="yf">{</span>
  <span class="lyf">echo set_ip_for_rule</span><span class="yf">(</span><span class="lbf">$_GET</span><span class="vf">[</span><span class="sf">"ip"</span><span class="vf">]</span>, <span class="lbf">$_GET</span><span class="vf">[</span><span class="sf">"desc"</span><span class="vf">]</span><span class="yf">)</span><span class="vf">[</span><span class="sf">"message"</span><span class="vf">]</span>;
<span class="yf">}</span>
<span class="vf">else</span> <span class="vf">if</span> <span class="yf">(</span><span class="lyf">isset</span><span class="yf">(</span><span class="lbf">$_GET</span><span class="vf">[</span><span class="sf">"desc"</span><span class="vf">]</span><span class="yf">)</span> <span class="grf">&&</span> <span class="lyf">isset</span><span class="yf">(</span><span class="lbf">$_GET</span><span class="vf">[</span><span class="sf">"get"</span><span class="vf">]</span><span class="yf">)</span><span class="yf">)</span> <span class="yf">{</span>
  <span class="lyf">echo get_ip_from_rule</span><span class="yf">(</span><span class="lbf">$_GET</span><span class="vf">[</span><span class="sf">"desc"</span><span class="vf">]</span><span class="yf">)</span>;
<span class="yf">}</span>
<span class="vf">else</span> <span class="yf">{</span>
  <span class="vf">die</span><span class="yf">(</span><span class="sf">"nothing changed"</span><span class="yf">)</span>;
<span class="yf">}</span>

<span class="gf">// reading the incoming IP address</span>
<span class="bf bold">function</span> <span class="lyf">get_ip_from_rule</span><span class="yf">(</span><span class="lbf">$ruleDescription</span><span class="yf">)</span> <span class="yf">{</span>
  <span class="lbf">$currentConfig</span> <span class="grf">=</span> <span class="lyf">api_request</span><span class="yf">(</span><span class="sf">"config_get"</span><span class="yf">)</span>;
  <span class="lbf">$filters</span> <span class="grf">=</span> <span class="lbf">$currentConfig</span><span class="vf">[</span><span class="sf">"data"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"config"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"filter"</span><span class="vf">]</span>;
  <span class="lbf">$i</span> <span class="grf">=</span> <span class="lyf">0</span>;
  <span class="vf">foreach</span> <span class="yf">(</span><span class="lbf">$filters</span><span class="vf">[</span><span class="sf">"rule"</span><span class="vf">]</span> <span class="lyf">as</span> <span class="lbf">$filter</span><span class="yf">)</span> <span class="yf">{</span>
    <span class="vf">if</span> <span class="yf">(</span><span class="lyf">strpos</span><span class="yf">(</span><span class="lbf">$filter</span><span class="vf">[</span><span class="sf">"descr"</span><span class="vf">]</span>, <span class="lbf">$ruleDescription</span><span class="yf">)</span> <span class="lyf">!==</span> <span class="bf">false</span><span class="yf">)</span> <span class="yf">{</span>
      <span class="vf">return</span> <span class="lbf">$currentConfig</span><span class="vf">[</span><span class="sf">"data"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"config"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"filter"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"rule"</span><span class="vf">]</span><span class="vf">[</span><span class="lbf">$i</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"source"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"address"</span><span class="vf">]</span>;
    <span class="yf">}</span>
    <span class="lbf">$i</span><span class="lyf">++</span>;
  <span class="yf">}</span>
  <span class="vf">return</span> <span class="sf">""</span>;
<span class="yf">}</span>

<span class="gf">// writing the incoming IP address back to the config</span>
<span class="bf bold">function</span> <span class="lyf">set_ip_for_rule</span><span class="yf">(</span><span class="lbf">$newIp</span>, <span class="lbf">$ruleDescription</span><span class="yf">)</span> <span class="yf">{</span>
  <span class="lbf">$currentConfig</span> <span class="grf">=</span> <span class="lyf">api_request</span><span class="yf">(</span><span class="sf">"config_get"</span><span class="yf">)</span>;

  <span class="gf">// firewall rule</span>
  <span class="lbf">$filters</span> <span class="grf">=</span> <span class="lbf">$currentConfig</span><span class="vf">[</span><span class="sf">"data"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"config"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"filter"</span><span class="vf">]</span>;

  <span class="lbf">$i</span> <span class="grf">=</span> <span class="lyf">0</span>;
  <span class="vf">foreach</span> <span class="yf">(</span><span class="lbf">$filters</span><span class="vf">[</span><span class="sf">"rule"</span><span class="vf">]</span> <span class="lyf">as</span> <span class="lbf">$filter</span><span class="yf">)</span> <span class="yf">{</span>
    <span class="gf">// finding the rule based on description</span>
    <span class="vf">if</span> <span class="yf">(</span><span class="lyf">strpos</span><span class="yf">(</span><span class="lbf">$filter</span><span class="vf">[</span><span class="sf">"descr"</span><span class="vf">]</span>, <span class="lbf">$ruleDescription</span><span class="yf">)</span> <span class="lyf">!==</span> <span class="bf">false</span><span class="yf">)</span> <span class="yf">{</span>
      <span class="lbf">$currentConfig</span><span class="vf">[</span><span class="sf">"data"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"config"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"filter"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"rule"</span><span class="vf">]</span><span class="vf">[</span><span class="lbf">$i</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"source"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"address"</span><span class="vf">]</span> <span class="grf">=</span> <span class="lbf">$newIp</span>;
    <span class="yf">}</span>
    <span class="lbf">$i</span><span class="lyf">++</span>;
  <span class="yf">}</span>
  <span class="gf">// END firewall rule</span>


  <span class="gf">// NAT port forward</span>
  <span class="lbf">$filters_nat</span> <span class="grf">=</span> <span class="lbf">$currentConfig</span><span class="vf">[</span><span class="sf">"data"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"config"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"nat"</span><span class="vf">]</span>;

  <span class="lbf">$i</span> <span class="grf">=</span> <span class="lyf">0</span>;
  <span class="vf">foreach</span> <span class="yf">(</span><span class="lbf">$filters_nat</span><span class="vf">[</span><span class="sf">"rule"</span><span class="vf">]</span> <span class="lyf">as</span> <span class="lbf">$filter</span><span class="yf">)</span> <span class="yf">{</span>
    <span class="gf">// finding the rule based on description</span>
    <span class="vf">if</span> <span class="yf">(</span><span class="lyf">strpos</span><span class="yf">(</span><span class="lbf">$filter</span><span class="vf">[</span><span class="sf">"descr"</span><span class="vf">]</span>, <span class="lbf">$ruleDescription</span><span class="yf">)</span> <span class="lyf">!==</span> <span class="bf">false</span><span class="yf">)</span> <span class="yf">{</span>
      <span class="lbf">$currentConfig</span><span class="vf">[</span><span class="sf">"data"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"config"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"nat"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"rule"</span><span class="vf">]</span><span class="vf">[</span><span class="lbf">$i</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"source"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"address"</span><span class="vf">]</span> <span class="grf">=</span> <span class="lbf">$newIp</span>;
    <span class="yf">}</span>
    <span class="lbf">$i</span><span class="lyf">++</span>;
  <span class="yf">}</span>
  <span class="gf">// END NAT port forward</span>

  <span class="gf">// Apply changed configuration back to firewall</span>
  <span class="lbf">$result</span> <span class="grf">=</span> <span class="lyf">api_request</span><span class="yf">(</span><span class="sf">"config_set"</span>, <span class="sf">""</span>, <span class="lbf">$currentConfig</span><span class="vf">[</span><span class="sf">"data"</span><span class="vf">]</span><span class="vf">[</span><span class="sf">"config"</span><span class="vf">]</span><span class="yf">)</span>;
  <span class="lbf">$result</span> <span class="grf">=</span> <span class="lyf">api_request</span><span class="yf">(</span><span class="sf">"send_event"</span>, "<span class="vf">[</span><span class="sf">\"filter reload\"</span></span><span class="vf">]</span>", <span class="sf">""</span>, <span class="bf">True</span><span class="yf">)</span>;

  <span class="vf">return</span> <span class="lbf">$result</span>;
<span class="yf">}</span>

<span class="bf bold">function</span> <span class="lyf">api_request</span><span class="yf">(</span><span class="lbf">$action</span>, <span class="lbf">$params</span> <span class="grf">=</span> <span class="sf">""</span>, <span class="lbf">$data</span> <span class="grf">=</span> <span class="sf">""</span>, <span class="lbf">$filter_reload</span> <span class="grf">=</span> <span class="bf">False</span><span class="yf">)</span> <span class="yf">{</span>
  <span class="bf">global</span> <span class="lbf">$apiKey</span>, <span class="lbf">$secret</span>, <span class="lbf">$pfsenseHost</span>;
  <span class="lbf">$path</span> <span class="grf">=</span> <span class="sf">"/?action="</span> . <span class="lbf">$action</span>;
  <span class="lbf">$url</span> <span class="grf">=</span> <span class="sf">"https://"</span> . <span class="lbf">$pfsenseHost</span> . <span class="sf">"/restapi/v1"</span> . <span class="lbf">$path</span>;

  <span class="lbf">$ch</span> <span class="grf">=</span> <span class="lyf">curl_init</span><span class="yf">(</span><span class="lbf">$url</span><span class="yf">)</span>;
  <span class="lyf">curl_setopt</span><span class="yf">(</span><span class="lbf">$ch</span>, CURLOPT_SSL_VERIFYHOST, <span class="lyf">0</span><span class="yf">)</span>;
  <span class="lyf">curl_setopt</span><span class="yf">(</span><span class="lbf">$ch</span>, CURLOPT_SSL_VERIFYPEER, <span class="lyf">0</span><span class="yf">)</span>;
  <span class="vf">if</span> <span class="yf">(</span><span class="lbf">$filter_reload</span> === <span class="bf">True</span><span class="yf">)</span> <span class="yf">{</span>
    <span class="lyf">curl_setopt</span><span class="yf">(</span><span class="lbf">$ch</span>, CURLOPT_POSTFIELDS, <span class="lbf">$params</span><span class="yf">)</span>;
  <span class="yf">}</span> <span class="vf">else</span> <span class="yf">{</span>
    <span class="lyf">curl_setopt</span><span class="yf">(</span><span class="lbf">$ch</span>, CURLOPT_POSTFIELDS, <span class="lyf">json_encode</span><span class="yf">(</span><span class="lbf">$data</span><span class="yf">)</span><span class="yf">)</span>;
  <span class="yf">}</span>
  <span class="lyf">curl_setopt</span><span class="yf">(</span><span class="lbf">$ch</span>, CURLOPT_HTTPHEADER, <span class="vf">[</span><span class="sf">"restapi-auth: . "</span> . <span class="lyf">auth_gen</span><span class="yf">(</span><span class="lbf">$apiKey</span>, <span class="lbf">$secret</span><span class="yf">)</span><span class="vf">]</span><span class="yf">)</span>;
  <span class="lyf">curl_setopt</span><span class="yf">(</span><span class="lbf">$ch</span>, CURLOPT_RETURNTRANSFER, <span class="bf">true</span><span class="yf">)</span>;

  <span class="lbf">$json</span> <span class="grf">=</span> <span class="lyf">curl_exec</span><span class="yf">(</span><span class="lbf">$ch</span><span class="yf">)</span>;
  <span class="lyf">curl_close</span><span class="yf">(</span><span class="lbf">$ch</span><span class="yf">)</span>;
  <span class="lbf">$result</span> <span class="grf">=</span> <span class="lyf">json_decode</span><span class="yf">(</span><span class="lbf">$json</span> , <span class="bf">true</span><span class="yf">)</span>;
  <span class="vf">return</span> <span class="lbf">$result</span>;
<span class="yf">}</span>


<span class="gf">// Generating auth string</span>
<span class="bf bold">function</span> <span class="lyf">auth_gen</span><span class="yf">(</span><span class="lbf">$apiKey</span>, <span class="lbf">$secret</span><span class="yf">)</span> <span class="yf">{</span>
  <span class="lbf">$nonce</span> <span class="grf">=</span> <span class="lyf">substr</span><span class="yf">(</span><span class="lyf">hash</span><span class="yf">(</span><span class="sf">"sha512"</span>, <span class="lyf">makeRandomString</span><span class="yf">(</span><span class="yf">)</span><span class="yf">)</span>, <span class="lyf">0</span>, <span class="lyf">8</span><span class="yf">)</span>;
  <span class="lbf">$timestamp</span> <span class="grf">=</span> <span class="lyf">gmdate</span><span class="yf">(</span><span class="sf">"Ymd"</span><span class="yf">)</span> . <span class="sf">"Z"</span> . <span class="lyf">gmdate</span><span class="yf">(</span><span class="sf">"His"</span><span class="yf">)</span>;
  <span class="lbf">$hash</span> <span class="grf">=</span> <span class="lyf">hash</span><span class="yf">(</span><span class="sf">"sha256"</span>, <span class="lbf">$secret</span> . <span class="lbf">$timestamp</span> . <span class="lbf">$nonce</span><span class="yf">)</span>;

  <span class="vf">return</span> <span class="lbf">$apiKey</span> . <span class="sf">":"</span> . <span class="lbf">$timestamp</span> . <span class="sf">":"</span> . <span class="lbf">$nonce</span> . <span class="sf">":"</span> . <span class="lbf">$hash</span>;
<span class="yf">}</span>

<span class="gf">// Generating random string for creating a nonce</span>
<span class="bf bold">function</span> <span class="lyf">makeRandomString</span><span class="yf">(</span><span class="lbf">$bits</span> <span class="grf">=</span> <span class="lyf">256</span><span class="yf">)</span> <span class="yf">{</span>
  <span class="lbf">$bytes</span> <span class="grf">=</span> <span class="lyf">ceil</span><span class="yf">(</span><span class="lbf">$bits</span> / <span class="lyf">8</span><span class="yf">)</span>;
  <span class="lbf">$result</span> <span class="grf">=</span> <span class="sf">""</span>;
  <span class="vf">for</span> <span class="yf">(</span><span class="lbf">$i</span> <span class="grf">=</span> <span class="lyf">0</span>; <span class="lbf">$i</span> <span class="lyf"><</span> <span class="lbf">$bytes</span>; <span class="lbf">$i</span><span class="lyf">++</span><span class="yf">)</span> <span class="yf">{</span> 
    <span class="lbf">$result</span> .<span class="grf">=</span> <span class="lyf">chr</span><span class="yf">(</span><span class="lyf">mt_rand</span><span class="yf">(</span><span class="lyf">0</span>, <span class="lyf">255</span><span class="yf">)</span><span class="yf">)</span>; 
  <span class="yf">}</span> 
  <span class="vf">return</span> <span class="lbf">$result</span>; 
<span class="yf">}</span>
</pre></div>


<hr>




<h2>
<a id="user-content-versions-and-testing" class="anchor" href="#versions-and-testing" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>Versions and Testing</h2>
<p>The RestAPI has been developed against the following pfSense versions</p>
<ul>
<li>
<strong>2.3.x</strong> - 2.3.2, 2.3.3, 2.3.4, 2.3.5</li>
<li>
<strong>2.4.x</strong> - 2.4.3, 2.4.4, 2.4.5</li>
<li>
<strong>2.5.x</strong> - 2.5.0, 2.5.1</li>
<li>
<strong>2.6.x</strong> - 2.6.0</li>
</ul>
<p>Testing is reasonable but does not achieve 100% code coverage within the RestAPI
codebase. Under the
hood RestAPI, performs real-time sanity checks and tests to make sure the user
supplied configurations will save, load and reload as expected.</p>


<h2>
<a id="user-content-releases" class="anchor" href="#releases" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>Releases</h2>
<h4>
<a id="user-content-v10---2023-06-21" class="anchor" href="#v10---2023-06-21" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>v1.0 - 2023-06-21</h4>
<ul>
<li>initial release</li>
</ul>


<!--READMEEND-->
</div>