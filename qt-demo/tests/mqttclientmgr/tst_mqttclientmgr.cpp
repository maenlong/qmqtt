#include "../../mqttclientmgr.h"

#include "qmqtt.h"

#include <QHostAddress>
#include <QMetaObject>
#include <QSignalSpy>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>

// MQTT 客户端管理层接口测试
class MqttClientMgrTest : public QObject
{
    Q_OBJECT

private:
    MqttConnectionParams connectionParams() const;     // 生成本地测试连接参数
    int mqttPacketLength(const QByteArray& data) const; // 计算首个完整 MQTT 数据包长度
    int mqttPayloadOffset(const QByteArray& data) const; // 计算 MQTT 有效载荷偏移
    void handleSubscribePacket(const QByteArray& packet); // 记录订阅并返回 SUBACK
    void handleUnsubscribePacket(const QByteArray& packet); // 返回 UNSUBACK
    void slot_acceptConnection();                      // 接受 qmqtt 测试连接
    void slot_readClientData();                        // 处理 qmqtt 测试数据包

private slots:
    void init();                                       // 为每个用例启动本地测试 Broker
    void cleanup();                                    // 清理本地测试 Broker
    void slot_rejectsOperationsWhenDisconnected();    // 未连接时拒绝操作
    void slot_rejectsInvalidAutoSubscriptionQos();     // 拒绝非法自动订阅 QoS
    void slot_rejectsInvalidOperationsWhenConnected(); // 已连接时拒绝非法参数
    void slot_acceptsValidOperationsWhenConnected();   // 已连接时接受合法请求
    void slot_schedulesReconnectAfterTemporaryDisconnect(); // 临时断线后安排重连
    void slot_manualDisconnectDoesNotReconnect();      // 主动断开后不再重连
    void slot_authFailureStopsReconnect();             // 鉴权失败后停止重连
    void slot_restoresSubscriptionAfterReconnect();    // 重连后恢复订阅
    void slot_unsubscribePreventsSubscriptionRestore(); // 取消订阅后不再恢复

private:
    QTcpServer* m_server = nullptr;                    // 本地测试 Broker
    QTcpSocket* m_clientSocket = nullptr;              // qmqtt 客户端连接
    QByteArray m_receivedData = QByteArray("");        // 已接收的 MQTT 数据
    QStringList m_subscribedTopics;                    // Broker 收到的订阅 Topic
    QStringList m_unsubscribedTopics;                  // Broker 收到的取消订阅 Topic
    QList<quint8> m_subscribedQos;                     // Broker 收到的订阅 QoS
    quint8 m_connackReturnCode = 0;                    // CONNACK 返回码
    bool m_connackSent = false;                        // 是否已经返回 CONNACK
};

MqttConnectionParams MqttClientMgrTest::connectionParams() const
{
    MqttConnectionParams params;
    params.host = QString("127.0.0.1");
    params.port = m_server->serverPort();
    params.clientId = QString("mqtt-client-manager-test");
    params.keepAlive = 0;
    return params;
}

int MqttClientMgrTest::mqttPacketLength(const QByteArray& data) const
{
    int packetLength = -1;
    if (data.size() >= 2)
    {
        int remainingLength = 0;
        int multiplier = 1;
        int index = 1;
        bool lengthComplete = false;
        while (index < data.size() && index <= 4 && !lengthComplete)
        {
            quint8 byte = static_cast<quint8>(data.at(index));
            remainingLength += static_cast<int>(byte & 0x7F) * multiplier;
            multiplier *= 128;
            ++index;
            lengthComplete = (byte & 0x80) == 0;
        }

        if (lengthComplete && data.size() >= index + remainingLength)
        {
            packetLength = index + remainingLength;
        }
    }
    return packetLength;
}

int MqttClientMgrTest::mqttPayloadOffset(const QByteArray& data) const
{
    int payloadOffset = -1;
    if (data.size() >= 2)
    {
        int index = 1;
        bool lengthComplete = false;
        while (index < data.size() && index <= 4 && !lengthComplete)
        {
            quint8 byte = static_cast<quint8>(data.at(index));
            ++index;
            lengthComplete = (byte & 0x80) == 0;
        }
        if (lengthComplete)
        {
            payloadOffset = index;
        }
    }
    return payloadOffset;
}

