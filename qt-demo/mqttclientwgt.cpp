#include "mqttclientwgt.h"
#include "qmqtt.h"
#include "ui_mqttclientwgt.h"
#include <QApplication>
#include <QTranslator>
#include <QUuid>
#include <QSslError>
#include <QtWebSockets/QWebSocketProtocol>

MqttClientWgt::MqttClientWgt(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::MqttClientWgt)
{
    m_ui->setupUi(this);
    m_ui->hostLet->setText("broker.emqx.io");
    m_ui->portLet->setText("1883");
    m_ui->clientIdLet->setText("qt_demo_" + QUuid::createUuid().toString().left(8));
    m_ui->keepAliveLet->setText("30");
    m_ui->typeCbx->addItems({"TCP", "WS", "WSS"});
    m_ui->willQosCbx->addItems({"0", "1", "2"});
    m_ui->subQosCbx->addItems({"0", "1", "2"});
    m_ui->subQosCbx->setCurrentIndex(1);
    m_ui->pubQosCbx->addItems({"0", "1", "2"});
    m_ui->pubQosCbx->setCurrentIndex(1);
    connect(m_ui->typeCbx, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        Q_UNUSED(index)
        QString type = m_ui->typeCbx->currentText();
        if (type == "WSS")
        {
            m_ui->portLet->setText("8084");
        }
        else if (type == "WS")
        {
            m_ui->portLet->setText("8083");
        }
        else
        {
            m_ui->portLet->setText("1883");
        }
    });

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
    if (m_client)
    {
        m_client->disconnect();
        m_client->disconnectFromHost();
        m_client->deleteLater();
        m_client = nullptr;
    }

    QString host = m_ui->hostLet->text();
    quint16 port = m_ui->portLet->text().toUShort();
    QString clientId = m_ui->clientIdLet->text();
    QString username = m_ui->usernameLet->text();
    QString password = m_ui->passwordLet->text();
    int keepAlive = m_ui->keepAliveLet->text().toInt();

    if (m_ui->typeCbx->currentText() == "WSS")
    {
        QString url = QString("wss://%1:%2/mqtt").arg(host).arg(port);
        m_client = new QMQTT::Client(url, "", QWebSocketProtocol::VersionLatest, false, this);
    }
    else if (m_ui->typeCbx->currentText() == "WS")
    {
        QString url = QString("ws://%1:%2/mqtt").arg(host).arg(port);
        m_client = new QMQTT::Client(url, "", QWebSocketProtocol::VersionLatest, false, this);
    }
    else
    {
        m_client = new QMQTT::Client(host, port, false, false, this);
    }

    m_client->setClientId(clientId);
    if (!username.isEmpty())
    {
        m_client->setUsername(username);
        m_client->setPassword(password.toUtf8());
    }
    m_client->setKeepAlive(static_cast<quint16>(keepAlive));
    m_client->setCleanSession(m_ui->cleanSessionCbk->isChecked());
    m_client->setAutoReconnect(true);
    m_client->setAutoReconnectInterval(5000);

    if (!m_ui->willTopicLet->text().isEmpty())
    {
        m_client->setWillTopic(m_ui->willTopicLet->text());
        m_client->setWillMessage(m_ui->willMessageLet->text().toUtf8());
        m_client->setWillQos(m_ui->willQosCbx->currentIndex());
        m_client->setWillRetain(m_ui->willRetainCbk->isChecked());
    }

    connect(m_client, &QMQTT::Client::connected, this, [this]() {
        appendMessage(tr("[Connected] %1:%2").arg(m_ui->hostLet->text()).arg(m_ui->portLet->text()), false); // [已连接] %1:%2
        updateConnectionState(true);
        if (!m_ui->selfImAccidLet->text().isEmpty())
        {
            QString topic = QString("user/%1/inbox").arg(m_ui->selfImAccidLet->text());
            m_client->subscribe(topic, static_cast<quint8>(m_ui->subQosCbx->currentIndex()));
            appendMessage(tr("[Subscribed] %1").arg(topic), false); // [已订阅] %1
        }
    });

    connect(m_client, &QMQTT::Client::disconnected, this, [this]() {
        appendMessage(tr("[Disconnected]"), false); // [已断开]
        updateConnectionState(false);
    });

    connect(m_client, &QMQTT::Client::error, this, [this](const QMQTT::ClientError error) {
        appendMessage(tr("[Error] code: %1").arg(error), false); // [错误] 代码: %1
    });

    connect(m_client, &QMQTT::Client::sslErrors, this, [this](const QList<QSslError>& errors) {
        for (const QSslError& err : errors)
        {
            appendMessage(tr("[SSL Error] %1").arg(err.errorString()), false); // [SSL 错误] %1
        }
    });

    connect(m_client, &QMQTT::Client::received, this, [this](const QMQTT::Message& msg) {
        appendMessage(tr("[Received] %1: %2").arg(msg.topic()).arg(QString::fromUtf8(msg.payload())), false); // [收到] %1: %2
    });

    m_client->connectToHost();
    appendMessage(tr("[Connecting] ..."), false); // [正在连接] ...
}

