#include "mainwgt.h"
#include "ui_mainwgt.h"
#include <QUuid>
#include <QSslError>
#include <QtWebSockets/QWebSocketProtocol>

MainWgt::MainWgt(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::MainWgt)
    , m_client(nullptr)
{
    m_ui->setupUi(this);
    setWindowTitle("MQTT Client Demo");

    m_ui->m_host->setText("broker.emqx.io");
    m_ui->m_port->setText("1883");
    m_ui->m_clientId->setText("qt_demo_" + QUuid::createUuid().toString().left(8));
    m_ui->m_keepAlive->setText("30");
    m_ui->m_type->addItems({"TCP", "WS", "WSS"});
    m_ui->m_willQos->addItems({"0", "1", "2"});
    m_ui->m_subQos->addItems({"0", "1", "2"});
    m_ui->m_subQos->setCurrentIndex(1);
    m_ui->m_pubQos->addItems({"0", "1", "2"});
    m_ui->m_pubQos->setCurrentIndex(1);

    m_ui->m_host->setToolTip("MQTT broker address, e.g. broker.emqx.io");
    m_ui->m_port->setToolTip("1883 = TCP, 8083 = WS, 8084 = WSS");
    m_ui->m_type->setToolTip("TCP: raw MQTT\nWS: WebSocket (port 8083)\nWSS: WebSocket + TLS (port 8084)");
    m_ui->m_clientId->setToolTip("Unique client identifier.\nBroker uses it to distinguish clients.");
    m_ui->m_username->setToolTip("Optional: broker authentication");
    m_ui->m_password->setToolTip("Optional: broker authentication password");
    m_ui->m_keepAlive->setToolTip("Heartbeat interval (seconds).\nIf broker receives no packet within 1.5x this interval, client is considered disconnected.");
    m_ui->m_cleanSession->setToolTip("ON: start a fresh session, discard old subscriptions & offline messages.\nOFF: broker preserves subscriptions & offline messages across reconnects.");
    m_ui->m_willTopic->setToolTip("Last Will topic.\nBroker publishes this message when client disconnects unexpectedly.");
    m_ui->m_willMessage->setToolTip("Last Will payload (sent when client goes offline unexpectedly)");
    m_ui->m_willQos->setToolTip("0: at most once\n1: at least once (default)\n2: exactly once");
    m_ui->m_willRetain->setToolTip("If checked, broker keeps this message for late subscribers.");
    m_ui->m_selfImAccid->setToolTip("Your IM account ID.\nAuto-subscribes to user/{imAccid}/inbox on connect.");
    m_ui->m_subQos->setToolTip("0: at most once (fastest)\n1: at least once (may duplicate)\n2: exactly once (slowest)");
    m_ui->m_targetImAccid->setToolTip("Recipient IM account ID.\nMessage is published to user/{imAccid}/inbox.");
    m_ui->m_pubQos->setToolTip("0: at most once (fastest)\n1: at least once (may duplicate)\n2: exactly once (slowest)");
    m_ui->m_payload->setToolTip("JSON message payload");

    connect(m_ui->m_type, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        Q_UNUSED(index)
        QString type = m_ui->m_type->currentText();
        if (type == "WSS") {
            m_ui->m_port->setText("8084");
        } else if (type == "WS") {
            m_ui->m_port->setText("8083");
        } else {
            m_ui->m_port->setText("1883");
        }
    });

    connect(m_ui->m_btnConnect, &QPushButton::clicked, this, &MainWgt::sig_connect);
    connect(m_ui->m_btnDisconnect, &QPushButton::clicked, this, &MainWgt::sig_disconnect);
    connect(m_ui->m_btnSubscribe, &QPushButton::clicked, this, &MainWgt::sig_subscribe);
    connect(m_ui->m_btnPublish, &QPushButton::clicked, this, &MainWgt::sig_publish);

    m_ui->m_btnDisconnect->setEnabled(false);
    m_ui->m_status->setText("Disconnected");
}

MainWgt::~MainWgt()
{
    delete m_ui;
}

