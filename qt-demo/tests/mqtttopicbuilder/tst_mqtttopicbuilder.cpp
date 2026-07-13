#include "../../mqtttopicbuilder.h"

#include <QtTest>

// MQTT Topic 构建工具测试
class MqttTopicBuilderTest : public QObject
{
    Q_OBJECT

private slots:
    void slot_emptyAccountReturnsEmpty();       // 空账号不生成 Topic
    void slot_buildsInboxTopic_data();          // 准备有效账号测试数据
    void slot_buildsInboxTopic();               // 生成标准 inbox Topic
};

void MqttTopicBuilderTest::slot_emptyAccountReturnsEmpty()
{
    QCOMPARE(MqttTopicBuilder::inboxTopic(QString("")), QString(""));
    QCOMPARE(MqttTopicBuilder::inboxTopic(QString("   ")), QString(""));
}

void MqttTopicBuilderTest::slot_buildsInboxTopic_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("expectedTopic");

    QTest::newRow("plain account")
            << QString("alice") << QString("user/alice/inbox");
    QTest::newRow("trimmed account")
            << QString("  alice-01  ") << QString("user/alice-01/inbox");
    QTest::newRow("unicode account")
            << QString::fromUtf8("用户01") << QString::fromUtf8("user/用户01/inbox");
}

void MqttTopicBuilderTest::slot_buildsInboxTopic()
{
    QFETCH(QString, accountId);
    QFETCH(QString, expectedTopic);

    QCOMPARE(MqttTopicBuilder::inboxTopic(accountId), expectedTopic);
}

QTEST_GUILESS_MAIN(MqttTopicBuilderTest)

#include "tst_mqtttopicbuilder.moc"
