#include "mqttclientmgr.h"
#include "qmqtt.h"
#include <QSslError>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QTimer>
#include <QUrl>
#include <QtWebSockets/QWebSocketProtocol>

namespace {
const int InitialReconnectIntervalMs = 5000;
const int MaximumReconnectIntervalMs = 60000;
}

MqttClientMgr::MqttClientMgr(QObject* parent)
    : QObject(parent)
{
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout,
            this, &MqttClientMgr::slot_reconnect);
}

MqttClientMgr::~MqttClientMgr()
{
    m_reconnectAllowed = false;
    m_reconnectTimer->stop();
    if (m_client)
    {
        m_client->disconnect();
        m_client->disconnectFromHost();
        m_client->deleteLater();
        m_client = nullptr;
    }
}

bool MqttClientMgr::connectToHost(const MqttConnectionParams& params)
{
    bool success = false;
    bool paramsValid = isConnectionParamsValid(params);
    bool caCertificateValid = true;
    QString caCertificatePath = params.sslCaCertPath.trimmed();
    if (params.type == MqttConnectionWss && !caCertificatePath.isEmpty())
    {
        caCertificateValid = !QSslCertificate::fromPath(caCertificatePath).isEmpty();
    }

    if (!caCertificateValid)
    {
        emit sig_caCertificateLoadFailed(caCertificatePath);
    }
    else if (!paramsValid)
    {
        emit sig_connectionParamsInvalid();
    }
    else
    {
        resetReconnectState();

        if (m_client)
        {
            m_client->disconnect();
            m_client->disconnectFromHost();
            m_client->deleteLater();
            m_client = nullptr;
        }

        MqttConnectionParams normalizedParams = params;
        normalizedParams.host = params.host.trimmed();
        normalizedParams.clientId = params.clientId.trimmed();

        createClient(normalizedParams);
        applyClientConfig(normalizedParams);
        forwardClientSignals();

        m_client->connectToHost();
        success = true;
    }
    return success;
}

void MqttClientMgr::disconnectFromHost()
{
    m_manualDisconnect = true;
    m_reconnectAllowed = false;
    m_reconnectTimer->stop();
    if (m_client)
    {
        m_client->disconnectFromHost();
    }
}

bool MqttClientMgr::subscribe(const QString& topic, quint8 qos)
{
    bool success = isConnected() && !topic.isEmpty() && qos <= 2;
    if (success)
    {
        m_client->subscribe(topic, qos);
    }
    return success;
}

bool MqttClientMgr::unsubscribe(const QString& topic)
{
    bool success = isConnected() && !topic.isEmpty();
    if (success)
    {
        m_client->unsubscribe(topic);
    }
    return success;
}

bool MqttClientMgr::publish(const QString& topic, const QByteArray& payload, quint8 qos)
{
    bool success = isConnected() && !topic.isEmpty() && qos <= 2;
    if (success)
    {
        QMQTT::Message msg(0, topic, payload, qos);
        m_client->publish(msg);
    }
    return success;
}

bool MqttClientMgr::isConnected() const
{
    return m_client && m_client->isConnectedToHost();
}

bool MqttClientMgr::isConnectionParamsValid(const MqttConnectionParams& params) const
{
    bool valid = !params.host.trimmed().isEmpty();
    valid = valid && params.port > 0;
    valid = valid && (params.type == MqttConnectionTcp
                      || params.type == MqttConnectionWs
                      || params.type == MqttConnectionWss);
    valid = valid && !params.clientId.trimmed().isEmpty();
    valid = valid && params.keepAlive >= 0 && params.keepAlive <= 65535;
    valid = valid && params.willQos >= 0 && params.willQos <= 2;
    if (valid && (params.type == MqttConnectionWs || params.type == MqttConnectionWss))
    {
        valid = !webSocketUrl(params).isEmpty();
    }
    return valid;
}

bool MqttClientMgr::isPermanentConnectionError(int errorCode) const
{
    return errorCode == QMQTT::SocketProxyAuthenticationRequiredError
            || errorCode == QMQTT::MqttUnacceptableProtocolVersionError
            || errorCode == QMQTT::MqttIdentifierRejectedError
            || errorCode == QMQTT::MqttBadUserNameOrPasswordError
            || errorCode == QMQTT::MqttNotAuthorizedError;
}

