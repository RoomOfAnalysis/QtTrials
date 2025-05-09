#include <QRhiWidget>
#include <QMatrix4x4>

class QRhiBuffer;
class QRhiResourceUpdateBatch;
class QRhiShaderResourceBindings;
class QRhiGraphicsPipeline;
class QRhiSampler;

struct QtMesh;

class QtRhiViewer: public QRhiWidget
{
    Q_OBJECT
public:
    explicit QtRhiViewer(QWidget* parent = nullptr);
    ~QtRhiViewer();

    void loadMesh(QString path);
    void loadTexture(QString path);
    void textureMapToPlane(bool flag);

protected:
    void initialize(QRhiCommandBuffer* cb) override;
    void render(QRhiCommandBuffer* cb) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void buildScene();

private:
    QRhi* m_rhi = nullptr;
    struct
    {
        QRhiResourceUpdateBatch* ru = nullptr;
        std::unique_ptr<QRhiBuffer> vbuf;
        std::unique_ptr<QRhiBuffer> ibuf;
        std::unique_ptr<QRhiBuffer> ubuf;
        std::unique_ptr<QRhiShaderResourceBindings> srb;
        std::unique_ptr<QRhiGraphicsPipeline> pipeline;
        std::unique_ptr<QRhiSampler> sampler;
        std::unique_ptr<QRhiTexture> tex;
        QMatrix4x4 mvp;
    } m_scene;
    QSharedPointer<QtMesh> m_mesh;
    QImage m_tex;
    bool m_tp = false;
    std::unique_ptr<QRhiBuffer> m_bound;
    bool m_rot = true;
};