void MqttClientMgrTest::handleSubscribePacket(const QByteArray& packet)
{
    int payloadOffset = mqttPayloadOffset(packet);
    if (payloadOffset > 0 && packet.size() >= payloadOffset + 5)
    {
        quint16 topicLength = (static_cast<quint8>(packet.at(payloadOffset + 2)) << 8)
                | static_cast<quint8>(packet.at(payloadOffset + 3));
        int qosOffset = payloadOffset + 4 + topicLength;
        if (packet.size() > qosOffset)
        {
            QString topic = QString::fromUtf8(packet.mid(payloadOffset + 4, topicLength));
            quint8 qos = static_cast<quint8>(packet.at(qosOffset));
            m_subscribedTopics.append(topic);
            m_subscribedQos.append(qos);

            QByteArray suback = QByteArray::fromHex("9003");
            suback.append(packet.at(payloadOffset));
            suback.append(packet.at(payloadOffset + 1));
            suback.append(static_cast<char>(qos));
            m_clientSocket->write(suback);
        }
    }
}

void MqttClientMgrTest::handleUnsubscribePacket(const QByteArray& packet)
{
    int payloadOffset = mqttPayloadOffset(packet);
    if (payloadOffset > 0 && packet.size() >= payloadOffset + 4)
    {
        quint16 topicLength = (static_cast<quint8>(packet.at(payloadOffset + 2)) << 8)
                | static_cast<quint8>(packet.at(payloadOffset + 3));
        if (packet.size() >= payloadOffset + 4 + topicLength)
        {
            m_unsubscribedTopics.append(
                        QString::fromUtf8(packet.mid(payloadOffset + 4, topicLength)));

            QByteArray unsuback = QByteArray::fromHex("B002");
            unsuback.append(packet.at(payloadOffset));
            unsuback.append(packet.at(payloadOffset + 1));
            m_clientSocket->write(unsuback);
        }
    }
}

void MqttClientMgrTest::slot_acceptConnection()
{
    m_clientSocket = m_server->nextPendingConnection();
    if (m_clientSocket)
    {
        m_receivedData.clear();
        m_connackSent = false;
        connect(m_clientSocket, &QTcpSocket::readyRead,
                this, &MqttClientMgrTest::slot_readClientData);
    }
}

void MqttClientMgrTest::slot_readClientData()
{
    m_receivedData.append(m_clientSocket->readAll());
    int packetLength = mqttPacketLength(m_receivedData);
    while (packetLength > 0)
    {
        QByteArray packet = m_receivedData.left(packetLength);
        m_receivedData.remove(0, packetLength);
        quint8 packetType = static_cast<quint8>(packet.at(0)) & 0xF0;
        if (!m_connackSent && packetType == 0x10)
        {
            QByteArray connack = QByteArray::fromHex("20020000");
            connack[3] = static_cast<char>(m_connackReturnCode);
            m_clientSocket->write(connack);
            m_connackSent = true;
        }
        else if (packetType == 0x80)
        {
            handleSubscribePacket(packet);
        }
        else if (packetType == 0xA0)
        {
            handleUnsubscribePacket(packet);
        }
        packetLength = mqttPacketLength(m_receivedData);
    }
    m_clientSocket->flush();
}

void MqttClientMgrTest::init()
{
    m_clientSocket = nullptr;
    m_receivedData.clear();
    m_subscribedTopics.clear();
    m_unsubscribedTopics.clear();
    m_subscribedQos.clear();
    m_connackReturnCode = 0;
    m_connackSent = false;
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection,
            this, &MqttClientMgrTest::slot_acceptConnection);
    QVERIFY(m_server->listen(QHostAddress::LocalHost, 0));
}

void MqttClientMgrTest::cleanup()
{
    delete m_server;
    m_server = nullptr;
    m_clientSocket = nullptr;
}

void MqttClientMgrTest::slot_rejectsOperationsWhenDisconnected()
{
    MqttClientMgr mqttClientMgr;

    QVERIFY(!mqttClientMgr.subscribe(QString("user/alice/inbox"), 1));
    QVERIFY(!mqttClientMgr.unsubscribe(QString("user/alice/inbox")));
    QVERIFY(!mqttClientMgr.publish(QString("user/alice/inbox"), QByteArray("message"), 1));
}

void MqttClientMgrTest::slot_rejectsInvalidAutoSubscriptionQos()
{
    MqttConnectionParams params = connectionParams();
    params.autoSubscribeTopic = QString("user/alice/inbox");
    params.autoSubscribeQos = 3;

    MqttClientMgr mqttClientMgr;
    QSignalSpy invalidParamsSpy(&mqttClientMgr,
                                &MqttClientMgr::sig_connectionParamsInvalid);
    QVERIFY(invalidParamsSpy.isValid());
    QVERIFY(!mqttClientMgr.connectToHost(params));
    QCOMPARE(invalidParamsSpy.count(), 1);
}

