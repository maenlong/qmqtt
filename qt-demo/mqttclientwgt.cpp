#include "mqttclientwgt.h"
#include "mqttclientmgr.h"
#include "mqttproxymanager.h"
#include "mqtttopicbuilder.h"
#include "ui_mqttclientwgt.h"
#include <QApplication>
#include <QTranslator>
#include <QUuid>
#include <QSslError>
#include <QFileDialog>
#include <QIntValidator>
#include <QSignalBlocker>
#include <QStyle>
#include <QTime>

MqttClientWgt::MqttClientWgt(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::MqttClientWgt)
{
    m_mqttMgr = new MqttClientMgr(this);
    m_ui->setupUi(this);

    QIntValidator* portValidator = new QIntValidator(1, 65535, this);
    m_ui->portLet->setValidator(portValidator);
    m_ui->proxyPortLet->setValidator(portValidator);
    m_ui->keepAliveLet->setValidator(new QIntValidator(0, 65535, this));

    m_ui->hostLet->setText("broker.emqx.io");
    m_ui->portLet->setText("1883");
    QString uuidFragment = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    m_ui->clientIdLet->setText(QString("qt_demo_%1").arg(uuidFragment));
    m_ui->keepAliveLet->setText("30");
    m_ui->typeCbx->addItem(QString("TCP"), static_cast<int>(MqttConnectionTcp));
    m_ui->typeCbx->addItem(QString("WS"), static_cast<int>(MqttConnectionWs));
    m_ui->typeCbx->addItem(QString("WSS"), static_cast<int>(MqttConnectionWss));
    m_ui->willQosCbx->addItems({"0", "1", "2"});
    m_ui->subQosCbx->addItems({"0", "1", "2"});
    m_ui->subQosCbx->setCurrentIndex(1);
    m_ui->pubQosCbx->addItems({"0", "1", "2"});
    m_ui->pubQosCbx->setCurrentIndex(1);
    m_ui->proxyTypeCbx->addItems({"None", "HTTP", "SOCKS5"});

    connect(m_mqttMgr, &MqttClientMgr::sig_connected,
            this, &MqttClientWgt::slot_onConnected);

    connect(m_mqttMgr, &MqttClientMgr::sig_disconnected,
            this, &MqttClientWgt::slot_onDisconnected);

    connect(m_mqttMgr, &MqttClientMgr::sig_error,
            this, &MqttClientWgt::slot_onError);

    connect(m_mqttMgr, &MqttClientMgr::sig_connectionParamsInvalid,
            this, &MqttClientWgt::slot_onConnectionParamsInvalid);

    connect(m_mqttMgr, &MqttClientMgr::sig_caCertificateLoadFailed,
            this, &MqttClientWgt::slot_onCaCertificateLoadFailed);

    connect(m_mqttMgr, &MqttClientMgr::sig_sslErrors,
            this, &MqttClientWgt::slot_onSslErrors);

    connect(m_mqttMgr, &MqttClientMgr::sig_messageReceived,
            this, &MqttClientWgt::slot_onMessageReceived);

    connect(m_mqttMgr, &MqttClientMgr::sig_subscribed,
            this, &MqttClientWgt::slot_onSubscribed);

    connect(m_mqttMgr, &MqttClientMgr::sig_unsubscribed,
            this, &MqttClientWgt::slot_onUnsubscribed);

    connect(m_mqttMgr, &MqttClientMgr::sig_published,
            this, &MqttClientWgt::slot_onPublished);

    connect(m_mqttMgr, &MqttClientMgr::sig_pingResp,
            this, &MqttClientWgt::slot_onPingResp);

    connect(m_mqttMgr, &MqttClientMgr::sig_reconnectScheduled,
            this, &MqttClientWgt::slot_onReconnectScheduled);

    connect(m_mqttMgr, &MqttClientMgr::sig_reconnectStopped,
            this, &MqttClientWgt::slot_onReconnectStopped);

    m_ui->mainLayout->setStretch(m_ui->mainLayout->indexOf(m_ui->logGrp), 1);
    m_ui->langCbx->addItems({"EN", "中文"});

    applyTranslations();
}

MqttClientWgt::~MqttClientWgt()
{
    delete m_ui;
}

