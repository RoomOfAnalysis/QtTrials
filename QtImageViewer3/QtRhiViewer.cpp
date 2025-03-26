#include "QtRhiViewer.h"

#include <rhi/qrhi.h>
#include <QFile>
#include <QMenu>
#include <QWheelEvent>

static float VERTICES[] = {
    // positions             // texture coords
    1.0F,  1.0F,  0.0F, 1.0F, 1.0F, // top right
    1.0F,  -1.0F, 0.0F, 1.0F, 0.0F, // bottom right
    -1.0F, -1.0F, 0.0F, 0.0F, 0.0F, // bottom left
    -1.0F, 1.0F,  0.0F, 0.0F, 1.0F  // top left
};
static unsigned int INDICES[] = {0, 1, 3, 1, 2, 3};

static QShader getShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

class QtRhiViewer::QtRhiViewerPrivate
{
public:
    explicit QtRhiViewerPrivate(QtRhiViewer* q): q_ptr(q) { createPopMenu(); }
    ~QtRhiViewerPrivate() = default;

    void initScene()
    {
        scene.vbuf.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VERTICES)));
        scene.vbuf->create();

        scene.resourceUpdates = rhi->nextResourceUpdateBatch();
        scene.resourceUpdates->uploadStaticBuffer(scene.vbuf.get(), VERTICES);

        scene.ibuf.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer, sizeof(INDICES)));
        scene.ibuf->create();
        scene.resourceUpdates->uploadStaticBuffer(scene.ibuf.get(), INDICES);

        scene.ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 16 * sizeof(float)));
        scene.ubuf->create();

        scene.sampler.reset(rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                            QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
        scene.sampler->create();

        scene.ps.reset(rhi->newGraphicsPipeline());
        scene.ps->setDepthTest(true);
        scene.ps->setDepthWrite(true);
        // scene.ps->setCullMode(QRhiGraphicsPipeline::Back);
        scene.ps->setShaderStages({{QRhiShaderStage::Vertex, getShader(QLatin1String("://shader/vulkan.vert.qsb"))},
                                   {QRhiShaderStage::Fragment, getShader(QLatin1String("://shader/vulkan.frag.qsb"))}});

        QRhiVertexInputLayout inputLayout;
        inputLayout.setBindings({{5 * sizeof(float)}});
        inputLayout.setAttributes(
            {{0, 0, QRhiVertexInputAttribute::Float3, 0}, {0, 1, QRhiVertexInputAttribute::Float2, 3 * sizeof(float)}});

        scene.ps->setSampleCount(sampleCount);
        scene.ps->setVertexInputLayout(inputLayout);
        scene.ps->setRenderPassDescriptor(q_ptr->renderTarget()->renderPassDescriptor());

        transform = rhi->clipSpaceCorrMatrix();
    }

    void initTexture()
    {
        scene.cubeTex.reset(rhi->newTexture(QRhiTexture::RGBA8, image.size()));
        scene.cubeTex->create();

        scene.srb.reset(rhi->newShaderResourceBindings());
        scene.srb->setBindings(
            {QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, scene.ubuf.get()),
             QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, scene.cubeTex.get(),
                                                       scene.sampler.get())});
        scene.srb->create();

        scene.ps->destroy();
        scene.ps->setShaderResourceBindings(scene.srb.get());
        scene.ps->create();

        updateTexture();
    }

    void updateTexture()
    {
        if (!scene.resourceUpdates) scene.resourceUpdates = rhi->nextResourceUpdateBatch();
        scene.resourceUpdates->uploadTexture(scene.cubeTex.get(), image);
    }

    void updateTransform()
    {
        if (!scene.resourceUpdates) scene.resourceUpdates = rhi->nextResourceUpdateBatch();
        scene.resourceUpdates->updateDynamicBuffer(scene.ubuf.get(), 0, 16 * sizeof(float), transform.constData());
    }

    void createPopMenu()
    {
        menu = new QMenu(q_ptr);
        menu->addAction(tr("Reset to original size"), q_ptr, &QtRhiViewer::resetToOriginalSize);
        menu->addAction(tr("Fit to screen"), q_ptr, &QtRhiViewer::fitToScreen);
        menu->addAction(tr("Rotate 90"), q_ptr, &QtRhiViewer::rotateNinetieth);
        menu->addAction(tr("Anti rotate 90"), q_ptr, &QtRhiViewer::anti_rotateNinetieth);
    }

    void emitScaleFactor()
    {
        updateTransform();
        auto factor = transform.toTransform().m11() * windowSize.width() / image.width();
        emit q_ptr->scaleFactorChanged(factor);
    }

public:
    QtRhiViewer* q_ptr;

    QRhi* rhi = nullptr;
    int sampleCount = 1;
    QSize pixelSize;

    struct
    {
        QRhiResourceUpdateBatch* resourceUpdates = nullptr;
        std::unique_ptr<QRhiBuffer> vbuf;
        std::unique_ptr<QRhiBuffer> ibuf;
        std::unique_ptr<QRhiBuffer> ubuf;
        std::unique_ptr<QRhiShaderResourceBindings> srb;
        std::unique_ptr<QRhiGraphicsPipeline> ps;
        std::unique_ptr<QRhiSampler> sampler;
        std::unique_ptr<QRhiTexture> cubeTex;
        QMatrix4x4 mvp;
    } scene;

    QImage image;
    QColor backgroundColor = Qt::black;
    QMatrix4x4 transform;
    qreal const scaleFactor = 1.1;
    QSize windowSize;

    QMenu* menu;
};

