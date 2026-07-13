#include "mqttreconnectpolicy.h"

namespace {
const int InitialReconnectDelayMs = 5000;
const int MaximumReconnectDelayMs = 60000;
}

MqttReconnectPolicy::MqttReconnectPolicy()
{
    reset();
}

int MqttReconnectPolicy::currentDelayMs() const
{
    return m_currentDelayMs;
}

void MqttReconnectPolicy::advance()
{
    if (m_currentDelayMs < MaximumReconnectDelayMs)
    {
        m_currentDelayMs *= 2;
        if (m_currentDelayMs > MaximumReconnectDelayMs)
        {
            m_currentDelayMs = MaximumReconnectDelayMs;
        }
    }
}

void MqttReconnectPolicy::reset()
{
    m_currentDelayMs = InitialReconnectDelayMs;
}
