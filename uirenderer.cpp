#include "uirenderer.h"
#include "renderdispatcher.h"
#include <QtQuick/QQuickRenderControl>
#include <QtQuick/QQuickWindow>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickItem>
#include <QtQuick/private/qquickwindow_p.h>
#include <QtQuick/private/qsgsoftwarerenderer_p.h>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLFunctions>

#include <QtCore/QDateTime>

#include <QFile>
#include <QTextStream>

UIRenderer::UIRenderer(const QSize &textureSize, ID3D11Texture2D *textureHandle, RenderDispatcher *dispatcher)
    : QObject(dispatcher)
    , m_unityTime(0.0f)
    , m_textureSize(textureSize)
    , m_textureHandle(textureHandle)
    , m_device(dispatcher->device())
    , m_framebuffer(m_textureSize, QImage::Format_ARGB32_Premultiplied)
    , m_qmlComponent(nullptr)
    , m_rootItem(nullptr)
    , m_isReady(false)
    , m_isUpdatePending(false)
{
    logError("test!");

    m_framebuffer.fill(Qt::transparent);
    QSurfaceFormat format;
    format.setDepthBufferSize(16);
    format.setStencilBufferSize(8);

    m_context = new QOpenGLContext(this);
    m_context->setFormat(format);
    bool contextCreated = m_context->create();
    if (!contextCreated) {
        logError("OpenGLContext Creation failed");
    }
    Q_ASSERT(contextCreated);

    m_offscreenSurface = new QOffscreenSurface;
    m_offscreenSurface->setFormat(m_context->format());
    m_offscreenSurface->create();

    m_renderControl = new QQuickRenderControl(this);
    m_offscreenWindow = new QQuickWindow(m_renderControl);
    m_offscreenWindow->setGeometry(0, 0, m_textureSize.width(), m_textureSize.height());

    m_qmlEngine = dispatcher->qmlEngine();
    if (!m_qmlEngine->incubationController())
        m_qmlEngine->setIncubationController(m_offscreenWindow->incubationController());

    bool isCurrent = m_context->makeCurrent(m_offscreenSurface);
    if (!isCurrent) {
        logError("OpenGL Context MakeCurrent failed");
    }
    Q_ASSERT(isCurrent);


    m_fbo = new QOpenGLFramebufferObject(m_textureSize, QOpenGLFramebufferObject::CombinedDepthStencil);
    m_offscreenWindow->setRenderTarget(m_fbo);

    QObject::connect(m_renderControl, SIGNAL(renderRequested()), this, SLOT(triggerUpdate()));
    QObject::connect(m_renderControl, SIGNAL(sceneChanged()), this, SLOT(triggerUpdate()));

    // DEBUG
    m_isReady = loadQML("qrc:/qml/calqlatr/calqlatr.qml");

    m_renderControl->initialize(m_context);
}

UIRenderer::~UIRenderer()
{
    m_context->makeCurrent(m_offscreenSurface);
    delete m_renderControl;
    delete m_qmlComponent;
    delete m_offscreenWindow;
    delete m_fbo;
    m_context->doneCurrent();
    delete m_offscreenSurface;
    delete m_context;
}

float UIRenderer::unityTime() const
{
    return m_unityTime;
}

void UIRenderer::setUnityTime(float unityTime)
{
    m_unityTime = unityTime;
}

QSize UIRenderer::textureSize() const
{
    return m_textureSize;
}

void UIRenderer::setTextureSize(const QSize &textureSize)
{
    if (m_textureSize != textureSize) {
        m_textureSize = textureSize;
        if (m_fbo)
            delete m_fbo;
        m_fbo = new QOpenGLFramebufferObject(m_textureSize, QOpenGLFramebufferObject::CombinedDepthStencil);
        m_offscreenWindow->setRenderTarget(m_fbo);
        m_offscreenWindow->setGeometry(0, 0, m_textureSize.width(), m_textureSize.height());
        if (m_rootItem) {
            m_rootItem->setWidth(m_textureSize.width());
            m_rootItem->setHeight(m_textureSize.height());
        }
    }
}

void UIRenderer::render()
{
    m_mutex.lock();
    if (m_isReady) {
        if (!m_context->makeCurrent(m_offscreenSurface))
            logError("failed to makeCurrent");
        m_renderControl->polishItems();
        m_renderControl->sync();
        m_renderControl->render();
        m_context->functions()->glFlush();
        m_framebuffer = m_fbo->toImage();
//        static int imageCounter = 0;
//        m_framebuffer.save(QString("image" + QString::number(imageCounter++) + ".png"));
        m_context->doneCurrent();
    } else {
        m_framebuffer.fill(Qt::magenta);
    }
    m_mutex.unlock();
}

void UIRenderer::updateTexture()
{
    ID3D11DeviceContext* ctx = nullptr;
    m_device->GetImmediateContext(&ctx);
    // Update texture data
    m_mutex.lock();
    ctx->UpdateSubresource(m_textureHandle, 0, nullptr, m_framebuffer.constBits(), m_textureSize.width() * 4, 0);
    m_mutex.unlock();
    ctx->Release();
}

bool UIRenderer::loadQML(const QString &qmlFile)
{

    if (m_qmlComponent != nullptr)
        delete m_qmlComponent;
    m_qmlComponent = new QQmlComponent(m_qmlEngine, QUrl(qmlFile), QQmlComponent::PreferSynchronous);

    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            logError(QString(error.url().toString() + error.line() + error.toString()));
        Q_ASSERT(false);

        return false;
    }

    QObject *rootObject = m_qmlComponent->create();
    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            logError(QString(error.url().toString() + error.line() + error.toString()));
        Q_ASSERT(false);
        return false;
    }

    m_rootItem = qobject_cast<QQuickItem *>(rootObject);
    if (!m_rootItem) {
        logError("run: Not a QQuickItem");
        delete rootObject;
        Q_ASSERT(false);
        return false;
    }

    // The root item is ready. Associate it with the window.
    m_rootItem->setParentItem(m_offscreenWindow->contentItem());

    m_rootItem->setWidth(m_textureSize.width());
    m_rootItem->setHeight(m_textureSize.height());

    m_offscreenWindow->setGeometry(0, 0, m_textureSize.width(), m_textureSize.height());

    return true;
}

void UIRenderer::sendGeneratedTouchEvent(QEvent *event)
{
    if (!m_isReady || !m_offscreenWindow) {
        delete event;
        return;
    }

    QWindow *window = qobject_cast<QWindow*>(m_offscreenWindow);
    if (window)
        QGuiApplication::postEvent(window, event);
}

void UIRenderer::triggerUpdate()
{
    if (!m_isUpdatePending) {
        const int exhaustDelay = 5;
        m_updateTimer.start(exhaustDelay, Qt::PreciseTimer, this);
        m_isUpdatePending = true;
    }
}

void UIRenderer::logError(const QString &error)
{
    QFile errorLog("error.log");
    errorLog.open(QFile::WriteOnly | QFile::Append);
    QTextStream errorOutput(&errorLog);
    errorOutput << QDateTime::currentDateTime().toString() << "\t" << error << "\r\n";
    errorLog.close();
}

void UIRenderer::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateTimer.timerId()) {
        m_isUpdatePending = false;
        m_updateTimer.stop();
        render();
    }
}