void MqttClientWgt::on_connectBtn_clicked()
{
    MqttConnectionParams params;
    params.host = m_ui->hostLet->text();
    params.port = m_ui->portLet->text().toUShort();
    params.clientId = m_ui->clientIdLet->text();
    params.username = m_ui->usernameLet->text();
    params.password = m_ui->passwordLet->text();
    bool keepAliveOk = false;
    params.keepAlive = m_ui->keepAliveLet->text().toInt(&keepAliveOk);
    if (!keepAliveOk)
    {
        params.keepAlive = -1;
    }
    params.type = static_cast<MqttConnectionType>(m_ui->typeCbx->currentData().toInt());
    params.cleanSession = m_ui->cleanSessionCbk->isChecked();
    params.websocketPath = m_ui->websocketPathLet->text();
    params.websocketOrigin = m_ui->websocketOriginLet->text();
    params.sslCaCertPath = m_ui->sslCaCertLet->text();
    params.ignoreSelfSigned = m_ui->ignoreSelfSignedCbk->isChecked();
    params.willTopic = m_ui->willTopicLet->text();
    params.willMessage = m_ui->willMessageLet->text();
    params.willQos = m_ui->willQosCbx->currentIndex();
    params.willRetain = m_ui->willRetainCbk->isChecked();

    if (m_mqttMgr->connectToHost(params))
    {
        appendMessage(tr("[Connecting] ..."), false); // [正在连接] ...
    }
}

void MqttClientWgt::on_disconnectBtn_clicked()
{
    m_mqttMgr->disconnectFromHost();
}

void MqttClientWgt::on_subscribeBtn_clicked()
{
    if (!m_mqttMgr->isConnected())
    {
        appendMessage(tr("[Error] Not connected"), false); // [错误] 未连接
        return;
    }
    QString topic = MqttTopicBuilder::inboxTopic(m_ui->selfImAccidLet->text());
    if (topic.isEmpty())
    {
        appendMessage(tr("[Error] Self imAccid is empty"), false); // [错误] 自己的 imAccid 为空
        return;
    }

    m_mqttMgr->subscribe(topic, static_cast<quint8>(m_ui->subQosCbx->currentIndex()));
    appendMessage(tr("[Subscribe] %1 (QoS %2)").arg(topic).arg(m_ui->subQosCbx->currentText()), false); // [订阅] %1(QoS %2)
}

void MqttClientWgt::on_unsubscribeBtn_clicked()
{
    if (!m_mqttMgr->isConnected())
    {
        appendMessage(tr("[Error] Not connected"), false); // [错误] 未连接
        return;
    }
    QString topic = MqttTopicBuilder::inboxTopic(m_ui->selfImAccidLet->text());
    if (topic.isEmpty())
    {
        appendMessage(tr("[Error] Self imAccid is empty"), false); // [错误] 自己的 imAccid 为空
        return;
    }

    m_mqttMgr->unsubscribe(topic);
    appendMessage(tr("[Unsubscribe] %1").arg(topic), false); // [取消订阅] %1
}

void MqttClientWgt::on_publishBtn_clicked()
{
    if (!m_mqttMgr->isConnected())
    {
        appendMessage(tr("[Error] Not connected"), false); // [错误] 未连接
        return;
    }

    QString topic = MqttTopicBuilder::inboxTopic(m_ui->targetImAccidLet->text());
    if (topic.isEmpty())
    {
        appendMessage(tr("[Error] Target imAccid is empty"), false); // [错误] 目标 imAccid 为空
        return;
    }
    QString payload = m_ui->payloadTed->toPlainText();
    if (payload.isEmpty())
    {
        appendMessage(tr("[Error] Payload is empty"), false); // [错误] 消息内容为空
        return;
    }

    m_mqttMgr->publish(topic, payload.toUtf8(), static_cast<quint8>(m_ui->pubQosCbx->currentIndex()));
    appendMessage(tr("[Sent] %1: %2 (QoS %3)").arg(topic).arg(payload).arg(m_ui->pubQosCbx->currentText()), true); // [已发送] %1: %2(QoS %3)
}

void MqttClientWgt::on_clearLogBtn_clicked()
{
    m_ui->logTed->clear();
}

void MqttClientWgt::appendMessage(const QString& msg, bool isSent)
{
    m_ui->logTed->appendPlainText((isSent ? ">> " : "<< ") + msg);
}