QString MqttClientMgr::webSocketUrl(const MqttConnectionParams& params) const
{
    QString result = QString("");
    QString host = params.host.trimmed();
    if (host.startsWith(QString("[")) && host.endsWith(QString("]")))
    {
        host = host.mid(1, host.length() - 2);
    }

    QString path = params.websocketPath.trimmed();
    if (path.isEmpty())
    {
        path = QString("/");
    }
    else if (!path.startsWith(QString("/")))
    {
        path.prepend(QString("/"));
    }

    QUrl url;
    url.setScheme(params.type == MqttConnectionWss ? QString("wss") : QString("ws"));
    url.setHost(host);
    url.setPort(params.port);
    url.setPath(path);
    if (url.isValid() && !url.host().isEmpty())
    {
        result = url.toString();
    }
    return result;
}

void MqttClientMgr::createClient(const MqttConnectionParams& params)
{
    if (params.type == MqttConnectionWs || params.type == MqttConnectionWss)
    {
        bool ignoreSelfSigned = params.type == MqttConnectionWss && params.ignoreSelfSigned;
        m_client = new QMQTT::Client(webSocketUrl(params), params.websocketOrigin.trimmed(),
                                    QWebSocketProtocol::VersionLatest, ignoreSelfSigned, this);
    }
    else
    {
        m_client = new QMQTT::Client(params.host, params.port, false, false, this);
    }

    // 由管理层统一实施指数退避，避免 qmqtt 固定间隔重连与业务策略冲突。
    m_client->setAutoReconnect(false);
}

void MqttClientMgr::applyClientConfig(const MqttConnectionParams& params)
{
    m_client->setVersion(QMQTT::V3_1_1);
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

    if (params.type == MqttConnectionWss && !params.sslCaCertPath.trimmed().isEmpty())
    {
        QSslConfiguration sslConfig = m_client->sslConfiguration();
        QList<QSslCertificate> caCerts = QSslCertificate::fromPath(params.sslCaCertPath.trimmed());
        if (!caCerts.isEmpty())
        {
            sslConfig.addCaCertificates(caCerts);
            m_client->setSslConfiguration(sslConfig);
        }
    }
}

void MqttClientMgr::forwardClientSignals()
{
    connect(m_client, &QMQTT::Client::connected,
            this, [this]()
    {
        m_reconnectTimer->stop();
        m_reconnectIntervalMs = InitialReconnectIntervalMs;
        emit sig_connected();
    });

    connect(m_client, &QMQTT::Client::disconnected,
            this, [this]()
    {
        emit sig_disconnected();
        scheduleReconnect();
    });

    connect(m_client, &QMQTT::Client::error,
            this, [this](const QMQTT::ClientError error)
    {
        int errorCode = static_cast<int>(error);
        if (isPermanentConnectionError(errorCode))
        {
            m_reconnectAllowed = false;
            m_reconnectTimer->stop();
            emit sig_reconnectStopped(errorCode);
            m_client->disconnectFromHost();
        }
        emit sig_error(errorCode);
    });

    connect(m_client, &QMQTT::Client::sslErrors,
            this, [this](const QList<QSslError>& errors)
    {
        emit sig_sslErrors(errors);
    });

    connect(m_client, &QMQTT::Client::received,
            this, [this](const QMQTT::Message& msg)
    {
        emit sig_messageReceived(msg.topic(), msg.payload());
    });

    connect(m_client, &QMQTT::Client::subscribed,
            this, &MqttClientMgr::sig_subscribed);

    connect(m_client, &QMQTT::Client::unsubscribed,
            this, &MqttClientMgr::sig_unsubscribed);

    connect(m_client, &QMQTT::Client::published,
            this, [this](const QMQTT::Message& msg, quint16 messageId)
    {
        emit sig_published(msg.topic(), msg.qos(), messageId);
    });

    connect(m_client, &QMQTT::Client::pingresp,
            this, &MqttClientMgr::sig_pingResp);
}

void MqttClientMgr::resetReconnectState()
{
    m_reconnectTimer->stop();
    m_reconnectIntervalMs = InitialReconnectIntervalMs;
    m_manualDisconnect = false;
    m_reconnectAllowed = true;
}

void MqttClientMgr::scheduleReconnect()
{
    if (!m_client || m_manualDisconnect || !m_reconnectAllowed
            || m_reconnectTimer->isActive())
    {
        return;
    }

    int delayMs = m_reconnectIntervalMs;
    m_reconnectTimer->start(delayMs);
    emit sig_reconnectScheduled(delayMs / 1000);

    if (m_reconnectIntervalMs < MaximumReconnectIntervalMs)
    {
        m_reconnectIntervalMs *= 2;
        if (m_reconnectIntervalMs > MaximumReconnectIntervalMs)
        {
            m_reconnectIntervalMs = MaximumReconnectIntervalMs;
        }
    }
}

void MqttClientMgr::slot_reconnect()
{
    if (m_client && !m_manualDisconnect && m_reconnectAllowed
            && !m_client->isConnectedToHost())
    {
        m_client->connectToHost();
    }
}
