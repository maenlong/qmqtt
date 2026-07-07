<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>MainWgt</name>
    <message>
        <location filename="mainwgt.ui" line="20"/>
        <location filename="mainwgt.cpp" line="13"/>
        <source>MQTT Client Demo</source>
        <translation>MQTT 客户端演示</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="32"/>
        <source>Connection</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="44"/>
        <source>Host:</source>
        <translation>主机：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="54"/>
        <source>Port:</source>
        <translation>端口：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="64"/>
        <source>Type:</source>
        <translation>类型：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="74"/>
        <source>Client ID:</source>
        <translation>客户端 ID：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="84"/>
        <source>Username:</source>
        <translation>用户名：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="94"/>
        <source>Password:</source>
        <translation>密码：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="108"/>
        <source>KA(s):</source>
        <translation>心跳(秒)：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="115"/>
        <source>30</source>
        <translation>30</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="131"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="138"/>
        <source>Disconnect</source>
        <translation>断开</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="145"/>
        <source>Status:</source>
        <translation>状态：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="152"/>
        <location filename="mainwgt.cpp" line="58"/>
        <location filename="mainwgt.cpp" line="195"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="159"/>
        <source>Clean Session</source>
        <translation>清理会话</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="188"/>
        <source>Last Will</source>
        <translation>遗嘱消息</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="200"/>
        <source>Will Topic:</source>
        <translation>遗嘱主题：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="210"/>
        <source>Will Message:</source>
        <translation>遗嘱消息：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="220"/>
        <location filename="mainwgt.ui" line="290"/>
        <location filename="mainwgt.ui" line="344"/>
        <source>QoS:</source>
        <translation>QoS：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="230"/>
        <source>Retain</source>
        <translation>保留</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="253"/>
        <location filename="mainwgt.ui" line="303"/>
        <source>Subscribe</source>
        <translation>订阅</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="280"/>
        <source>Self imAccid:</source>
        <translation>自己的 imAccid：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="313"/>
        <location filename="mainwgt.ui" line="364"/>
        <source>Publish</source>
        <translation>发布</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="334"/>
        <source>Target imAccid:</source>
        <translation>目标 imAccid：</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="357"/>
        <source>{&quot;v&quot;:1,&quot;senderId&quot;:&quot;...&quot;,&quot;commandType&quot;:&quot;CS&quot;,...}</source>
        <translation>{&quot;v&quot;:1,&quot;senderId&quot;:&quot;...&quot;,&quot;commandType&quot;:&quot;CS&quot;,...}</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="374"/>
        <source>Log</source>
        <translation>日志</translation>
    </message>
    <message>
        <location filename="mainwgt.ui" line="417"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="25"/>
        <source>MQTT broker address, e.g. broker.emqx.io</source>
        <translation>MQTT 地址，如 broker.emqx.io</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="26"/>
        <source>1883 = TCP, 8083 = WS, 8084 = WSS</source>
        <translation>1883 = TCP, 8083 = WS, 8084 = WSS</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="27"/>
        <source>TCP: raw MQTT
WS: WebSocket (port 8083)
WSS: WebSocket + TLS (port 8084)</source>
        <translation>TCP：原始 MQTT
WS：WebSocket（端口 8083）
WSS：WebSocket + TLS（端口 8084）</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="28"/>
        <source>Unique client identifier.
Broker uses it to distinguish clients.</source>
        <translation>唯一客户端标识，代理用于区分客户端。</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="29"/>
        <source>Optional: broker authentication</source>
        <translation>可选：代理认证用户名</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="30"/>
        <source>Optional: broker authentication password</source>
        <translation>可选：代理认证密码</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="31"/>
        <source>Heartbeat interval (seconds).
If broker receives no packet within 1.5x this interval, client is considered disconnected.</source>
        <translation>心跳间隔（秒）。超过 1.5 倍未收到数据包则视为断开。</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="32"/>
        <source>ON: start a fresh session, discard old subscriptions &amp; offline messages.
OFF: broker preserves subscriptions &amp; offline messages across reconnects.</source>
        <translation>ON：全新会话，丢弃旧订阅和离线消息。
OFF：代理保留订阅和离线消息。</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="33"/>
        <source>Last Will topic.
Broker publishes this message when client disconnects unexpectedly.</source>
        <translation>遗嘱主题，客户端异常断开时代理发布此消息。</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="34"/>
        <source>Last Will payload (sent when client goes offline unexpectedly)</source>
        <translation>遗嘱消息内容（客户端异常离线时发送）</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="35"/>
        <source>0: at most once
1: at least once (default)
2: exactly once</source>
        <translation>0：最多一次
1：至少一次（默认）
2：恰好一次</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="36"/>
        <source>If checked, broker keeps this message for late subscribers.</source>
        <translation>选中后代理保留此消息给后续订阅者。</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="37"/>
        <source>Your IM account ID.
Auto-subscribes to user/{imAccid}/inbox on connect.</source>
        <translation>您的 IM 账号。连接后自动订阅 user/{imAccid}/inbox。</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="38"/>
        <location filename="mainwgt.cpp" line="40"/>
        <source>0: at most once (fastest)
1: at least once (may duplicate)
2: exactly once (slowest)</source>
        <translation>0：最多一次（最快）
1：至少一次（可能重复）
2：恰好一次（最慢）</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="39"/>
        <source>Recipient IM account ID.
Message is published to user/{imAccid}/inbox.</source>
        <translation>接收方 IM 账号。消息发布到 user/{imAccid}/inbox。</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="41"/>
        <source>JSON message payload</source>
        <translation>JSON 消息载荷</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="110"/>
        <source>[Connected] %1:%2</source>
        <translation>[已连接] %1:%2</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="115"/>
        <source>[Subscribed] %1</source>
        <translation>[已订阅] %1</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="120"/>
        <source>[Disconnected]</source>
        <translation>[已断开]</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="125"/>
        <source>[Error] code: %1</source>
        <translation>[错误] 代码: %1</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="130"/>
        <source>[SSL Error] %1</source>
        <translation>[SSL 错误] %1</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="135"/>
        <source>[Received] %1: %2</source>
        <translation>[收到] %1: %2</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="139"/>
        <source>[Connecting] ...</source>
        <translation>[正在连接] ...</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="153"/>
        <location filename="mainwgt.cpp" line="167"/>
        <source>[Error] Not connected</source>
        <translation>[错误] 未连接</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="161"/>
        <source>[Subscribe] %1 (QoS %2)</source>
        <translation>[订阅] %1（QoS %2）</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="178"/>
        <source>[Sent] %1: %2 (QoS %3)</source>
        <translation>[已发送] %1: %2（QoS %3）</translation>
    </message>
    <message>
        <location filename="mainwgt.cpp" line="195"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
</context>
</TS>