void MqttClientWgt::updateConnectionState(bool connected)
{
    m_ui->connectBtn->setEnabled(!connected);
    m_ui->disconnectBtn->setEnabled(connected);
    m_ui->statusTextLbl->setText(connected ? tr("Connected") : tr("Disconnected")); // 已连接 / 已断开
    m_ui->statusTextLbl->setProperty("mqttConnectionState",
                                     connected ? QString("connected") : QString("disconnected"));
    m_ui->statusTextLbl->style()->unpolish(m_ui->statusTextLbl);
    m_ui->statusTextLbl->style()->polish(m_ui->statusTextLbl);
}

void MqttClientWgt::on_typeCbx_currentIndexChanged(int index)
{
    MqttConnectionType type = static_cast<MqttConnectionType>(m_ui->typeCbx->itemData(index).toInt());
    bool webSocketEnabled = type == MqttConnectionWs || type == MqttConnectionWss;
    m_ui->websocketPathTipLbl->setEnabled(webSocketEnabled);
    m_ui->websocketPathLet->setEnabled(webSocketEnabled);
    m_ui->websocketOriginTipLbl->setEnabled(webSocketEnabled);
    m_ui->websocketOriginLet->setEnabled(webSocketEnabled);
    m_ui->sslGrp->setEnabled(type == MqttConnectionWss);
    if (type == MqttConnectionWss)
    {
        m_ui->portLet->setText(QString("8084"));
    }
    else if (type == MqttConnectionWs)
    {
        m_ui->portLet->setText(QString("8083"));
    }
    else
    {
        m_ui->portLet->setText(QString("1883"));
    }
}

void MqttClientWgt::slot_onConnected()
{
    m_pingCount = 0;
    appendMessage(tr("[Connected] %1:%2").arg(m_ui->hostLet->text()).arg(m_ui->portLet->text()), false); // [已连接] %1:%2
    updateConnectionState(true);
    QString topic = MqttTopicBuilder::inboxTopic(m_ui->selfImAccidLet->text());
    if (!topic.isEmpty())
    {
        m_mqttMgr->subscribe(topic, static_cast<quint8>(m_ui->subQosCbx->currentIndex()));
        appendMessage(tr("[Subscribe] %1 (QoS %2)").arg(topic)
                                                       .arg(m_ui->subQosCbx->currentText()), false); // [订阅] %1(QoS %2)
    }
}

void MqttClientWgt::slot_onDisconnected()
{
    appendMessage(tr("[Disconnected]"), false); // [已断开]
    updateConnectionState(false);
}

void MqttClientWgt::slot_onError(int errorCode)
{
    appendMessage(tr("[Error] code: %1").arg(errorCode), false); // [错误] 代码: %1
}

void MqttClientWgt::slot_onConnectionParamsInvalid()
{
    appendMessage(tr("[Error] Invalid connection parameters"), false); // [错误] 连接参数无效
}

void MqttClientWgt::slot_onCaCertificateLoadFailed(const QString& path)
{
    appendMessage(tr("[Error] Failed to load CA certificate: %1").arg(path), false); // [错误] CA 证书加载失败: %1
}

void MqttClientWgt::slot_onSslErrors(const QList<QSslError>& errors)
{
    for (const QSslError& err : errors)
    {
        appendMessage(tr("[SSL Error] %1").arg(err.errorString()), false); // [SSL 错误] %1
    }
}

void MqttClientWgt::slot_onMessageReceived(const QString& topic, const QByteArray& payload)
{
    appendMessage(tr("[Received] %1: %2").arg(topic).arg(QString::fromUtf8(payload)), false); // [收到] %1: %2
}

void MqttClientWgt::slot_onSubscribed(const QString& topic, quint8 qos)
{
    if (qos <= 2)
    {
        appendMessage(tr("[Subscribed] %1 (QoS %2)").arg(topic).arg(qos), false); // [已订阅] %1(QoS %2)
    }
    else
    {
        appendMessage(tr("[Subscribe Failed] %1").arg(topic), false); // [订阅失败] %1
    }
}

void MqttClientWgt::slot_onUnsubscribed(const QString& topic)
{
    appendMessage(tr("[Unsubscribed] %1").arg(topic), false); // [已取消订阅] %1
}

