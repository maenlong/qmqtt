#include "../../mqttclientmgr.h"

#include <QHostAddress>
#include <QSignalSpy>
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
    void slot_acceptConnection();                      // 接受 qmqtt 测试连接
    void slot_readClientData();                        // 接收 CONNECT 并返回 CONNACK

private slots:
    void init();                                       // 为每个用例启动本地测试 Broker
    void cleanup();                                    // 清理本地测试 Broker
    void slot_rejectsOperationsWhenDisconnected();    // 未连接时拒绝操作
    void slot_rejectsInvalidOperationsWhenConnected(); // 已连接时拒绝非法参数
    void slot_acceptsValidOperationsWhenConnected();  // 已连接时接受合法请求

private:
    QTcpServer* m_server = nullptr;                    // 本地测试 Broker
    QTcpSocket* m_clientSocket = nullptr;              // qmqtt 客户端连接
    QByteArray m_receivedData = QByteArray("");        // 已接收的 MQTT 数据
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

void MqttClientMgrTest::slot_acceptConnection()
{
    m_clientSocket = m_server->nextPendingConnection();
    if (m_clientSocket)
    {
        connect(m_clientSocket, &QTcpSocket::readyRead,
                this, &MqttClientMgrTest::slot_readClientData);
    }
}

void MqttClientMgrTest::slot_readClientData()
{
    m_receivedData.append(m_clientSocket->readAll());
    int packetLength = mqttPacketLength(m_receivedData);
    if (!m_connackSent && packetLength > 0
            && (static_cast<quint8>(m_receivedData.at(0)) & 0xF0) == 0x10)
    {
        m_clientSocket->write(QByteArray::fromHex("20020000"));
        m_clientSocket->flush();
        m_connackSent = true;
    }
}

void MqttClientMgrTest::init()
{
    m_clientSocket = nullptr;
    m_receivedData.clear();
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

QTEST_GUILESS_MAIN(MqttClientMgrTest)

#include "tst_mqttclientmgr.moc"