void MqttClientMgrTest::slot_rejectsInvalidOperationsWhenConnected()
{
    MqttClientMgr mqttClientMgr;
    QSignalSpy connectedSpy(&mqttClientMgr, &MqttClientMgr::sig_connected);
    QVERIFY(connectedSpy.isValid());
    QVERIFY(mqttClientMgr.connectToHost(connectionParams()));
    QTRY_COMPARE(connectedSpy.count(), 1);

    QVERIFY(!mqttClientMgr.subscribe(QString(""), 0));
    QVERIFY(!mqttClientMgr.subscribe(QString("user/alice/inbox"), 3));
    QVERIFY(!mqttClientMgr.unsubscribe(QString("")));
    QVERIFY(!mqttClientMgr.publish(QString(""), QByteArray("message"), 0));
    QVERIFY(!mqttClientMgr.publish(QString("user/alice/inbox"), QByteArray("message"), 3));
}

void MqttClientMgrTest::slot_acceptsValidOperationsWhenConnected()
{
    MqttClientMgr mqttClientMgr;
    QSignalSpy connectedSpy(&mqttClientMgr, &MqttClientMgr::sig_connected);
    QVERIFY(connectedSpy.isValid());
    QVERIFY(mqttClientMgr.connectToHost(connectionParams()));
    QTRY_COMPARE(connectedSpy.count(), 1);

    QVERIFY(mqttClientMgr.subscribe(QString("user/alice/inbox"), 0));
    QVERIFY(mqttClientMgr.subscribe(QString("user/alice/inbox"), 1));
    QVERIFY(mqttClientMgr.subscribe(QString("user/alice/inbox"), 2));
    QVERIFY(mqttClientMgr.unsubscribe(QString("user/alice/inbox")));
    QVERIFY(mqttClientMgr.publish(QString("user/alice/inbox"), QByteArray(""), 0));
    QVERIFY(mqttClientMgr.publish(QString("user/alice/inbox"), QByteArray("message"), 1));
    QVERIFY(mqttClientMgr.publish(QString("user/alice/inbox"), QByteArray("message"), 2));
}

void MqttClientMgrTest::slot_schedulesReconnectAfterTemporaryDisconnect()
{
    MqttClientMgr mqttClientMgr;
    QSignalSpy connectedSpy(&mqttClientMgr, &MqttClientMgr::sig_connected);
    QSignalSpy reconnectScheduledSpy(&mqttClientMgr,
                                     &MqttClientMgr::sig_reconnectScheduled);
    QVERIFY(connectedSpy.isValid());
    QVERIFY(reconnectScheduledSpy.isValid());
    QVERIFY(mqttClientMgr.connectToHost(connectionParams()));
    QTRY_COMPARE(connectedSpy.count(), 1);
    QVERIFY(m_clientSocket);

    m_clientSocket->abort();

    QTRY_COMPARE(reconnectScheduledSpy.count(), 1);
    QCOMPARE(reconnectScheduledSpy.at(0).at(0).toInt(), 5);
}

void MqttClientMgrTest::slot_manualDisconnectDoesNotReconnect()
{
    MqttClientMgr mqttClientMgr;
    QSignalSpy connectedSpy(&mqttClientMgr, &MqttClientMgr::sig_connected);
    QSignalSpy disconnectedSpy(&mqttClientMgr, &MqttClientMgr::sig_disconnected);
    QSignalSpy reconnectScheduledSpy(&mqttClientMgr,
                                     &MqttClientMgr::sig_reconnectScheduled);
    QVERIFY(connectedSpy.isValid());
    QVERIFY(disconnectedSpy.isValid());
    QVERIFY(reconnectScheduledSpy.isValid());
    QVERIFY(mqttClientMgr.connectToHost(connectionParams()));
    QTRY_COMPARE(connectedSpy.count(), 1);

    mqttClientMgr.disconnectFromHost();

    QTRY_COMPARE(disconnectedSpy.count(), 1);
    QCOMPARE(reconnectScheduledSpy.count(), 0);
}