void MqttClientWgt::slot_onPublished(const QString& topic, quint8 qos, quint16 messageId)
{
    if (qos > 0)
    {
        appendMessage(tr("[Published] %1 (QoS %2, Message ID %3)").arg(topic).arg(qos).arg(messageId), true); // [发布确认] %1(QoS %2, 消息 ID %3)
    }
}

void MqttClientWgt::on_applyProxyBtn_clicked()
{
    MqttProxyParams params;
    params.type = static_cast<MqttProxyType>(m_ui->proxyTypeCbx->currentIndex());
    params.host = m_ui->proxyHostLet->text();
    params.port = m_ui->proxyPortLet->text().toUShort();
    params.username = m_ui->proxyUsernameLet->text();
    params.password = m_ui->proxyPasswordLet->text();

    if (!MqttProxyManager::applyProxy(params))
    {
        appendMessage(tr("[Proxy] Invalid parameters"), false); // [代理] 参数无效
    }
    else if (params.type == MqttProxyNone)
    {
        appendMessage(tr("[Proxy] Disabled"), false); // [代理] 已禁用
    }
    else
    {
        appendMessage(tr("[Proxy] %1 %2:%3").arg(m_ui->proxyTypeCbx->currentText())
                                                .arg(params.host.trimmed()).arg(params.port), false); // [代理] %1 %2:%3
    }
}

void MqttClientWgt::slot_onPingResp()
{
    ++m_pingCount;
    appendMessage(QString("[%1] %2 #%3").arg(QTime::currentTime().toString("HH:mm:ss")).arg(tr("[Ping] OK")).arg(m_pingCount), false); // [心跳] OK
}

void MqttClientWgt::slot_onReconnectScheduled(int delaySeconds)
{
    appendMessage(tr("[Reconnect] Retrying in %1 seconds").arg(delaySeconds), false); // [重连] 将在 %1 秒后重试
}

void MqttClientWgt::slot_onReconnectStopped(int errorCode)
{
    appendMessage(tr("[Reconnect] Stopped, error code: %1").arg(errorCode), false); // [重连] 已停止，错误代码: %1
}

void MqttClientWgt::on_browseCaCertBtn_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select CA Certificate"), // 选择 CA 证书
                                                 QString(),
                                                 tr("Certificates (*.pem *.crt *.cer *.der);;All Files (*)")); // 证书文件 (*.pem *.crt *.cer *.der);;所有文件 (*)
    if (!path.isEmpty())
    {
        m_ui->sslCaCertLet->setText(path);
    }
}

void MqttClientWgt::on_langCbx_currentIndexChanged(int index)
{
    qApp->removeTranslator(m_translator);
    if (index == 1)
    {
        if (!m_translator)
        {
            m_translator = new QTranslator(this);
        }
        QString translationPath = QString(":/i18n/mqtt-client_zh_CN.qm");
        if (m_translator->load(translationPath))
        {
            qApp->installTranslator(m_translator);
        }
        else
        {
            QSignalBlocker blocker(m_ui->langCbx);
            m_ui->langCbx->setCurrentIndex(0);
            appendMessage(tr("[Error] Failed to load translation: %1").arg(translationPath), false); // [错误] 翻译资源加载失败: %1
        }
    }
}

void MqttClientWgt::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        applyTranslations();
    }
    QWidget::changeEvent(event);
}

