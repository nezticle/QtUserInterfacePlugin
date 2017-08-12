#ifndef RENDERDISPATCHER_H
#define RENDERDISPATCHER_H

#include <QObject>
#include <QHash>

#include <d3d11.h>

class UIRenderer;
class QQmlEngine;

class RenderDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit RenderDispatcher(ID3D11Device *device, QObject *parent = nullptr);
    ~RenderDispatcher();

    void addWindow(int id, const QSize &size, ID3D11Texture2D* textureHandle);
    void removeWindow(int id);

    void setQmlSource(int objectId, const QUrl &qmlSource);
    void setIsVisible(int objectId, bool isVisible);

    void dispatchTouchStartEvent(int objectId, float x, float y, int touchpoint);
    void dispatchTouchEndEvent(int objectId, float x, float y, int touchpoint);
    void dispatchTouchMoveEvent(int objectId, float x, float y, int touchpoint);

    void updateTexture(int objectId);

    ID3D11Device *device() const;
    QQmlEngine *qmlEngine() const;

private:
    ID3D11Device *m_device;
    QQmlEngine *m_qmlEngine;
    QHash<int, UIRenderer*> m_windows;
};

#endif // RENDERDISPATCHER_H
