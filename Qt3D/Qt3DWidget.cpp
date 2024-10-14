#include "Qt3DWidget.h"

#include <Qt3DCore/QAspectEngine>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QFrameGraphNode>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QMultiSampleAntiAliasing>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetOutput>
#include <Qt3DRender/QTexture>
#include <Qt3DInput/QInputAspect>
#include <Qt3DInput/QInputSettings>
#include <Qt3DLogic/QLogicAspect>
#include <Qt3DLogic/QFrameAction>
#include <Qt3DExtras/QForwardRenderer>

#include <QOffscreenSurface>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGlFunctions>

static const char* VERTEX_SHADER = "#version 150\n"
                                   "in highp vec3 vertex;\n"
                                   "in mediump vec2 texCoord;\n"
                                   "out mediump vec2 texc;\n"
                                   "uniform mediump mat4 matrix;\n"
                                   "void main(void)\n"
                                   "{\n"
                                   "        gl_Position = matrix * vec4(vertex, 1.0);\n"
                                   "        texc = texCoord;\n"
                                   "}\n";

static const char* FRAG_SHADER = "#version 150\n"
                                 "uniform sampler2DMS texture;\n"
                                 "in mediump vec2 texc;\n"
                                 "uniform int samples;\n"
                                 "out vec4 fragColor;"
                                 "void main(void)\n"
                                 "{\n"
                                 "   ivec2 tc = ivec2(floor(textureSize(texture) * texc));\n"
                                 "   vec4 color = vec4(0.0);\n"
                                 "   for(int i = 0; i < samples; i++) {\n"
                                 "       color += texelFetch(texture, tc, i);\n"
                                 "   }\n"
                                 "   fragColor = color / float(samples);\n"
                                 "}\n";

Qt3DWidget::Qt3DWidget(QWidget* parent)
    : QOpenGLWidget(parent), m_aspect_engine(new Qt3DCore::QAspectEngine),
      m_render_aspect(new Qt3DRender::QRenderAspect(Qt3DRender::QRenderAspect::Automatic)),
      m_input_aspect(new Qt3DInput::QInputAspect), m_logic_aspect(new Qt3DLogic::QLogicAspect),
      m_render_settings(new Qt3DRender::QRenderSettings), m_forward_renderer(new Qt3DExtras::QForwardRenderer),
      m_camera(new Qt3DRender::QCamera), m_active_frame_graph(m_forward_renderer),
      m_input_settings(new Qt3DInput::QInputSettings), m_frame_action(new Qt3DLogic::QFrameAction),
      m_root_entity(new Qt3DCore::QEntity), m_render_state_set(new Qt3DRender::QRenderStateSet),
      m_depth_test(new Qt3DRender::QDepthTest), m_multisample_antialiasing(new Qt3DRender::QMultiSampleAntiAliasing),
      m_render_surface_selector(new Qt3DRender::QRenderSurfaceSelector),
      m_render_target_selector(new Qt3DRender::QRenderTargetSelector), m_render_target(new Qt3DRender::QRenderTarget),
      m_color_output(new Qt3DRender::QRenderTargetOutput), m_color_texture(new Qt3DRender::QTexture2DMultisample),
      m_depth_output(new Qt3DRender::QRenderTargetOutput), m_depth_texture(new Qt3DRender::QTexture2DMultisample),
      m_offscreen_surface(new QOffscreenSurface), m_vao(new QOpenGLVertexArrayObject(this)), m_vbo(new QOpenGLBuffer)
{
    // 3d
    setMouseTracking(true);

    int samples = QSurfaceFormat::defaultFormat().samples();

    m_offscreen_surface->setFormat(QSurfaceFormat::defaultFormat());
    m_offscreen_surface->create();

    m_aspect_engine->registerAspect(m_render_aspect);
    m_aspect_engine->registerAspect(m_input_aspect);
    m_aspect_engine->registerAspect(m_logic_aspect);

    // setup color
    m_color_output->setAttachmentPoint(Qt3DRender::QRenderTargetOutput::Color0);

    // color texture to render into
    m_color_texture->setSize(width(), height());
    m_color_texture->setFormat(Qt3DRender::QAbstractTexture::RGB8_UNorm);
    m_color_texture->setMinificationFilter(Qt3DRender::QAbstractTexture::Linear);
    m_color_texture->setMagnificationFilter(Qt3DRender::QAbstractTexture::Linear);
    m_color_texture->setSamples(samples);

    m_color_output->setTexture(m_color_texture);
    m_render_target->addOutput(m_color_output);

    // setup depth
    m_depth_output->setAttachmentPoint(Qt3DRender::QRenderTargetOutput::Depth);

    // depth texture
    m_depth_texture->setSize(width(), height());
    m_depth_texture->setFormat(Qt3DRender::QAbstractTexture::DepthFormat);
    m_depth_texture->setMinificationFilter(Qt3DRender::QAbstractTexture::Linear);
    m_depth_texture->setMagnificationFilter(Qt3DRender::QAbstractTexture::Linear);
    m_depth_texture->setComparisonFunction(Qt3DRender::QAbstractTexture::CompareLessEqual);
    m_depth_texture->setComparisonMode(Qt3DRender::QAbstractTexture::CompareRefToTexture);
    m_depth_texture->setSamples(samples);

    m_depth_output->setTexture(m_depth_texture);
    m_render_target->addOutput(m_depth_output);

    m_render_state_set->addRenderState(m_multisample_antialiasing);
    m_render_state_set->addRenderState(m_depth_test);
    m_depth_test->setDepthFunction(Qt3DRender::QDepthTest::LessOrEqual);

    m_render_target_selector->setParent(m_render_state_set);
    m_render_target_selector->setTarget(m_render_target);

    m_render_surface_selector->setSurface(m_offscreen_surface);
    m_render_surface_selector->setParent(m_render_target_selector);

    m_camera->setParent(m_render_surface_selector);

    m_forward_renderer->setCamera(m_camera);
    m_forward_renderer->setSurface(m_offscreen_surface);
    m_forward_renderer->setParent(m_render_surface_selector);
    m_forward_renderer->setClearColor("white");

    m_render_settings->setActiveFrameGraph(m_render_state_set);
    m_input_settings->setEventSource(this);

    m_active_frame_graph = m_forward_renderer;

    m_root_entity->addComponent(m_render_settings);
    m_root_entity->addComponent(m_input_settings);
    m_root_entity->addComponent(m_frame_action);
    connect(m_frame_action, &Qt3DLogic::QFrameAction::triggered, [this]() {
        emit frameDrawn();
        update();
    });
    m_aspect_engine->setRootEntity(Qt3DCore::QEntityPtr(m_root_entity));
}