void MqttClientMgrTest::slot_authFailureStopsReconnect()
{
    m_connackReturnCode = 5;
    MqttClientMgr mqttClientMgr;
    QSignalSpy errorSpy(&mqttClientMgr, &MqttClientMgr::sig_error);
    QSignalSpy reconnectScheduledSpy(&mqttClientMgr,
                                     &MqttClientMgr::sig_reconnectScheduled);
    QSignalSpy reconnectStoppedSpy(&mqttClientMgr,
                                   &MqttClientMgr::sig_reconnectStopped);
    QVERIFY(errorSpy.isValid());
    QVERIFY(reconnectScheduledSpy.isValid());
    QVERIFY(reconnectStoppedSpy.isValid());
    QVERIFY(mqttClientMgr.connectToHost(connectionParams()));

    QTRY_COMPARE(reconnectStoppedSpy.count(), 1);
    QTRY_COMPARE(errorSpy.count(), 1);
    QCOMPARE(reconnectStoppedSpy.at(0).at(0).toInt(),
             static_cast<int>(QMQTT::MqttNotAuthorizedError));
    QCOMPARE(errorSpy.at(0).at(0).toInt(),
             static_cast<int>(QMQTT::MqttNotAuthorizedError));
    QCOMPARE(reconnectScheduledSpy.count(), 0);
}

void MqttClientMgrTest::slot_restoresSubscriptionAfterReconnect()
{
    QString topic = QString("user/alice/inbox");
    MqttConnectionParams params = connectionParams();
    params.autoSubscribeTopic = topic;
    params.autoSubscribeQos = 1;

    MqttClientMgr mqttClientMgr;
    QSignalSpy connectedSpy(&mqttClientMgr, &MqttClientMgr::sig_connected);
    QSignalSpy subscribedSpy(&mqttClientMgr, &MqttClientMgr::sig_subscribed);
    QSignalSpy reconnectScheduledSpy(&mqttClientMgr,
                                     &MqttClientMgr::sig_reconnectScheduled);
    QVERIFY(connectedSpy.isValid());
    QVERIFY(subscribedSpy.isValid());
    QVERIFY(reconnectScheduledSpy.isValid());
    QVERIFY(mqttClientMgr.connectToHost(params));
    QTRY_COMPARE(connectedSpy.count(), 1);
    QTRY_COMPARE(subscribedSpy.count(), 1);
    QCOMPARE(m_subscribedTopics, QStringList() << topic);
    QCOMPARE(m_subscribedQos, QList<quint8>() << static_cast<quint8>(1));

    m_clientSocket->abort();
    QTRY_COMPARE(reconnectScheduledSpy.count(), 1);
    QVERIFY(QMetaObject::invokeMethod(&mqttClientMgr, "slot_reconnect",
                                      Qt::DirectConnection));

    QTRY_COMPARE(connectedSpy.count(), 2);
    QTRY_COMPARE(subscribedSpy.count(), 2);
    QCOMPARE(m_subscribedTopics, QStringList() << topic << topic);
    QCOMPARE(m_subscribedQos, QList<quint8>()
             << static_cast<quint8>(1) << static_cast<quint8>(1));
}

void MqttClientMgrTest::slot_unsubscribePreventsSubscriptionRestore()
{
    QString topic = QString("user/alice/inbox");
    MqttConnectionParams params = connectionParams();
    params.autoSubscribeTopic = topic;
    params.autoSubscribeQos = 1;

    MqttClientMgr mqttClientMgr;
    QSignalSpy connectedSpy(&mqttClientMgr, &MqttClientMgr::sig_connected);
    QSignalSpy subscribedSpy(&mqttClientMgr, &MqttClientMgr::sig_subscribed);
    QSignalSpy reconnectScheduledSpy(&mqttClientMgr,
                                     &MqttClientMgr::sig_reconnectScheduled);
    QVERIFY(connectedSpy.isValid());
    QVERIFY(subscribedSpy.isValid());
    QVERIFY(reconnectScheduledSpy.isValid());
    QVERIFY(mqttClientMgr.connectToHost(params));
    QTRY_COMPARE(connectedSpy.count(), 1);
    QTRY_COMPARE(subscribedSpy.count(), 1);
    QVERIFY(mqttClientMgr.unsubscribe(topic));
    QTRY_COMPARE(m_unsubscribedTopics, QStringList() << topic);

    m_clientSocket->abort();
    QTRY_COMPARE(reconnectScheduledSpy.count(), 1);
    QVERIFY(QMetaObject::invokeMethod(&mqttClientMgr, "slot_reconnect",
                                      Qt::DirectConnection));

    QTRY_COMPARE(connectedSpy.count(), 2);
    QTest::qWait(50);
    QCOMPARE(subscribedSpy.count(), 1);
    QCOMPARE(m_subscribedTopics, QStringList() << topic);
}

QTEST_GUILESS_MAIN(MqttClientMgrTest)

#include "tst_mqttclientmgr.moc"
