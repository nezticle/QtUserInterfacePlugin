#include "renderdispatcher.h"
#include "uirenderer.h"
#include <QEvent>
#include <QtQml/QQmlEngine>

RenderDispatcher::RenderDispatcher(ID3D11Device *device, QObject *parent)
    : QObject(parent)
    , m_device(device)
{
    m_qmlEngine = new QQmlEngine;
}

RenderDispatcher::~RenderDispatcher()
{
    for (auto value : m_windows.values())
        delete value;
    delete m_qmlEngine;
}

void RenderDispatcher::addWindow(int id, const QSize &size, ID3D11Texture2D *textureHandle)
{
    m_windows.insert(id, new UIRenderer(size, textureHandle, this));
}

void RenderDispatcher::removeWindow(int id)
{
    auto window = m_windows[id];
    m_windows.remove(id);
    delete window;
}

void RenderDispatcher::setQmlSource(int objectId, const QUrl &qmlSource)
{

}

void RenderDispatcher::setIsVisible(int objectId, bool isVisible)
{

}

void RenderDispatcher::dispatchTouchStartEvent(int objectId, float x, float y, int touchpoint)
{
    auto target = m_windows[objectId];
    auto touchEvent = new QMouseEvent(QEvent::MouseButtonPress, QPointF(x * target->textureSize().width(), y * target->textureSize().height()), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    target->sendGeneratedTouchEvent(touchEvent);
}

void RenderDispatcher::dispatchTouchEndEvent(int objectId, float x, float y, int touchpoint)
{
    auto target = m_windows[objectId];
    auto touchEvent = new QMouseEvent(QEvent::MouseButtonRelease, QPointF(x * target->textureSize().width(), y * target->textureSize().height()), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    target->sendGeneratedTouchEvent(touchEvent);
}

void RenderDispatcher::dispatchTouchMoveEvent(int objectId, float x, float y, int touchpoint)
{
    auto target = m_windows[objectId];
    auto touchEvent = new QMouseEvent(QEvent::MouseMove, QPointF(x * target->textureSize().width(), y * target->textureSize().height()), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    target->sendGeneratedTouchEvent(touchEvent);
}

void RenderDispatcher::updateTexture(int objectId)
{
    m_windows[objectId]->updateTexture();
}

ID3D11Device *RenderDispatcher::device() const
{
    return m_device;
}

QQmlEngine *RenderDispatcher::qmlEngine() const
{
    return m_qmlEngine;
}
