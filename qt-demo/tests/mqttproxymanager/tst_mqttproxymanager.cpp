#include "../../mqttproxymanager.h"

#include <QNetworkProxy>
#include <QtTest>

// MQTT 代理配置管理器测试
class MqttProxyManagerTest : public QObject
{
    Q_OBJECT

public:
    MqttProxyManagerTest();
    ~MqttProxyManagerTest();

private slots:
    void slot_noProxyDisablesApplicationProxy(); // 禁用全局代理
    void slot_rejectsInvalidParams_data();       // 准备无效代理参数
    void slot_rejectsInvalidParams();            // 拒绝无效参数并保留原配置
    void slot_appliesProxy_data();                // 准备有效代理参数
    void slot_appliesProxy();                     // 应用 HTTP 或 SOCKS5 代理
};

MqttProxyManagerTest::MqttProxyManagerTest()
{
    QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::NoProxy));
}

MqttProxyManagerTest::~MqttProxyManagerTest()
{
    QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::NoProxy));
}

void MqttProxyManagerTest::slot_noProxyDisablesApplicationProxy()
{
    QNetworkProxy::setApplicationProxy(
                QNetworkProxy(QNetworkProxy::HttpProxy, QString("old.proxy"), 8080));

    MqttProxyParams params;
    params.type = MqttProxyNone;

    QVERIFY(MqttProxyManager::applyProxy(params));
    QCOMPARE(QNetworkProxy::applicationProxy().type(), QNetworkProxy::NoProxy);
}

void MqttProxyManagerTest::slot_rejectsInvalidParams_data()
{
    QTest::addColumn<int>("proxyType");
    QTest::addColumn<QString>("host");
    QTest::addColumn<quint16>("port");

    QTest::newRow("empty host")
            << static_cast<int>(MqttProxyHttp) << QString("") << static_cast<quint16>(8080);
    QTest::newRow("blank host")
            << static_cast<int>(MqttProxySocks5) << QString("   ") << static_cast<quint16>(1080);
    QTest::newRow("zero port")
            << static_cast<int>(MqttProxyHttp) << QString("proxy.local") << static_cast<quint16>(0);
    QTest::newRow("unknown type")
            << 99 << QString("proxy.local") << static_cast<quint16>(8080);
}

void MqttProxyManagerTest::slot_rejectsInvalidParams()
{
    QFETCH(int, proxyType);
    QFETCH(QString, host);
    QFETCH(quint16, port);

    QNetworkProxy oldProxy(QNetworkProxy::HttpProxy, QString("old.proxy"), 3128,
                           QString("old-user"), QString("old-password"));
    QNetworkProxy::setApplicationProxy(oldProxy);

    MqttProxyParams params;
    params.type = static_cast<MqttProxyType>(proxyType);
    params.host = host;
    params.port = port;

    QVERIFY(!MqttProxyManager::applyProxy(params));

    QNetworkProxy currentProxy = QNetworkProxy::applicationProxy();
    QCOMPARE(currentProxy.type(), oldProxy.type());
    QCOMPARE(currentProxy.hostName(), oldProxy.hostName());
    QCOMPARE(currentProxy.port(), oldProxy.port());
    QCOMPARE(currentProxy.user(), oldProxy.user());
    QCOMPARE(currentProxy.password(), oldProxy.password());
}

void MqttProxyManagerTest::slot_appliesProxy_data()
{
    QTest::addColumn<int>("proxyType");
    QTest::addColumn<int>("expectedNetworkProxyType");
    QTest::addColumn<quint16>("port");

    QTest::newRow("http proxy")
            << static_cast<int>(MqttProxyHttp)
            << static_cast<int>(QNetworkProxy::HttpProxy)
            << static_cast<quint16>(8080);
    QTest::newRow("socks5 proxy")
            << static_cast<int>(MqttProxySocks5)
            << static_cast<int>(QNetworkProxy::Socks5Proxy)
            << static_cast<quint16>(1080);
}

void MqttProxyManagerTest::slot_appliesProxy()
{
    QFETCH(int, proxyType);
    QFETCH(int, expectedNetworkProxyType);
    QFETCH(quint16, port);

    MqttProxyParams params;
    params.type = static_cast<MqttProxyType>(proxyType);
    params.host = QString("  proxy.local  ");
    params.port = port;
    params.username = QString("  demo-user  ");
    params.password = QString("demo-password");

    QVERIFY(MqttProxyManager::applyProxy(params));

    QNetworkProxy currentProxy = QNetworkProxy::applicationProxy();
    QCOMPARE(static_cast<int>(currentProxy.type()), expectedNetworkProxyType);
    QCOMPARE(currentProxy.hostName(), QString("proxy.local"));
    QCOMPARE(currentProxy.port(), port);
    QCOMPARE(currentProxy.user(), QString("demo-user"));
    QCOMPARE(currentProxy.password(), QString("demo-password"));
}

QTEST_GUILESS_MAIN(MqttProxyManagerTest)

#include "tst_mqttproxymanager.moc"