void MqttClientWgt::on_disconnectBtn_clicked()
{
    if (m_client)
    {
        m_client->setAutoReconnect(false);
        m_client->disconnectFromHost();
    }
}

void MqttClientWgt::on_subscribeBtn_clicked()
{
    if (!m_client || !m_client->isConnectedToHost())
    {
        appendMessage(tr("[Error] Not connected"), false); // [错误] 未连接
        return;
    }
    QString imAccid = m_ui->selfImAccidLet->text();
    if (imAccid.isEmpty()) return;

    QString topic = QString("user/%1/inbox").arg(imAccid);
    m_client->subscribe(topic, static_cast<quint8>(m_ui->subQosCbx->currentIndex()));
    appendMessage(tr("[Subscribe] %1 (QoS %2)").arg(topic).arg(m_ui->subQosCbx->currentText()), false); // [订阅] %1（QoS %2）
}

void MqttClientWgt::on_publishBtn_clicked()
{
    if (!m_client || !m_client->isConnectedToHost())
    {
        appendMessage(tr("[Error] Not connected"), false); // [错误] 未连接
        return;
    }

    QString target = m_ui->targetImAccidLet->text();
    QString payload = m_ui->payloadTed->toPlainText();
    if (target.isEmpty() || payload.isEmpty()) return;

    QString topic = QString("user/%1/inbox").arg(target);
    QMQTT::Message msg(0, topic, payload.toUtf8(), static_cast<quint8>(m_ui->pubQosCbx->currentIndex()));
    m_client->publish(msg);
    appendMessage(tr("[Sent] %1: %2 (QoS %3)").arg(topic).arg(payload).arg(m_ui->pubQosCbx->currentText()), true); // [已发送] %1: %2（QoS %3）
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
    m_ui->statusTextLbl->setStyleSheet(connected ? "color: green;" : "color: red;");
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
        m_translator->load("mqtt-client_zh_CN.qm", ":/i18n/");
        qApp->installTranslator(m_translator);
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
    m_ui->willTopicLet->setToolTip(tr("Last Will topic.\nBroker publishes this message when client disconnects unexpectedly.")); // 遗嘱主题，客户端异常断开时代理发布此消息。
    m_ui->willMessageLet->setToolTip(tr("Last Will payload (sent when client goes offline unexpectedly)")); // 遗嘱消息内容（客户端异常离线时发送）
    m_ui->willQosCbx->setToolTip(tr("0: at most once\n1: at least once (default)\n2: exactly once")); // 0：最多一次\n1：至少一次（默认）\n2：恰好一次
    m_ui->willRetainCbk->setToolTip(tr("If checked, broker keeps this message for late subscribers.")); // 选中后代理保留此消息给后续订阅者。
    m_ui->selfImAccidLet->setToolTip(tr("Your IM account ID.\nAuto-subscribes to user/{imAccid}/inbox on connect.")); // 您的 IM 账号。连接后自动订阅 user/{imAccid}/inbox。
    m_ui->subQosCbx->setToolTip(tr("0: at most once (fastest)\n1: at least once (may duplicate)\n2: exactly once (slowest)")); // 0：最多一次（最快）\n1：至少一次（可能重复）\n2：恰好一次（最慢）
    m_ui->targetImAccidLet->setToolTip(tr("Recipient IM account ID.\nMessage is published to user/{imAccid}/inbox.")); // 接收方 IM 账号。消息发布到 user/{imAccid}/inbox。
    m_ui->pubQosCbx->setToolTip(tr("0: at most once (fastest)\n1: at least once (may duplicate)\n2: exactly once (slowest)")); // 0：最多一次（最快）\n1：至少一次（可能重复）\n2：恰好一次（最慢）
    m_ui->payloadTed->setToolTip(tr("JSON message payload")); // JSON 消息载荷

    bool connected = m_client && m_client->isConnectedToHost();
    updateConnectionState(connected);
}


