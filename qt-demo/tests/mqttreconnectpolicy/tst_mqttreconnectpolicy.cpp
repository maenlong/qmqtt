#include "../../mqttreconnectpolicy.h"

#include <QList>
#include <QtTest>

// MQTT 指数退避重连策略测试
class MqttReconnectPolicyTest : public QObject
{
    Q_OBJECT

private slots:
    void slot_startsWithFiveSecondDelay();       // 初始等待时间为 5 秒
    void slot_advancesWithSixtySecondCap();      // 指数增长并封顶 60 秒
    void slot_resetRestoresInitialDelay();       // 重置后恢复初始等待时间
};

void MqttReconnectPolicyTest::slot_startsWithFiveSecondDelay()
{
    MqttReconnectPolicy reconnectPolicy;

    QCOMPARE(reconnectPolicy.currentDelayMs(), 5000);
}

void MqttReconnectPolicyTest::slot_advancesWithSixtySecondCap()
{
    MqttReconnectPolicy reconnectPolicy;
    QList<int> expectedDelays;
    expectedDelays << 5000 << 10000 << 20000 << 40000 << 60000 << 60000;

    for (int index = 0; index < expectedDelays.size(); ++index)
    {
        QCOMPARE(reconnectPolicy.currentDelayMs(), expectedDelays.at(index));
        reconnectPolicy.advance();
    }
}

void MqttReconnectPolicyTest::slot_resetRestoresInitialDelay()
{
    MqttReconnectPolicy reconnectPolicy;
    reconnectPolicy.advance();
    reconnectPolicy.advance();
    QCOMPARE(reconnectPolicy.currentDelayMs(), 20000);

    reconnectPolicy.reset();

    QCOMPARE(reconnectPolicy.currentDelayMs(), 5000);
}

QTEST_GUILESS_MAIN(MqttReconnectPolicyTest)

#include "tst_mqttreconnectpolicy.moc"
