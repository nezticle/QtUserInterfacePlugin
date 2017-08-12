#ifndef UIRENDERER_H
#define UIRENDERER_H

#include <QtCore/QObject>
#include <QtCore/QSize>
#include <QtGui/QImage>
#include <QtGui/QTouchEvent>
#include <QtCore/QBasicTimer>
#include <QtCore/QMutex>
#include <d3d11.h>

class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOffscreenSurface;
class QQuickRenderControl;
class QQuickWindow;
class QQmlEngine;
class QQmlComponent;
class QQuickItem;
class RenderDispatcher;

class UIRenderer : public QObject
{
    Q_OBJECT
public:
    explicit UIRenderer(const QSize &textureSize, ID3D11Texture2D *textureHandle, RenderDispatcher *dispatcher);
    ~UIRenderer();

    float unityTime() const;
    void setUnityTime(float unityTime);

    QSize textureSize() const;
    void setTextureSize(const QSize &textureSize);

    void render();
    void updateTexture();

    bool loadQML(const QString &qmlFile);

    void sendGeneratedTouchEvent(QEvent *event);

public slots:
    void triggerUpdate();

private:
    float m_unityTime;
    QSize m_textureSize;
    ID3D11Texture2D* m_textureHandle;
    ID3D11Device* m_device;
    QImage m_framebuffer;

    // Render Control
    QOpenGLContext *m_context;
    QOffscreenSurface *m_offscreenSurface;
    QQuickRenderControl *m_renderControl;
    QOpenGLFramebufferObject *m_fbo;
    QQuickWindow *m_offscreenWindow;
    QQmlEngine *m_qmlEngine;
    QQmlComponent *m_qmlComponent;
    QQuickItem *m_rootItem;
    bool m_isReady;

    bool m_isUpdatePending;
    QBasicTimer m_updateTimer;
    QMutex m_mutex;

    // QObject interface
protected:
    void timerEvent(QTimerEvent *event) override;
};

#endif // UIRENDERER_H
