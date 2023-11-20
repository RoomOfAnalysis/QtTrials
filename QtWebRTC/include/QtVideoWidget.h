#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class QtVideoWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit QtVideoWidget(QWidget* parent = nullptr);
    virtual ~QtVideoWidget();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setFrameSize(const QSize& frameSize);
    const QSize& frameSize();
    void updateTextures(const quint8* dataY, const quint8* dataU, const quint8* dataV, quint32 linesizeY,
                        quint32 linesizeU, quint32 linesizeV);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:
    void initShader();
    void initTextures();
    void deInitTextures();
    void updateTexture(GLuint texture, quint32 textureType, const quint8* pixels, quint32 stride);

private:
    QSize m_frameSize = {-1, -1};
    bool m_needUpdate = false;
    bool m_textureInited = false;

    QOpenGLBuffer m_vbo;
    QOpenGLShaderProgram m_shaderProgram;
    GLuint m_texture[3] = {0};
};