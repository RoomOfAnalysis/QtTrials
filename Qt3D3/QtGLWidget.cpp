#include "QtGLWidget.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

QtGLWidget::QtGLWidget(QWidget* parent): QOpenGLWidget(parent) {}

QtGLWidget::~QtGLWidget()
{
    mesh_vbo.destroy();
    delete mesh_sp;
}

void QtGLWidget::loadMesh(QString path)
{
    makeCurrent();
    Assimp::Importer importer;
    auto* scene = importer.ReadFile(path.toStdString(), aiProcess_Triangulate | aiProcess_FixInfacingNormals);
    auto* mesh = scene->mMeshes[0];
    mesh_vbo.create();
    mesh_vbo.bind();
    vertices_num = mesh->mNumVertices;
    if (!mesh->HasNormals())
        mesh_vbo.allocate(mesh->mVertices, mesh->mNumVertices * sizeof(aiVector3D));
    else
    {
        QList<QVector3D> vnl;
        vnl.reserve(mesh->mNumVertices * 2);
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            vnl.append(QVector3D(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
            vnl.append(QVector3D(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z).normalized());
        }
        mesh_vbo.allocate(vnl.constData(), (int)(vnl.size() * sizeof(QVector3D)));
        with_normal = true;
    }
    // TODO: use indices + glDrawElements
    with_color = true;
    initShaders();
    // TODO: texture, tangent
}

QSize QtGLWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize QtGLWidget::sizeHint() const
{
    return QSize(600, 600);
}

void QtGLWidget::setNearPlane(float n)
{
    near_plane = n;
}

void QtGLWidget::setFarPlane(float f)
{
    far_plane = f;
}

void QtGLWidget::setViewPosX(float x)
{
    pos_x = x;
}

void QtGLWidget::setViewPosY(float y)
{
    pos_y = y;
}

void QtGLWidget::setViewPosZ(float z)
{
    pos_z = z;
}

void QtGLWidget::setViewRotX(float x)
{
    rot_x = x;
}

void QtGLWidget::setViewRotY(float y)
{
    rot_y = y;
}

void QtGLWidget::setViewRotZ(float z)
{
    rot_z = z;
}

void QtGLWidget::rotateView(float angle_x, float angle_y, float angle_z)
{
    rot_x += angle_x;
    rot_y += angle_y;
    rot_z += angle_z;
    update();
}

void QtGLWidget::translateView(float x, float y, float z)
{
    pos_x += x;
    pos_y += y;
    pos_z += z;
    update();
}

void QtGLWidget::resetView()
{
    pos_x = 0.f;
    pos_y = 0.f;
    pos_z = -10.f;
    rot_x = 0.f;
    rot_y = 0.f;
    rot_z = 0.f;
    update();
}

void QtGLWidget::setClearColor(QColor const& color)
{
    clear_color = color;
    update();
}

void QtGLWidget::screenshot(QString path)
{
    grabFramebuffer().save(path);
}

void QtGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void QtGLWidget::paintGL()
{
    glClearColor(clear_color.redF(), clear_color.greenF(), clear_color.blueF(), clear_color.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 m;

    m.perspective(45.0f, aspect_ratio, near_plane, far_plane);
    m.translate(pos_x, pos_y, pos_z);
    m.rotate(rot_x, 1.f, 0.f, 0.f);
    m.rotate(rot_y, 0.f, 1.f, 0.f);
    m.rotate(rot_z, 0.f, 0.f, 1.f);

    renderMesh(m);
}

void QtGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect_ratio = (GLdouble)width / (GLdouble)height;
}

void QtGLWidget::mousePressEvent(QMouseEvent* ev)
{
    last_pos = ev->pos();
}

void QtGLWidget::mouseMoveEvent(QMouseEvent* ev)
{
    auto pos = ev->position();
    float dx = pos.x() - last_pos.x();
    float dy = pos.y() - last_pos.y();

    if (ev->buttons() & Qt::LeftButton) rotateView(dy, dx, 0);
    if (ev->buttons() & Qt::RightButton)
    {
        float ratio = fabs(pos_z) / 720.f;
        translateView(dx * ratio, dy * ratio, 0.f);
    }
    last_pos = ev->pos();
}

void QtGLWidget::wheelEvent(QWheelEvent* ev)
{
    int delta = ev->angleDelta().y();
    if (delta < 0)
        pos_z *= 1.1f;
    else if (delta > 0 && pos_z < 5)
        pos_z *= 0.9f;
    update();
    ev->accept();
}

void QtGLWidget::initShaders()
{
    QString url_mesh_vs(":/shaders/mesh.vs");
    QString url_mesh_fs(":/shaders/mesh.fs");

    if (with_normal && !with_color)
    {
        url_mesh_vs = QString(":/shaders/mesh_n.vs");
        url_mesh_fs = QString(":/shaders/mesh_n.fs");
    }

    if (with_normal && with_color)
    {
        url_mesh_vs = QString(":/shaders/mesh_nc.vs");
        url_mesh_fs = QString(":/shaders/mesh_nc.fs");
    }

    mesh_sp = new QOpenGLShaderProgram;
    initShader(mesh_sp, url_mesh_vs, url_mesh_fs);
}

bool QtGLWidget::initShader(QOpenGLShaderProgram* program, QString url_vs, QString url_fs)
{
    if (!program) return false;

    QOpenGLShader* vs = new QOpenGLShader(QOpenGLShader::Vertex, this);
    QOpenGLShader* fs = new QOpenGLShader(QOpenGLShader::Fragment, this);
    if (!vs->compileSourceFile(url_vs))
    {
        qDebug() << "Error: loading vs from " << url_vs;
        return false;
    }
    program->addShader(vs);
    if (!fs->compileSourceFile(url_fs))
    {
        qDebug() << "Error: loading fs from " << url_fs;
        return false;
    }
    program->addShader(fs);

    program->bindAttributeLocation("vertex", 0);
    if (with_normal) program->bindAttributeLocation("normal", 1);
    if (with_color) program->bindAttributeLocation("color", 2);

    return program->link();
}

void QtGLWidget::renderMesh(QMatrix4x4 mvp)
{
    if (vertices_num < 3) return;

    mesh_vbo.bind();
    mesh_sp->bind();
    mesh_sp->setUniformValue("matrix", mvp);
    if (with_color) mesh_sp->setUniformValue("color", QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    // vertices
    mesh_sp->enableAttributeArray(0);
    if (!with_normal)
    {
        mesh_sp->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
        mesh_sp->setUniformValue("point_size", pos_z + 15);
        glDrawArrays(GL_POINTS, 0, vertices_num);
        mesh_sp->release();
        mesh_vbo.release();
        return;
    }
    mesh_sp->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    // normals
    mesh_sp->enableAttributeArray(1);
    mesh_sp->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));
    // set light and material uniform value
    mesh_sp->setUniformValue("light0_position", light0_position);
    mesh_sp->setUniformValue("light0_ambient", light0_ambient);
    mesh_sp->setUniformValue("light0_diffuse", light0_diffuse);
    mesh_sp->setUniformValue("light0_specular", light0_specular);
    mesh_sp->setUniformValue("mat_diffuse", mat_diffuse);
    mesh_sp->setUniformValue("mat_ambient", mat_ambient);
    mesh_sp->setUniformValue("mat_specular", mat_specular);
    mesh_sp->setUniformValue("high_shininess", high_shininess);
    glDrawArrays(GL_TRIANGLES, 0, vertices_num);
    mesh_sp->release();
    mesh_vbo.release();
}
