#ifndef MQTTCLIENTMGR_H
#define MQTTCLIENTMGR_H

#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>

class QSslError;

namespace QMQTT {
class Client;
}

// MQTT 连接参数
struct MqttConnectionParams
{
    QString host;
    quint16 port;
    int     type;           // 0=TCP, 1=WS, 2=WSS
    QString clientId;
    QString username;
    QString password;
    int     keepAlive;
    bool    cleanSession;
    QString willTopic;
    QString willMessage;
    int     willQos;
    bool    willRetain;
};

// MQTT 客户端管理类，封装连接、订阅、发布逻辑，与 UI 无关
class MqttClientMgr : public QObject
{
    Q_OBJECT

public:
    explicit MqttClientMgr(QObject* parent = nullptr);
    ~MqttClientMgr();

    void connectToHost(const MqttConnectionParams& params);
    void disconnectFromHost();
    void subscribe(const QString& topic, quint8 qos);
    void unsubscribe(const QString& topic);
    void publish(const QString& topic, const QByteArray& payload, quint8 qos);
    bool isConnected() const;

signals:
    void sig_connected();
    void sig_disconnected();
    void sig_error(int errorCode);
    void sig_sslErrors(const QList<QSslError>& errors);
    void sig_messageReceived(const QString& topic, const QByteArray& payload);

private:
    void createClient(const MqttConnectionParams& params);
    void applyClientConfig(const MqttConnectionParams& params);
    void forwardClientSignals();

    QMQTT::Client* m_client = nullptr;
};

#endif
