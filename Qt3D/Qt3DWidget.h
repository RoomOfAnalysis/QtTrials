#pragma once

#include <QOpenGLWidget>
#include <QScopedPointer>
#include <QOpenGLShaderProgram>

namespace Qt3DCore
{
    class QAspectEngine;
    class QEntity;
} // namespace Qt3DCore
namespace Qt3DRender
{
    class QRenderAspect;
    class QRenderSettings;
    class QFrameGraphNode;
    class QCamera;
    class QRenderStateSet;
    class QRenderSurfaceSelector;
    class QRenderTarget;
    class QRenderTargetSelector;
    class QRenderTargetOutput;
    class QDepthTest;
    class QMultiSampleAntiAliasing;
    class QTexture2DMultisample;
} // namespace Qt3DRender
namespace Qt3DInput
{
    class QInputAspect;
    class QInputSettings;
} // namespace Qt3DInput
namespace Qt3DLogic
{
    class QLogicAspect;
    class QFrameAction;
} // namespace Qt3DLogic
namespace Qt3DExtras
{
    class QForwardRenderer;
}

class QOffscreenSurface;
class QOpenGLVertexArrayObject;
class QOpenGLTexture;
class QOpenGLBuffer;

// code from https://github.com/florianblume/qt3d-widget
// offscreen rendering to embedd Qt3D without having to use Qt3DWindow
// it renders everything offline and uses the offscreen texture to paint a quad in a QOpenGLWidget
class Qt3DWidget: public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit Qt3DWidget(QWidget* parent = nullptr);
    ~Qt3DWidget();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // https://doc.qt.io/qt-6/qt3dcore-qentity.html#details
    void setRootEntity(Qt3DCore::QEntity* root);
    // https://doc.qt.io/qt-6/qt3drender-framegraph.html
    void setActiveFrameGraph(Qt3DRender::QFrameGraphNode* frame_graph);

    Qt3DRender::QCamera* camera() const;

signals:
    void frameDrawn();

private:
    // run loop: https://doc.qt.io/qt-6/qt3dcore-qaspectengine.html#details
    Qt3DCore::QAspectEngine* m_aspect_engine;

    // aspects
    Qt3DRender::QRenderAspect* m_render_aspect; // submitting rendering commands to the GPU
    Qt3DInput::QInputAspect* m_input_aspect; // responsible for creating physical devices and handling associated jobs
    Qt3DLogic::QLogicAspect* m_logic_aspect; // responsible for handling frame synchronization jobs

    // renderer
    Qt3DRender::QRenderSettings* m_render_settings;
    Qt3DExtras::QForwardRenderer* m_forward_renderer;
    Qt3DRender::QCamera* m_camera;
    Qt3DRender::QFrameGraphNode* m_active_frame_graph;

    // input
    Qt3DInput::QInputSettings* m_input_settings;

    // emit signal at each frame
    Qt3DLogic::QFrameAction* m_frame_action; // https://doc.qt.io/qt-6/qt3dlogic-qframeaction.html#details

    // scene
    Qt3DCore::QEntity* m_root_entity;
    Qt3DCore::QEntity* m_user_root_entity = nullptr;

    // offscreen framegraph
    Qt3DRender::QRenderStateSet* m_render_state_set;
    Qt3DRender::QDepthTest* m_depth_test;
    Qt3DRender::QMultiSampleAntiAliasing* m_multisample_antialiasing;
    Qt3DRender::QRenderSurfaceSelector* m_render_surface_selector;
    Qt3DRender::QRenderTargetSelector* m_render_target_selector;
    Qt3DRender::QRenderTarget* m_render_target;
    Qt3DRender::QRenderTargetOutput* m_color_output;
    Qt3DRender::QTexture2DMultisample* m_color_texture;
    Qt3DRender::QRenderTargetOutput* m_depth_output;
    Qt3DRender::QTexture2DMultisample* m_depth_texture;

    // gl
    QOffscreenSurface* m_offscreen_surface; // https://doc.qt.io/qt-6/qoffscreensurface.html#details
    QScopedPointer<QOpenGLShaderProgram> m_shader_program;
    QOpenGLVertexArrayObject* m_vao;
    QOpenGLBuffer* m_vbo;
};