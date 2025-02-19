#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QString>
#include <QVector4D>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class QtGLWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    QtGLWidget(QWidget* parent = nullptr);
    ~QtGLWidget();

    void loadMesh(QString path);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setNearPlane(float n);
    void setFarPlane(float f);
    void setViewPosX(float x);
    void setViewPosY(float y);
    void setViewPosZ(float z);
    void setViewRotX(float x);
    void setViewRotY(float y);
    void setViewRotZ(float z);
    void rotateView(float angle_x, float angle_y, float angle_z);
    void translateView(float x, float y, float z);
    void resetView();

    void setClearColor(QColor const& color);

    void screenshot(QString path);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    //void mouseReleaseEvent(QMouseEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;

private:
    void initShaders();
    bool initShader(QOpenGLShaderProgram* program, QString url_vs, QString url_fs);
    void renderMesh(QMatrix4x4 mvp);

private:
    QColor clear_color{Qt::black};
    bool with_normal{false};
    bool with_color{false};
    int vertices_num{};

    // viewport
    GLfloat pos_x{};
    GLfloat pos_y{};
    GLfloat pos_z{-10.f};
    GLfloat rot_x{};
    GLfloat rot_y{};
    GLfloat rot_z{};
    GLdouble aspect_ratio{1.};
    GLfloat near_plane{0.01f};
    GLfloat far_plane{1000.f};

    // mouse
    QPoint last_pos;

    // shader
    QOpenGLShaderProgram* mesh_sp = nullptr;
    QOpenGLBuffer mesh_vbo;

    // light and material
    QVector4D light0_position{1.f, 1.f, 1.f, 0.f};
    QVector4D light0_diffuse{0.8f, 0.8f, 0.8f, 1.f};
    QVector4D light0_ambient{0.3f, 0.3f, 0.3f, 1.f};
    QVector4D light0_specular{1.f, 1.f, 1.f, 1.f};
    QVector4D mat_diffuse{1.f, 1.f, 1.f, 1.f};
    QVector4D mat_ambient{1.f, 1.f, 1.f, 1.f};
    QVector4D mat_specular{1.f, 1.f, 1.f, 1.f};
    float high_shininess{1000.f};
};