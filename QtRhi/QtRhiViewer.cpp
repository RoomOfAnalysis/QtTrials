#include "QtRhiViewer.h"
#include "QtMesh.h"

#include <rhi/qrhi.h>
#include <QFile>

static QShader getShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

QtRhiViewer::QtRhiViewer(QWidget* parent): QRhiWidget(parent) {}

QtRhiViewer::~QtRhiViewer() = default;

void QtRhiViewer::loadMesh(QString path)
{
    m_mesh = QtMesh::Load(path);
}

void QtRhiViewer::initialize(QRhiCommandBuffer* cb)
{
    assert(m_mesh);

    if (m_rhi != rhi())
    {
        m_rhi = rhi();
        m_scene = {};
        qInfo() << "RHI backend changed to" << m_rhi->backendName();
    }
    if (!m_scene.vbuf)
    {
        rebuildScene();
        cb->resourceUpdate(m_scene.ru);
    }
}

void QtRhiViewer::render(QRhiCommandBuffer* cb)
{
    static int rotation = 0;
    rotation = (rotation + 1) % 360;
    const QSize outputSize = colorTexture()->pixelSize();

    auto* ru = m_rhi->nextResourceUpdateBatch();
    m_scene.mvp.rotate(rotation, 0, 1, 0);
    ru->updateDynamicBuffer(m_scene.ubuf.get(), 0, 16 * sizeof(float), m_scene.mvp.constData());
    cb->beginPass(renderTarget(), Qt::black, {1.0f, 0}, ru);
    cb->setGraphicsPipeline(m_scene.pipeline.get());
    cb->setViewport(QRhiViewport(0, 0, outputSize.width(), outputSize.height()));
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput vbufBinding(m_scene.vbuf.get(), 0);
    cb->setVertexInput(0, 1, &vbufBinding, m_scene.ibuf.get(), 0, QRhiCommandBuffer::IndexUInt32);
    cb->drawIndexed(static_cast<quint32>(m_mesh->m_indices.size()));

    cb->endPass();

    update();
}

void QtRhiViewer::rebuildScene()
{
    const QSize outputSize = colorTexture()->pixelSize();
    QImage img(outputSize, QImage::Format_RGBA8888_Premultiplied);
    img.fill(Qt::yellow);

    m_scene.vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                        static_cast<quint32>(sizeof(QtMesh::Vertex) * m_mesh->m_vertices.size())));
    m_scene.vbuf->create();
    m_scene.ibuf.reset(m_rhi->newBuffer(QRhiBuffer::Type::Immutable, QRhiBuffer::IndexBuffer,
                                        static_cast<quint32>(sizeof(QtMesh::Index) * m_mesh->m_indices.size())));
    m_scene.ibuf->create();

    m_scene.ubuf.reset(
        m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, static_cast<quint32>(16 * sizeof(float))));
    m_scene.ubuf->create();

    m_scene.tex.reset(m_rhi->newTexture(QRhiTexture::RGBA8, img.size()));
    m_scene.tex->create();

    m_scene.mvp = m_rhi->clipSpaceCorrMatrix();
    m_scene.mvp.perspective(45.0f, outputSize.width() / (float)outputSize.height(), 0.01f, 1000.0f);
    m_scene.mvp.translate(0, 0, -4);

    m_scene.ru = m_rhi->nextResourceUpdateBatch();
    m_scene.ru->uploadStaticBuffer(m_scene.vbuf.get(), m_mesh->m_vertices.constData());
    m_scene.ru->uploadStaticBuffer(m_scene.ibuf.get(), m_mesh->m_indices.constData());
    m_scene.ru->updateDynamicBuffer(m_scene.ubuf.get(), 0, 16 * sizeof(float), m_scene.mvp.constData());
    m_scene.ru->uploadTexture(m_scene.tex.get(), img);

    m_scene.sampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                            QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
    m_scene.sampler->create();

    m_scene.srb.reset(m_rhi->newShaderResourceBindings());
    m_scene.srb->setBindings(
        {QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_scene.ubuf.get()),
         QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, m_scene.tex.get(),
                                                   m_scene.sampler.get())});
    m_scene.srb->create();

    m_scene.pipeline.reset(m_rhi->newGraphicsPipeline());
    m_scene.pipeline->setDepthTest(true);
    m_scene.pipeline->setDepthWrite(true);
    m_scene.pipeline->setShaderStages({{QRhiShaderStage::Vertex, getShader(QLatin1String("://shader/vert.qsb"))},
                                       {QRhiShaderStage::Fragment, getShader(QLatin1String("://shader/frag.qsb"))}});
    m_scene.pipeline->setShaderResourceBindings(m_scene.srb.get());

    QRhiVertexInputLayout input_layout;
    input_layout.setBindings({sizeof(QtMesh::Vertex), QRhiVertexInputBinding::PerVertex, 1});
    input_layout.setAttributes({{0, 0, QRhiVertexInputAttribute::Float3, offsetof(QtMesh::Vertex, position)},
                                {0, 1, QRhiVertexInputAttribute::Float3, offsetof(QtMesh::Vertex, normal)},
                                {0, 2, QRhiVertexInputAttribute::Float3, offsetof(QtMesh::Vertex, tangent)},
                                {0, 3, QRhiVertexInputAttribute::Float3, offsetof(QtMesh::Vertex, bitangent)},
                                {0, 4, QRhiVertexInputAttribute::Float2, offsetof(QtMesh::Vertex, uv)}});

    m_scene.pipeline->setSampleCount(renderTarget()->sampleCount());
    m_scene.pipeline->setVertexInputLayout(input_layout);
    m_scene.pipeline->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
    m_scene.pipeline->create();
}