QtRhiViewer::QtRhiViewer(QWidget* parent): QRhiWidget(parent), d_ptr(new QtRhiViewerPrivate(this))
{
    setApi(QRhiWidget::Api::Direct3D12);
}

QtRhiViewer::~QtRhiViewer() = default;

void QtRhiViewer::setImage(QImage image)
{
    if (image.isNull())
    {
        image = QImage(800, 600, QImage::Format_RGBA8888_Premultiplied);
        image.fill(Qt::red);
    }
    d_ptr->image = image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    d_ptr->initTexture();

    auto size = d_ptr->image.size();
    if (size.width() > width() || size.height() > height())
        fitToScreen();
    else
        resetToOriginalSize();

    emit imageChanged(image);
    emit imageSizeChanged(size);
}

void QtRhiViewer::resetToOriginalSize()
{
    if (d_ptr->image.isNull()) return;

    auto size = d_ptr->image.size();
    auto factor_w = static_cast<qreal>(size.width()) / width();
    auto factor_h = static_cast<qreal>(size.height()) / height();
    d_ptr->transform = d_ptr->rhi->clipSpaceCorrMatrix();
    d_ptr->transform.scale(factor_w, factor_h, 1.0);
    d_ptr->emitScaleFactor();

    QMetaObject::invokeMethod(this, [this] { update(); }, Qt::QueuedConnection);
}

void QtRhiViewer::fitToScreen()
{
    if (d_ptr->image.isNull()) return;

    auto size = d_ptr->image.size();
    auto factor_w = static_cast<qreal>(width()) / size.width();
    auto factor_h = static_cast<qreal>(height()) / size.height();
    auto factor = qMin(factor_w, factor_h);
    d_ptr->transform = d_ptr->rhi->clipSpaceCorrMatrix();
    d_ptr->transform.scale(factor / factor_w, factor / factor_h, 1.0);
    d_ptr->emitScaleFactor();

    QMetaObject::invokeMethod(this, [this] { update(); }, Qt::QueuedConnection);
}

void QtRhiViewer::rotateNinetieth()
{
    d_ptr->transform.rotate(90, 0, 0, 1);
    d_ptr->updateTransform();

    QMetaObject::invokeMethod(this, [this] { update(); }, Qt::QueuedConnection);
}

void QtRhiViewer::anti_rotateNinetieth()
{
    d_ptr->transform.rotate(-90, 0, 0, 1);
    d_ptr->updateTransform();

    QMetaObject::invokeMethod(this, [this] { update(); }, Qt::QueuedConnection);
}

void QtRhiViewer::initialize(QRhiCommandBuffer* cb)
{
    if (d_ptr->rhi != rhi())
    {
        d_ptr->rhi = rhi();
        d_ptr->scene = {};
        qInfo() << "RHI backend changed to" << d_ptr->rhi->backendName();
        emit rhiChanged(QString::fromUtf8(d_ptr->rhi->backendName()));
    }
    if (d_ptr->pixelSize != renderTarget()->pixelSize()) d_ptr->pixelSize = renderTarget()->pixelSize();
    if (d_ptr->sampleCount != renderTarget()->sampleCount())
    {
        d_ptr->sampleCount = renderTarget()->sampleCount();
        d_ptr->scene = {};
    }
    if (!d_ptr->scene.vbuf)
    {
        d_ptr->initScene();
        setImage({});
    }

    d_ptr->windowSize = size();
    d_ptr->emitScaleFactor();
}

void QtRhiViewer::render(QRhiCommandBuffer* cb)
{
    auto* resourceUpdates = d_ptr->scene.resourceUpdates;
    if (resourceUpdates) d_ptr->scene.resourceUpdates = nullptr;

    cb->beginPass(renderTarget(), d_ptr->backgroundColor, {1.0f, 0}, resourceUpdates);

    cb->setGraphicsPipeline(d_ptr->scene.ps.get());
    cb->setViewport(QRhiViewport(0, 0, width(), height()));
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput vbufBinding(d_ptr->scene.vbuf.get(), 0);
    cb->setVertexInput(0, 1, &vbufBinding, d_ptr->scene.ibuf.get(), 0, QRhiCommandBuffer::IndexUInt32);
    cb->drawIndexed(sizeof(INDICES));

    cb->endPass();
}

void QtRhiViewer::releaseResources()
{
    d_ptr->scene = {};
}

void QtRhiViewer::wheelEvent(QWheelEvent* event)
{
    QRhiWidget::wheelEvent(event);
    if (d_ptr->image.isNull()) return;

    qreal factor = qPow(d_ptr->scaleFactor, event->angleDelta().y() / 240.0);
    d_ptr->transform.scale(factor, factor, 1.0);
    d_ptr->emitScaleFactor();

    QMetaObject::invokeMethod(this, [this] { update(); }, Qt::QueuedConnection);
}

void QtRhiViewer::mouseDoubleClickEvent(QMouseEvent* event)
{
    QRhiWidget::mouseDoubleClickEvent(event);
    fitToScreen();
}

void QtRhiViewer::contextMenuEvent(QContextMenuEvent* event)
{
    d_ptr->menu->exec(event->globalPos());
}

void QtRhiViewer::resizeEvent(QResizeEvent* event)
{
    QRhiWidget::resizeEvent(event);

    // TODO: reisze changes the image aspect ratio due to viewport rect changes, need recalculate the corresponding factor
    fitToScreen();
}
