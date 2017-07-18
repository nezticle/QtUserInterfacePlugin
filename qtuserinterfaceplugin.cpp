#include "qtuserinterfaceplugin.h"
#include "uirenderer.h"

#include <d3d11.h>
#include "Unity/IUnityGraphicsD3D11.h"

#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickWindow>

static IUnityInterfaces* s_UnityInterfaces = nullptr;
static IUnityGraphics* s_Graphics = nullptr;
static UnityGfxRenderer s_RendererType = kUnityGfxRendererNull;
static QGuiApplication *s_qtApp;
char arg0[] = "UnityUI";
char* argv[] = { &arg0[0], NULL };
int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
static UIRenderer *s_uiRenderer = nullptr;

static void UNITY_INTERFACE_API
    OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    switch (eventType)
    {
        case kUnityGfxDeviceEventInitialize:
        {
            s_RendererType = s_Graphics->GetRenderer();
            if (s_RendererType == kUnityGfxRendererD3D11) {
                IUnityGraphicsD3D11* d3d = s_UnityInterfaces->Get<IUnityGraphicsD3D11>();
                s_uiRenderer->setDevice(d3d->GetDevice());
            }
            break;
        }
        case kUnityGfxDeviceEventShutdown:
        {
            s_RendererType = kUnityGfxRendererNull;
            break;
        }
        case kUnityGfxDeviceEventBeforeReset:
        {
            break;
        }
        case kUnityGfxDeviceEventAfterReset:
        {
            break;
        }
    };
}

static void UNITY_INTERFACE_API
    OnRenderEvent(int eventID)
{
    Q_UNUSED(eventID)
    //rendering code...
    s_uiRenderer->updateTexture();
}

static void UNITY_INTERFACE_API
    OnRenderEventAndData(int eventId, void* data)
{
    Q_UNUSED(eventId)
    Q_UNUSED(data)
}

void UnityPluginLoad(IUnityInterfaces *unityInterfaces)
{
    s_qtApp = new QGuiApplication(argc, &argv[0]);
    QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
    s_uiRenderer = new UIRenderer();
    s_UnityInterfaces = unityInterfaces;
    s_Graphics = unityInterfaces->Get<IUnityGraphics>();
    s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

    // Run OnGraphicsDeviceEvent(initialize) manually on plugin load
    // to not miss the event in case the graphics device is already initialized
    OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

void UnityPluginUnload()
{
    s_qtApp->quit();
    s_qtApp->processEvents();
    delete s_uiRenderer;
    delete s_qtApp;
    s_qtApp = nullptr;
    s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

UnityRenderingEvent GetRenderEventFunc()
{
    return OnRenderEvent;
}

UnityRenderingEventAndData GetRenderEventAndDataFunc()
{
    return OnRenderEventAndData;
}

void SetTextureFromUnity(void *textureHandle, int width, int height)
{
    // A script calls this at initialization time; just remember the texture pointer here.
    // Will update texture pixels each frame from the plugin rendering event (texture update
    // needs to happen on the rendering thread).
    s_uiRenderer->setTextureHandle((ID3D11Texture2D*)textureHandle);
    s_uiRenderer->setTextureSize(QSize(width, height));
}

void UpdateQtEventLoop()
{
    s_qtApp->processEvents();
}

void SetTimeFromUnity(float time)
{
    s_uiRenderer->setUnityTime(time);
}
