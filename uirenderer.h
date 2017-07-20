#ifndef UIRENDERER_H
#define UIRENDERER_H

#include <QtCore/QObject>
#include <QtCore/QSize>
#include <QtGui/QImage>
#include <QtGui/QTouchEvent>
#include <QtCore/QBasicTimer>
#include <QtCore/QMutex>
#include <d3d11.h>

class QQuickRenderControl;
class QQuickWindow;
class QQmlEngine;
class QQmlComponent;
class QQuickItem;


class UIRenderer : public QObject
{
    Q_OBJECT
public:
    explicit UIRenderer(QObject *parent = nullptr);

    float unityTime() const;
    void setUnityTime(float unityTime);

    QSize textureSize() const;
    void setTextureSize(const QSize &textureSize);

    ID3D11Texture2D *textureHandle() const;
    void setTextureHandle(ID3D11Texture2D *textureHandle);

    ID3D11Device *device() const;
    void setDevice(ID3D11Device *device);

    void render();
    void updateTexture();

    bool loadQML(const QString &qmlFile);

    void sendGeneratedTouchEvent(QEvent *event);
signals:

public slots:
    void triggerUpdate();

private:
    float m_unityTime;
    QSize m_textureSize;
    ID3D11Texture2D* m_textureHandle;
    ID3D11Device* m_device;
    QImage m_framebuffer;

    // Render Control
    QQuickRenderControl *m_renderControl;
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
