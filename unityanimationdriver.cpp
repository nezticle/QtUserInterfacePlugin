#include "unityanimationdriver.h"

UnityAnimationDriver::UnityAnimationDriver()
    : m_elapsed(0)
{
}

void UnityAnimationDriver::appendDelta(float timeDelta)
{
    qint64 timeDeltaMs = static_cast<qint64>(timeDelta * 1000.f);
    m_elapsed += timeDeltaMs;
}

void UnityAnimationDriver::advance()
{
    //do nothing
}

qint64 UnityAnimationDriver::elapsed() const
{
    return m_elapsed;
}
