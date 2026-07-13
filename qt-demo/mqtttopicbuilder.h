#ifndef MQTTTOPICBUILDER_H
#define MQTTTOPICBUILDER_H

class QString;

// MQTT 业务主题构建工具
class MqttTopicBuilder
{
public:
    static bool isAccountIdValid(const QString& imAccid); // 校验账号能否作为单个 Topic 层级
    static QString inboxTopic(const QString& imAccid); // 构建用户收件箱主题，账号无效时返回空字符串
};

#endif
