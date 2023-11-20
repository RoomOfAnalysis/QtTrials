#include "QtVideoWidget.h"

#include <QCoreApplication>
#include <QOpenGLTexture>

static const GLfloat coordinate[] = {
    // x     y     z
    -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,

    0.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,  0.0f};

static const QString s_vertShader = R"(
    attribute vec3 vertexIn;
    attribute vec2 textureIn;
    varying vec2 textureOut;
    void main(void)
    {
        gl_Position = vec4(vertexIn, 1.0);
        textureOut = textureIn;
    }
)";

static QString s_fragShader = R"(
    varying vec2 textureOut;
    uniform sampler2D textureY;
    uniform sampler2D textureU;
    uniform sampler2D textureV;
    void main(void)
    {
        vec3 yuv;
        vec3 rgb;
        yuv.x = texture2D(textureY, textureOut).r;
        yuv.y = texture2D(textureU, textureOut).r - 0.5;
        yuv.z = texture2D(textureV, textureOut).r - 0.5;
        rgb = mat3(1.0, 1.0, 1.0,
                    0.0, -0.39465, 2.03211,
                    1.13983, -0.58060, 0.0) * yuv;
        gl_FragColor = vec4(rgb, 1.0);
    }
)";

QtVideoWidget::QtVideoWidget(QWidget* parent): QOpenGLWidget(parent) {}

QtVideoWidget::~QtVideoWidget()
{
    makeCurrent();
    m_vbo.destroy();
    deInitTextures();
    doneCurrent();
}

QSize QtVideoWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize QtVideoWidget::sizeHint() const
{
    return size();
}

void QtVideoWidget::setFrameSize(const QSize& frameSize)
{
    if (m_frameSize != frameSize)
    {
        m_frameSize = frameSize;
        m_needUpdate = true;
        // inittexture immediately
        repaint();
    }
}

const QSize& QtVideoWidget::frameSize()
{
    return m_frameSize;
}

void QtVideoWidget::updateTextures(const quint8* dataY, const quint8* dataU, const quint8* dataV, quint32 linesizeY,
                                   quint32 linesizeU, quint32 linesizeV)
{
    if (m_textureInited)
    {
        updateTexture(m_texture[0], 0, dataY, linesizeY);
        updateTexture(m_texture[1], 1, dataU, linesizeU);
        updateTexture(m_texture[2], 2, dataV, linesizeV);
        update();
    }
}

void QtVideoWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glDisable(GL_DEPTH_TEST);

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(coordinate, sizeof(coordinate));
    initShader();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void QtVideoWidget::paintGL()
{
    if (m_needUpdate)
    {
        deInitTextures();
        initTextures();
        m_needUpdate = false;
    }

    if (m_textureInited)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texture[1]);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_texture[2]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void QtVideoWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    repaint();
}

void QtVideoWidget::initShader()
{
    if (QCoreApplication::testAttribute(Qt::AA_UseOpenGLES))
    {
        s_fragShader.prepend(R"(
                             precision mediump int;
                             precision mediump float;
                             )");
    }
    m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, s_vertShader);
    m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, s_fragShader);
    m_shaderProgram.link();
    m_shaderProgram.bind();

    m_shaderProgram.setAttributeBuffer("vertexIn", GL_FLOAT, 0, 3, 3 * sizeof(float));
    m_shaderProgram.enableAttributeArray("vertexIn");

    m_shaderProgram.setAttributeBuffer("textureIn", GL_FLOAT, 12 * sizeof(float), 2, 2 * sizeof(float));
    m_shaderProgram.enableAttributeArray("textureIn");

    m_shaderProgram.setUniformValue("textureY", 0);
    m_shaderProgram.setUniformValue("textureU", 1);
    m_shaderProgram.setUniformValue("textureV", 2);
}

void QtVideoWidget::initTextures()
{

    glGenTextures(1, &m_texture[0]);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_frameSize.width(), m_frameSize.height(), 0, GL_RED, GL_UNSIGNED_BYTE,
                 NULL);

    glGenTextures(1, &m_texture[1]);
    glBindTexture(GL_TEXTURE_2D, m_texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_frameSize.width() / 2, m_frameSize.height() / 2, 0, GL_RED,
                 GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_texture[2]);
    glBindTexture(GL_TEXTURE_2D, m_texture[2]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_frameSize.width() / 2, m_frameSize.height() / 2, 0, GL_RED,
                 GL_UNSIGNED_BYTE, NULL);

    m_textureInited = true;
}

void QtVideoWidget::deInitTextures()
{
    if (QOpenGLFunctions::isInitialized(QOpenGLFunctions::d_ptr)) glDeleteTextures(3, m_texture);

    memset(m_texture, 0, 3);
    m_textureInited = false;
}

void QtVideoWidget::updateTexture(GLuint texture, quint32 textureType, const quint8* pixels, quint32 stride)
{
    if (!pixels) return;

    QSize size = 0 == textureType ? m_frameSize : m_frameSize / 2;

    makeCurrent();
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width(), size.height(), GL_RED, GL_UNSIGNED_BYTE, pixels);
    doneCurrent();
}