Qt3DWidget::~Qt3DWidget()
{
    m_aspect_engine->setRootEntity(Qt3DCore::QEntityPtr());
    delete m_aspect_engine;
}

void Qt3DWidget::paintGL()
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(1.0, 1.0, 1.0, 1.0);
    f->glDisable(GL_BLEND);
    f->glEnable(GL_MULTISAMPLE);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader_program->bind();
    {
        QMatrix4x4 m;
        m.ortho(0, 1, 1, 0, 1.0f, 3.0f);
        m.translate(0.0f, 0.0f, -2.0f);

        QOpenGLVertexArrayObject::Binder vao_binder(m_vao);

        m_shader_program->setUniformValue("matrix", m);
        f->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_color_texture->handle().toUInt());
        f->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    m_shader_program->release();
}

void Qt3DWidget::initializeGL()
{
    // gl
    static QVector<GLfloat> const vertex_data = {1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0};

    m_vao->create();
    QOpenGLVertexArrayObject::Binder vao_binder(m_vao);

    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(vertex_data.constData(), vertex_data.count() * sizeof(GLfloat));

    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                             reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    m_vbo->release();

    m_shader_program.reset(new QOpenGLShaderProgram);
    m_shader_program->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER);
    m_shader_program->addShaderFromSourceCode(QOpenGLShader::Fragment, FRAG_SHADER);
    m_shader_program->bindAttributeLocation("vertex", 0);
    m_shader_program->bindAttributeLocation("texCoord", 0);
    m_shader_program->link();
    m_shader_program->bind();
    m_shader_program->setUniformValue("texture", 0);
    m_shader_program->setUniformValue("samples", QSurfaceFormat::defaultFormat().samples());
    m_shader_program->release();
}

void Qt3DWidget::resizeGL(int w, int h)
{
    m_camera->setAspectRatio(w / (float)h);
    m_color_texture->setSize(w, h);
    m_depth_texture->setSize(w, h);
    m_render_surface_selector->setExternalRenderTargetSize(QSize(w, h));
}

void Qt3DWidget::setRootEntity(Qt3DCore::QEntity* root)
{
    if (m_user_root_entity != root)
    {
        if (m_user_root_entity != nullptr) m_user_root_entity->setParent(static_cast<Qt3DCore::QNode*>(nullptr));
        if (root != nullptr) root->setParent(m_root_entity);
        m_user_root_entity = root;
    }
}

void Qt3DWidget::setActiveFrameGraph(Qt3DRender::QFrameGraphNode* activeFrameGraph)
{
    m_active_frame_graph->setParent(static_cast<Qt3DCore::QNode*>(nullptr));
    m_active_frame_graph = activeFrameGraph;
    activeFrameGraph->setParent(m_render_surface_selector);
}

Qt3DRender::QCamera* Qt3DWidget::camera() const
{
    return m_camera;
}
