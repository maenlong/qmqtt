#include "mqtttopicbuilder.h"

#include <QString>

QString MqttTopicBuilder::inboxTopic(const QString& imAccid)
{
    QString topic = QString("");
    QString normalizedImAccid = imAccid.trimmed();
    if (!normalizedImAccid.isEmpty())
    {
        topic = QString("user/%1/inbox").arg(normalizedImAccid);
    }
    return topic;
}
