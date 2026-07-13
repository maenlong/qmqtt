#include "mqttproxymanager.h"

#include <QNetworkProxy>

bool MqttProxyManager::applyProxy(const MqttProxyParams& params)
{
    bool success = false;
    if (params.type == MqttProxyNone)
    {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
        success = true;
    }
    else if ((params.type == MqttProxyHttp || params.type == MqttProxySocks5)
             && !params.host.trimmed().isEmpty() && params.port > 0)
    {
        QNetworkProxy proxy;
        if (params.type == MqttProxyHttp)
        {
            proxy.setType(QNetworkProxy::HttpProxy);
        }
        else
        {
            proxy.setType(QNetworkProxy::Socks5Proxy);
        }
        proxy.setHostName(params.host.trimmed());
        proxy.setPort(params.port);

        QString username = params.username.trimmed();
        if (!username.isEmpty())
        {
            proxy.setUser(username);
            proxy.setPassword(params.password);
        }

        QNetworkProxy::setApplicationProxy(proxy);
        success = true;
    }
    return success;
}
