#include "mqtttopicbuilder.h"

#include <QString>

bool MqttTopicBuilder::isAccountIdValid(const QString& imAccid)
{
    QString normalizedImAccid = imAccid.trimmed();
    bool valid = !normalizedImAccid.isEmpty();
    valid = valid && !normalizedImAccid.contains(QString("/"));
    valid = valid && !normalizedImAccid.contains(QString("+"));
    valid = valid && !normalizedImAccid.contains(QString("#"));
    valid = valid && !normalizedImAccid.contains(QChar(static_cast<ushort>(0)));
    return valid;
}

QString MqttTopicBuilder::inboxTopic(const QString& imAccid)
{
    QString topic = QString("");
    QString normalizedImAccid = imAccid.trimmed();
    if (isAccountIdValid(normalizedImAccid))
    {
        topic = QString("user/%1/inbox").arg(normalizedImAccid);
    }
    return topic;
}
