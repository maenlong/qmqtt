#include "mqttclientmgr.h"
#include "qmqtt.h"
#include <QSslError>
#include <QtWebSockets/QWebSocketProtocol>

MqttClientMgr::MqttClientMgr(QObject* parent)
    : QObject(parent)
{
}

MqttClientMgr::~MqttClientMgr()
{
    if (m_client)
    {
        m_client->disconnect();
        m_client->disconnectFromHost();
        m_client->deleteLater();
        m_client = nullptr;
    }
}

void MqttClientMgr::connectToHost(const MqttConnectionParams& params)
{
    if (m_client)
    {
        m_client->disconnect();
        m_client->disconnectFromHost();
        m_client->deleteLater();
        m_client = nullptr;
    }

    createClient(params);
    applyClientConfig(params);
    forwardClientSignals();

    m_client->connectToHost();
}

void MqttClientMgr::disconnectFromHost()
{
    if (m_client)
    {
        m_client->setAutoReconnect(false);
        m_client->disconnectFromHost();
    }
}

void MqttClientMgr::subscribe(const QString& topic, quint8 qos)
{
    if (m_client)
    {
        m_client->subscribe(topic, qos);
    }
}

void MqttClientMgr::publish(const QString& topic, const QByteArray& payload, quint8 qos)
{
    if (m_client)
    {
        QMQTT::Message msg(0, topic, payload, qos);
        m_client->publish(msg);
    }
}

bool MqttClientMgr::isConnected() const
{
    return m_client && m_client->isConnectedToHost();
}

void MqttClientMgr::createClient(const MqttConnectionParams& params)
{
    if (params.type == 2)
    {
        QString url = QString("wss://%1:%2/mqtt").arg(params.host).arg(params.port);
        m_client = new QMQTT::Client(url, QString(""), QWebSocketProtocol::VersionLatest, false, this);
    }
    else if (params.type == 1)
    {
        QString url = QString("ws://%1:%2/mqtt").arg(params.host).arg(params.port);
        m_client = new QMQTT::Client(url, QString(""), QWebSocketProtocol::VersionLatest, false, this);
    }
    else
    {
        m_client = new QMQTT::Client(params.host, params.port, false, false, this);
    }

    m_client->setAutoReconnect(true);
    m_client->setAutoReconnectInterval(5000);
}

void MqttClientMgr::applyClientConfig(const MqttConnectionParams& params)
{
    m_client->setClientId(params.clientId);
    if (!params.username.isEmpty())
    {
        m_client->setUsername(params.username);
        m_client->setPassword(params.password.toUtf8());
    }
    m_client->setKeepAlive(static_cast<quint16>(params.keepAlive));
    m_client->setCleanSession(params.cleanSession);

    if (!params.willTopic.isEmpty())
    {
        m_client->setWillTopic(params.willTopic);
        m_client->setWillMessage(params.willMessage.toUtf8());
        m_client->setWillQos(static_cast<quint8>(params.willQos));
        m_client->setWillRetain(params.willRetain);
    }
}

void MqttClientMgr::forwardClientSignals()
{
    connect(m_client, &QMQTT::Client::connected,
            this, &MqttClientMgr::sig_connected);

    connect(m_client, &QMQTT::Client::disconnected,
            this, &MqttClientMgr::sig_disconnected);

    connect(m_client, &QMQTT::Client::error,
            this, [this](const QMQTT::ClientError error) {
        emit sig_error(static_cast<int>(error));
    });

    connect(m_client, &QMQTT::Client::sslErrors,
            this, [this](const QList<QSslError>& errors) {
        emit sig_sslErrors(errors);
    });

    connect(m_client, &QMQTT::Client::received,
            this, [this](const QMQTT::Message& msg) {
        emit sig_messageReceived(msg.topic(), msg.payload());
    });
}
