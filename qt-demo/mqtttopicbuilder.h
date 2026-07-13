#ifndef MQTTTOPICBUILDER_H
#define MQTTTOPICBUILDER_H

class QString;

// MQTT 业务主题构建工具
class MqttTopicBuilder
{
public:
    static QString inboxTopic(const QString& imAccid); // 构建用户收件箱主题，账号为空时返回空字符串
};

#endif