void MainWgt::sig_connect()
{
    if (m_client) {
        m_client->disconnect();
        m_client->disconnectFromHost();
        m_client->deleteLater();
        m_client = nullptr;
    }

    QString host = m_ui->m_host->text();
    quint16 port = m_ui->m_port->text().toUShort();
    QString clientId = m_ui->m_clientId->text();
    QString username = m_ui->m_username->text();
    QString password = m_ui->m_password->text();
    int keepAlive = m_ui->m_keepAlive->text().toInt();

    if (m_ui->m_type->currentText() == "WSS") {
        QString url = QString("wss://%1:%2/mqtt").arg(host).arg(port);
        m_client = new QMQTT::Client(url, "", QWebSocketProtocol::VersionLatest, false, this);
    } else if (m_ui->m_type->currentText() == "WS") {
        QString url = QString("ws://%1:%2/mqtt").arg(host).arg(port);
        m_client = new QMQTT::Client(url, "", QWebSocketProtocol::VersionLatest, false, this);
    } else {
        m_client = new QMQTT::Client(host, port, false, false, this);
    }

    m_client->setClientId(clientId);
    if (!username.isEmpty()) {
        m_client->setUsername(username);
        m_client->setPassword(password.toUtf8());
    }
    m_client->setKeepAlive(static_cast<quint16>(keepAlive));
    m_client->setCleanSession(m_ui->m_cleanSession->isChecked());
    m_client->setAutoReconnect(true);
    m_client->setAutoReconnectInterval(5000);

    if (!m_ui->m_willTopic->text().isEmpty()) {
        m_client->setWillTopic(m_ui->m_willTopic->text());
        m_client->setWillMessage(m_ui->m_willMessage->text().toUtf8());
        m_client->setWillQos(m_ui->m_willQos->currentIndex());
        m_client->setWillRetain(m_ui->m_willRetain->isChecked());
    }

    connect(m_client, &QMQTT::Client::connected, this, [this]() {
        appendMessage("[Connected] " + m_ui->m_host->text() + ":" + m_ui->m_port->text(), false);
        updateConnectionState(true);
        if (!m_ui->m_selfImAccid->text().isEmpty()) {
            QString topic = QString("user/%1/inbox").arg(m_ui->m_selfImAccid->text());
            m_client->subscribe(topic, static_cast<quint8>(m_ui->m_subQos->currentIndex()));
            appendMessage("[Subscribed] " + topic, false);
        }
    });

    connect(m_client, &QMQTT::Client::disconnected, this, [this]() {
        appendMessage("[Disconnected]", false);
        updateConnectionState(false);
    });

    connect(m_client, &QMQTT::Client::error, this, [this](const QMQTT::ClientError error) {
        appendMessage("[Error] code: " + QString::number(error), false);
    });

    connect(m_client, &QMQTT::Client::sslErrors, this, [this](const QList<QSslError>& errors) {
        for (const QSslError& err : errors) {
            appendMessage("[SSL Error] " + err.errorString(), false);
        }
    });

    connect(m_client, &QMQTT::Client::received, this, [this](const QMQTT::Message& msg) {
        appendMessage("[Received] " + msg.topic() + ": " + QString::fromUtf8(msg.payload()), false);
    });

    m_client->connectToHost();
    appendMessage("[Connecting] ...", false);
}

void MainWgt::sig_disconnect()
{
    if (m_client) {
        m_client->setAutoReconnect(false);
        m_client->disconnectFromHost();
    }
}

void MainWgt::sig_subscribe()
{
    if (!m_client || !m_client->isConnectedToHost()) {
        appendMessage("[Error] Not connected", false);
        return;
    }
    QString imAccid = m_ui->m_selfImAccid->text();
    if (imAccid.isEmpty()) return;

    QString topic = QString("user/%1/inbox").arg(imAccid);
    m_client->subscribe(topic, static_cast<quint8>(m_ui->m_subQos->currentIndex()));
    appendMessage("[Subscribe] " + topic + " (QoS " + m_ui->m_subQos->currentText() + ")", false);
}

void MainWgt::sig_publish()
{
    if (!m_client || !m_client->isConnectedToHost()) {
        appendMessage("[Error] Not connected", false);
        return;
    }

    QString target = m_ui->m_targetImAccid->text();
    QString payload = m_ui->m_payload->toPlainText();
    if (target.isEmpty() || payload.isEmpty()) return;

    QString topic = QString("user/%1/inbox").arg(target);
    QMQTT::Message msg(0, topic, payload.toUtf8(), static_cast<quint8>(m_ui->m_pubQos->currentIndex()));
    m_client->publish(msg);
    appendMessage("[Sent] " + topic + ": " + payload + " (QoS " + m_ui->m_pubQos->currentText() + ")", true);
}

void MainWgt::appendMessage(const QString& msg, bool isSent)
{
    m_ui->m_log->appendPlainText((isSent ? ">> " : "<< ") + msg);
}

void MainWgt::updateConnectionState(bool connected)
{
    m_ui->m_btnConnect->setEnabled(!connected);
    m_ui->m_btnDisconnect->setEnabled(connected);
    m_ui->m_status->setText(connected ? "Connected" : "Disconnected");
    m_ui->m_status->setStyleSheet(connected ? "color: green;" : "color: red;");
}
