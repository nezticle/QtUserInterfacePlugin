#ifndef UNITYANIMATIONDRIVER_H
#define UNITYANIMATIONDRIVER_H

#include <QtCore/QAnimationDriver>

class UnityAnimationDriver : public QAnimationDriver
{
public:
    UnityAnimationDriver();
    void appendDelta(float timeDelta);

public:
    void advance() override;
    qint64 elapsed() const override;
private:
    qint64 m_elapsed;
};

#endif // UNITYANIMATIONDRIVER_H
