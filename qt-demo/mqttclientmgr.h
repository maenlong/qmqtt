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
    QString host;                  // Broker 地址
    quint16 port = 0;              // Broker 端口
    int     type = 0;              // 连接类型：0=TCP, 1=WS, 2=WSS
    QString clientId;              // 客户端标识
    QString username;              // 认证用户名
    QString password;              // 认证密码
    int     keepAlive = -1;        // 心跳间隔（秒）
    bool    cleanSession = true;   // 是否清理会话
    QString sslCaCertPath;         // CA 证书文件路径
    bool    ignoreSelfSigned = false; // 是否忽略自签名证书错误
    QString willTopic;             // 遗嘱主题
    QString willMessage;           // 遗嘱消息
    int     willQos = -1;          // 遗嘱 QoS
    bool    willRetain = false;    // 是否保留遗嘱
};

// MQTT 客户端管理类，封装连接、订阅、发布逻辑，与 UI 无关
class MqttClientMgr : public QObject
{
    Q_OBJECT

public:
    explicit MqttClientMgr(QObject* parent = nullptr);
    ~MqttClientMgr();

    void connectToHost(const MqttConnectionParams& params);    // 连接 MQTT Broker
    void disconnectFromHost();                                 // 断开连接
    void subscribe(const QString& topic, quint8 qos);          // 订阅主题
    void unsubscribe(const QString& topic);                    // 取消订阅
    void publish(const QString& topic, const QByteArray& payload, quint8 qos); // 发布消息
    bool isConnected() const;                                  // 当前是否已连接

signals:
    void sig_connected();              // MQTT 连接成功
    void sig_disconnected();           // MQTT 断开连接
    void sig_error(int errorCode);     // MQTT 错误
    void sig_sslErrors(const QList<QSslError>& errors);  // SSL 证书错误
    void sig_messageReceived(const QString& topic, const QByteArray& payload);  // 收到消息
    void sig_pingresp();               // 收到心跳回复（PINGRESP）

private:
    void createClient(const MqttConnectionParams& params);      // 创建 qmqtt 客户端实例
    void applyClientConfig(const MqttConnectionParams& params); // 应用连接参数
    void forwardClientSignals();                                // 透传 qmqtt 信号

    QMQTT::Client* m_client = nullptr;   // qmqtt 客户端实例
};

#endif
