#include "mainwgt.h"
#include "qmqtt.h"
#include "ui_mainwgt.h"
#include <QUuid>
#include <QSslError>
#include <QtWebSockets/QWebSocketProtocol>

MainWgt::MainWgt(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::MainWgt)
{
    m_ui->setupUi(this);
    setWindowTitle("MQTT Client Demo");

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

    m_ui->hostLet->setToolTip("MQTT broker address, e.g. broker.emqx.io");
    m_ui->portLet->setToolTip("1883 = TCP, 8083 = WS, 8084 = WSS");
    m_ui->typeCbx->setToolTip("TCP: raw MQTT\nWS: WebSocket (port 8083)\nWSS: WebSocket + TLS (port 8084)");
    m_ui->clientIdLet->setToolTip("Unique client identifier.\nBroker uses it to distinguish clients.");
    m_ui->usernameLet->setToolTip("Optional: broker authentication");
    m_ui->passwordLet->setToolTip("Optional: broker authentication password");
    m_ui->keepAliveLet->setToolTip("Heartbeat interval (seconds).\nIf broker receives no packet within 1.5x this interval, client is considered disconnected.");
    m_ui->cleanSessionCbk->setToolTip("ON: start a fresh session, discard old subscriptions & offline messages.\nOFF: broker preserves subscriptions & offline messages across reconnects.");
    m_ui->willTopicLet->setToolTip("Last Will topic.\nBroker publishes this message when client disconnects unexpectedly.");
    m_ui->willMessageLet->setToolTip("Last Will payload (sent when client goes offline unexpectedly)");
    m_ui->willQosCbx->setToolTip("0: at most once\n1: at least once (default)\n2: exactly once");
    m_ui->willRetainCbk->setToolTip("If checked, broker keeps this message for late subscribers.");
    m_ui->selfImAccidLet->setToolTip("Your IM account ID.\nAuto-subscribes to user/{imAccid}/inbox on connect.");
    m_ui->subQosCbx->setToolTip("0: at most once (fastest)\n1: at least once (may duplicate)\n2: exactly once (slowest)");
    m_ui->targetImAccidLet->setToolTip("Recipient IM account ID.\nMessage is published to user/{imAccid}/inbox.");
    m_ui->pubQosCbx->setToolTip("0: at most once (fastest)\n1: at least once (may duplicate)\n2: exactly once (slowest)");
    m_ui->payloadTed->setToolTip("JSON message payload");

    connect(m_ui->typeCbx, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        Q_UNUSED(index)
        QString type = m_ui->typeCbx->currentText();
        if (type == "WSS") {
            m_ui->portLet->setText("8084");
        } else if (type == "WS") {
            m_ui->portLet->setText("8083");
        } else {
            m_ui->portLet->setText("1883");
        }
    });

    m_ui->mainLayout->setStretch(m_ui->mainLayout->indexOf(m_ui->logGrp), 1);

    m_ui->disconnectBtn->setEnabled(false);
    m_ui->statusTextLbl->setText("Disconnected");
}

MainWgt::~MainWgt()
{
    delete m_ui;
}

void MainWgt::on_connectBtn_clicked()
{
    if (m_client) {
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

    if (m_ui->typeCbx->currentText() == "WSS") {
        QString url = QString("wss://%1:%2/mqtt").arg(host).arg(port);
        m_client = new QMQTT::Client(url, "", QWebSocketProtocol::VersionLatest, false, this);
    } else if (m_ui->typeCbx->currentText() == "WS") {
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
    m_client->setCleanSession(m_ui->cleanSessionCbk->isChecked());
    m_client->setAutoReconnect(true);
    m_client->setAutoReconnectInterval(5000);

    if (!m_ui->willTopicLet->text().isEmpty()) {
        m_client->setWillTopic(m_ui->willTopicLet->text());
        m_client->setWillMessage(m_ui->willMessageLet->text().toUtf8());
        m_client->setWillQos(m_ui->willQosCbx->currentIndex());
        m_client->setWillRetain(m_ui->willRetainCbk->isChecked());
    }

    connect(m_client, &QMQTT::Client::connected, this, [this]() {
        appendMessage("[Connected] " + m_ui->hostLet->text() + ":" + m_ui->portLet->text(), false);
        updateConnectionState(true);
        if (!m_ui->selfImAccidLet->text().isEmpty()) {
            QString topic = QString("user/%1/inbox").arg(m_ui->selfImAccidLet->text());
            m_client->subscribe(topic, static_cast<quint8>(m_ui->subQosCbx->currentIndex()));
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

void MainWgt::on_disconnectBtn_clicked()
{
    if (m_client) {
        m_client->setAutoReconnect(false);
        m_client->disconnectFromHost();
    }
}

void MainWgt::on_subscribeBtn_clicked()
{
    if (!m_client || !m_client->isConnectedToHost()) {
        appendMessage("[Error] Not connected", false);
        return;
    }
    QString imAccid = m_ui->selfImAccidLet->text();
    if (imAccid.isEmpty()) return;

    QString topic = QString("user/%1/inbox").arg(imAccid);
    m_client->subscribe(topic, static_cast<quint8>(m_ui->subQosCbx->currentIndex()));
    appendMessage("[Subscribe] " + topic + " (QoS " + m_ui->subQosCbx->currentText() + ")", false);
}

void MainWgt::on_publishBtn_clicked()
{
    if (!m_client || !m_client->isConnectedToHost()) {
        appendMessage("[Error] Not connected", false);
        return;
    }

    QString target = m_ui->targetImAccidLet->text();
    QString payload = m_ui->payloadTed->toPlainText();
    if (target.isEmpty() || payload.isEmpty()) return;

    QString topic = QString("user/%1/inbox").arg(target);
    QMQTT::Message msg(0, topic, payload.toUtf8(), static_cast<quint8>(m_ui->pubQosCbx->currentIndex()));
    m_client->publish(msg);
    appendMessage("[Sent] " + topic + ": " + payload + " (QoS " + m_ui->pubQosCbx->currentText() + ")", true);
}

void MainWgt::on_clearLogBtn_clicked()
{
    m_ui->logTed->clear();
}

void MainWgt::appendMessage(const QString& msg, bool isSent)
{
    m_ui->logTed->appendPlainText((isSent ? ">> " : "<< ") + msg);
}

void MainWgt::updateConnectionState(bool connected)
{
    m_ui->connectBtn->setEnabled(!connected);
    m_ui->disconnectBtn->setEnabled(connected);
    m_ui->statusTextLbl->setText(connected ? "Connected" : "Disconnected");
    m_ui->statusTextLbl->setStyleSheet(connected ? "color: green;" : "color: red;");
}