void MqttClientWgt::applyTranslations()
{
    m_ui->retranslateUi(this);
    m_ui->hostLet->setToolTip(tr("MQTT broker address, e.g. broker.emqx.io")); // MQTT 地址，如 broker.emqx.io
    m_ui->portLet->setToolTip(tr("1883 = TCP, 8083 = WS, 8084 = WSS")); // 1883 = TCP, 8083 = WS, 8084 = WSS
    m_ui->typeCbx->setToolTip(tr("TCP: raw MQTT\nWS: WebSocket (port 8083)\nWSS: WebSocket + TLS (port 8084)")); // TCP：原始 MQTT\nWS：WebSocket（端口 8083）\nWSS：WebSocket + TLS（端口 8084）
    m_ui->clientIdLet->setToolTip(tr("Unique client identifier.\nBroker uses it to distinguish clients.")); // 唯一客户端标识，代理用于区分客户端。
    m_ui->usernameLet->setToolTip(tr("Optional: broker authentication")); // 可选：代理认证用户名
    m_ui->passwordLet->setToolTip(tr("Optional: broker authentication password")); // 可选：代理认证密码
    m_ui->keepAliveLet->setToolTip(tr("Heartbeat interval (seconds).\nIf broker receives no packet within 1.5x this interval, client is considered disconnected.")); // 心跳间隔（秒）。超过 1.5 倍未收到数据包则视为断开。
    m_ui->cleanSessionCbk->setToolTip(tr("ON: start a fresh session, discard old subscriptions & offline messages.\nOFF: broker preserves subscriptions & offline messages across reconnects.")); // ON：全新会话，丢弃旧订阅和离线消息。\nOFF：代理保留订阅和离线消息。
    m_ui->websocketPathLet->setToolTip(tr("WebSocket request path, e.g. /mqtt")); // WebSocket 请求路径，例如 /mqtt
    m_ui->websocketOriginLet->setToolTip(tr("Optional WebSocket Origin header")); // 可选的 WebSocket Origin 请求头
    m_ui->willTopicLet->setToolTip(tr("Last Will topic.\nBroker publishes this message when client disconnects unexpectedly.")); // 遗嘱主题，客户端异常断开时代理发布此消息。
    m_ui->willMessageLet->setToolTip(tr("Last Will payload (sent when client goes offline unexpectedly)")); // 遗嘱消息内容（客户端异常离线时发送）
    m_ui->willQosCbx->setToolTip(tr("0: at most once\n1: at least once (default)\n2: exactly once")); // 0：最多一次\n1：至少一次（默认）\n2：恰好一次
    m_ui->willRetainCbk->setToolTip(tr("If checked, broker keeps this message for late subscribers.")); // 选中后代理保留此消息给后续订阅者。
    m_ui->selfImAccidLet->setToolTip(tr("Your IM account ID.\nAuto-subscribes to user/{imAccid}/inbox on connect.")); // 您的 IM 账号。连接后自动订阅 user/{imAccid}/inbox。
    m_ui->subQosCbx->setToolTip(tr("0: at most once (fastest)\n1: at least once (may duplicate)\n2: exactly once (slowest)")); // 0：最多一次（最快）\n1：至少一次（可能重复）\n2：恰好一次（最慢）
    m_ui->targetImAccidLet->setToolTip(tr("Recipient IM account ID.\nMessage is published to user/{imAccid}/inbox.")); // 接收方 IM 账号。消息发布到 user/{imAccid}/inbox。
    m_ui->pubQosCbx->setToolTip(tr("0: at most once (fastest)\n1: at least once (may duplicate)\n2: exactly once (slowest)")); // 0：最多一次（最快）\n1：至少一次（可能重复）\n2：恰好一次（最慢）
    m_ui->payloadTed->setToolTip(tr("JSON message payload")); // JSON 消息载荷

    m_ui->proxyTypeCbx->setToolTip(tr("None: no proxy\nHTTP: HTTP CONNECT\nSOCKS5: SOCKS5 proxy")); // 无代理 / HTTP CONNECT / SOCKS5
    m_ui->proxyHostLet->setToolTip(tr("Proxy server hostname or IP address")); // 代理服务器主机名或 IP
    m_ui->proxyPortLet->setToolTip(tr("Proxy server port")); // 代理服务器端口
    m_ui->proxyUsernameLet->setToolTip(tr("Optional proxy authentication username")); // 可选：代理认证用户名
    m_ui->proxyPasswordLet->setToolTip(tr("Optional proxy authentication password")); // 可选：代理认证密码

    m_ui->sslCaCertLet->setToolTip(tr("Path to CA certificate file (.pem / .crt / .cer / .der).\nCustom certificates are added to the system CA bundle.")); // CA 证书文件路径，自定义证书将追加到系统 CA
    m_ui->ignoreSelfSignedCbk->setToolTip(tr("If checked, self-signed certificates are accepted.\nUse for testing with custom CA or self-signed servers.")); // 选中后接受自签名证书，用于测试环境

    bool connected = m_mqttMgr->isConnected();
    updateConnectionState(connected);
}
