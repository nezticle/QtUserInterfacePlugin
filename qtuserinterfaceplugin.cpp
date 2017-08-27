#include "qtuserinterfaceplugin.h"
#include "uirenderer.h"
#include "renderdispatcher.h"

#include <d3d11.h>
#include "Unity/IUnityGraphicsD3D11.h"

#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickWindow>
#include <QtGui/QMouseEvent>

static IUnityInterfaces* s_UnityInterfaces = nullptr;
static IUnityGraphics* s_Graphics = nullptr;
static UnityGfxRenderer s_RendererType = kUnityGfxRendererNull;
static QGuiApplication *s_qtApp;
char arg0[] = "UnityUI";
char* argv[] = { &arg0[0], NULL };
int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
static RenderDispatcher *s_renderDispatcher = nullptr;

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
                if (s_renderDispatcher)
                    delete s_renderDispatcher;
                s_renderDispatcher = new RenderDispatcher(d3d->GetDevice());
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
    //rendering code...
    Q_ASSERT(s_renderDispatcher);
    s_renderDispatcher->updateTexture(eventID);
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
    QQuickWindow::setSceneGraphBackend(QSGRendererInterface::OpenGL);
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
    delete s_renderDispatcher;
    delete s_qtApp;
    s_qtApp = nullptr;
    s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

void UpdateQtEventLoop()
{
    s_qtApp->processEvents();
}

UnityRenderingEvent GetRenderEventFunc()
{
    return OnRenderEvent;
}

UnityRenderingEventAndData GetRenderEventAndDataFunc()
{
    return OnRenderEventAndData;
}

void SetTextureFromUnity(int objectId, void *textureHandle, int width, int height)
{
    // A script calls this at initialization time; just remember the texture pointer here.
    // Will update texture pixels each frame from the plugin rendering event (texture update
    // needs to happen on the rendering thread).
    Q_ASSERT(s_renderDispatcher);
    s_renderDispatcher->addWindow(objectId, QSize(width, height), (ID3D11Texture2D*)textureHandle);
}

void RegisterTouchStartEvent(int objectId, float x, float y, int touchpoint)
{
    Q_ASSERT(s_renderDispatcher);
    s_renderDispatcher->dispatchTouchStartEvent(objectId, x, y, touchpoint);
}

void RegisterTouchEndEvent(int objectId, float x, float y, int touchpoint)
{
    Q_ASSERT(s_renderDispatcher);
    s_renderDispatcher->dispatchTouchEndEvent(objectId, x, y, touchpoint);
}

void RegisterTouchMoveEvent(int objectId, float x, float y, int touchpoint)
{
    Q_ASSERT(s_renderDispatcher);
    s_renderDispatcher->dispatchTouchMoveEvent(objectId, x, y, touchpoint);
}

void RemoveUIObject(int objectId)
{
    s_renderDispatcher->removeWindow(objectId);
}

void UpdateAnimations(float time)
{
    s_renderDispatcher->updateAnimationTime(time);
}
