#include "uirenderer.h"
#include <QtQuick/QQuickRenderControl>
#include <QtQuick/QQuickWindow>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickItem>
#include <QtQuick/private/qquickwindow_p.h>
#include <QtQuick/private/qsgsoftwarerenderer_p.h>

#include <QFile>
#include <QTextStream>

UIRenderer::UIRenderer(QObject *parent)
    : QObject(parent)
    , m_unityTime(0.0f)
    , m_textureSize(512, 512)
    , m_textureHandle(nullptr)
    , m_device(nullptr)
    , m_framebuffer(m_textureSize, QImage::Format_ARGB32_Premultiplied)
    , m_qmlComponent(nullptr)
    , m_rootItem(nullptr)
    , m_isReady(false)
    , m_isUpdatePending(false)
{
    m_framebuffer.fill(Qt::transparent);
    m_renderControl = new QQuickRenderControl(this);
    m_offscreenWindow = new QQuickWindow(m_renderControl);
    m_offscreenWindow->setGeometry(0, 0, m_textureSize.width(), m_textureSize.height());
    m_qmlEngine = new QQmlEngine;
    if (!m_qmlEngine->incubationController())
        m_qmlEngine->setIncubationController(m_offscreenWindow->incubationController());

    QObject::connect(m_renderControl, SIGNAL(renderRequested()), this, SLOT(triggerUpdate()));
    QObject::connect(m_renderControl, SIGNAL(sceneChanged()), this, SLOT(triggerUpdate()));

    // DEBUG
    m_isReady = loadQML("qrc:/qml/calqlatr/calqlatr.qml");
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
        m_framebuffer = QImage(m_textureSize, QImage::Format_ARGB32_Premultiplied);
        m_framebuffer.fill(Qt::transparent);
        if (m_rootItem) {
            m_rootItem->setWidth(m_textureSize.width());
            m_rootItem->setHeight(m_textureSize.height());
        }
        m_offscreenWindow->setGeometry(0, 0, m_textureSize.width(), m_textureSize.height());
    }
}

ID3D11Texture2D *UIRenderer::textureHandle() const
{
    return m_textureHandle;
}

void UIRenderer::setTextureHandle(ID3D11Texture2D *textureHandle)
{
    m_textureHandle = textureHandle;
}

ID3D11Device *UIRenderer::device() const
{
    return m_device;
}

void UIRenderer::setDevice(ID3D11Device *device)
{
    m_device = device;
}

void UIRenderer::render()
{
    m_mutex.lock();
    if (m_isReady) {
        m_renderControl->polishItems();
        m_renderControl->sync();

        QQuickWindowPrivate *cd = QQuickWindowPrivate::get(m_offscreenWindow);
        auto softwareRenderer = static_cast<QSGSoftwareRenderer*>(cd->renderer);
        softwareRenderer->setCurrentPaintDevice(&m_framebuffer);
        m_renderControl->render();
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
    QFile errorLog("error.log");
    errorLog.open(QFile::WriteOnly | QFile::Truncate);
    QTextStream errorOutput(&errorLog);
    if (m_qmlComponent != nullptr)
        delete m_qmlComponent;
    m_qmlComponent = new QQmlComponent(m_qmlEngine, QUrl(qmlFile), QQmlComponent::PreferSynchronous);

    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            errorOutput << error.url().toString() << error.line() << error.toString();
        Q_ASSERT(false);
        errorLog.close();
        return false;
    }

    QObject *rootObject = m_qmlComponent->create();
    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            errorOutput << error.url().toString() << error.line() << error.toString();
        Q_ASSERT(false);
        errorLog.close();
        return false;
    }

    m_rootItem = qobject_cast<QQuickItem *>(rootObject);
    if (!m_rootItem) {
        errorOutput << "run: Not a QQuickItem";
        delete rootObject;
        Q_ASSERT(false);
        errorLog.close();
        return false;
    }

    // The root item is ready. Associate it with the window.
    m_rootItem->setParentItem(m_offscreenWindow->contentItem());

    m_rootItem->setWidth(m_textureSize.width());
    m_rootItem->setHeight(m_textureSize.height());

    m_offscreenWindow->setGeometry(0, 0, m_textureSize.width(), m_textureSize.height());

    errorLog.close();
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

void UIRenderer::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateTimer.timerId()) {
        m_isUpdatePending = false;
        m_updateTimer.stop();
        render();
    }
}
