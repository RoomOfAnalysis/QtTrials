#include <QSurfaceFormat>
#include <QApplication>
#include <Qt3DExtras>
#include <QLabel>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QPaintedTextureImage>

#include "Qt3DWidget.h"
#include "trackball_camera_controller.h"

class DynamicImageTexture: public Qt3DRender::QPaintedTextureImage
{
public:
    DynamicImageTexture(Qt3DCore::QNode* parent = nullptr): QPaintedTextureImage(parent) {}
    ~DynamicImageTexture() = default;

    void setImage(QImage img)
    {
        m_img = img;
        setSize(img.size());
    }

    void paint(QPainter* painter) override
    {
        // https://doc.qt.io/qt-6/qt3drender-qpaintedtextureimage.html#paint
        // painter->setViewport(0, height(), width(), -height()); // here, no need to mirror
        painter->drawImage(0, 0, m_img);
    }

private:
    QImage m_img;
};

void addTextureCoordinatesToMesh(Qt3DRender::QMesh* mesh)
{
    auto geo = mesh->geometry();
    for (auto attr : geo->attributes())
        if (attr->name() == Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName()) return;

    Qt3DCore::QAttribute* vertice_attr = nullptr;
    for (auto attr : geo->attributes())
        if (attr->name() == Qt3DCore::QAttribute::defaultPositionAttributeName())
        {
            vertice_attr = attr;
            break;
        }
    if (!vertice_attr) return;

    //qDebug() << vertice_attr->count();

    auto min_extent = geo->minExtent();
    auto bound = geo->maxExtent() - min_extent;

    QList<QVector2D> texCoords;
    texCoords.reserve(vertice_attr->count());
    QByteArray const& data = vertice_attr->buffer()->data();
    for (uint i = 0; i < vertice_attr->count(); i++)
    {
        auto j = vertice_attr->byteOffset() + i * vertice_attr->byteStride();
        const char* rawData = &(data.constData()[j]);
        const float* value = reinterpret_cast<const float*>(rawData);
        // simple use uv = (x / bound_x, y / bound_y)
        // TODO: implement algo like [vtk](https://github.com/Kitware/VTK/blob/master/Filters/Texture/vtkTextureMapToPlane.cxx)
        texCoords.append(QVector2D(std::clamp((value[0] - min_extent[0]) / bound[0], 0.f, 1.f),
                                   std::clamp((value[1] - min_extent[1]) / bound[1], 0.f, 1.f)));
    }

    //qDebug() << texCoords;

    auto* texCoordBuffer = new Qt3DCore::QBuffer(mesh);
    QByteArray texCoordData;
    texCoordData.resize(texCoords.size() * sizeof(QVector2D));
    memcpy(texCoordData.data(), texCoords.constData(), texCoordData.size());
    texCoordBuffer->setData(texCoordData);

    auto* texCoordAttribute = new Qt3DCore::QAttribute();
    texCoordAttribute->setName(Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName());
    texCoordAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    texCoordAttribute->setVertexSize(2);
    texCoordAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    texCoordAttribute->setBuffer(texCoordBuffer);
    texCoordAttribute->setByteStride(2 * sizeof(float));
    texCoordAttribute->setCount(texCoords.size());

    geo->addAttribute(texCoordAttribute);
}

int main(int argc, char* argv[])
{
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(4);
    format.setVersion(3, 2);
    QSurfaceFormat::setDefaultFormat(format);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication app(argc, argv);
    Qt3DWidget w;
    w.setFixedSize(800, 800);

    QLabel fps(&w);
    fps.setStyleSheet("font-size:8pt;font-weight:bold;color:yellow;");
    fps.setGeometry(15, 15, 100, 100);
    QElapsedTimer timer;
    QObject::connect(&w, &Qt3DWidget::frameDrawn, &fps, [&]() {
        fps.setText(QString::number(1000 / timer.elapsed()));
        timer.start();
    });

    Qt3DRender::QGeometryRenderer* mesh = nullptr;
    if (argc > 1)
    {
        auto* m = new Qt3DRender::QMesh();
        m->setSource(QUrl::fromLocalFile(argv[1])); // .obj/.ply/.stl
        mesh = m;
    }
    else
    {
        //auto* m = new Qt3DExtras::QTorusMesh();
        //m->setRings(100);
        //m->setSlices(100);
        //m->setMinorRadius(0.5);
        //m->setRadius(1);
        auto m = new Qt3DExtras::QCuboidMesh();
        m->setXExtent(10);
        m->setYExtent(10);
        m->setZExtent(10);
        mesh = m;
    }
    if (auto p = qobject_cast<Qt3DRender::QMesh*>(mesh); p)
        QObject::connect(p, &Qt3DRender::QMesh::statusChanged, [p, &w](Qt3DRender::QMesh::Status status) {
            if (status == Qt3DRender::QMesh::Status::Ready)
                QObject::connect(
                    p->geometry(), &Qt3DCore::QGeometry::maxExtentChanged, [p, &w](const QVector3D& maxExtent) {
                        addTextureCoordinatesToMesh(p);
                        // for (auto attr : p->geometry()->attributes())
                        //     if (attr->name() == Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName())
                        //         qDebug() << attr->count();

                        // seems minExtent will be calculated before maxExtent, so here minExtent is ready
                        auto minExtent = p->geometry()->minExtent();
                        auto camera = w.camera();
                        //qDebug() << minExtent << maxExtent << camera->nearPlane() << camera->farPlane();
                        auto center = (maxExtent - minExtent) / 2;
                        auto mesh_dia = (maxExtent - minExtent).length();
                        // make sure the whole mesh are inside the camera view
                        camera->setPosition(center + 1.1f * QVector3D(0, 0, 1).normalized() * mesh_dia /
                                                         (2 * qTan(camera->fieldOfView() / 2)));
                        camera->setViewCenter(center);
                        if (auto len = (camera->position() - camera->viewCenter()).length(); len > camera->farPlane())
                        {
                            camera->setFarPlane(len * 1.5);
                            //qDebug() << camera->farPlane();
                        }
                    });
        });

    Qt3DRender::QMaterial* material = nullptr;
    if (argc > 2)
    {
        Qt3DExtras::QTextureMaterial* _material = new Qt3DExtras::QTextureMaterial;
        Qt3DRender::QTexture2D* texture = new Qt3DRender::QTexture2D(_material);
        DynamicImageTexture* t_image = new DynamicImageTexture(_material);
        t_image->setImage(QImage(argv[2]));
        texture->addTextureImage(t_image);
        _material->setTexture(texture);
        material = _material;
        qDebug() << "use texture image";
    }
    else
    {
        Qt3DExtras::QPhongMaterial* _material = new Qt3DExtras::QPhongMaterial();
        _material->setAmbient("red");
        material = _material;
    }

    Qt3DCore::QTransform* transform = new Qt3DCore::QTransform();
    Qt3DCore::QEntity* entity = new Qt3DCore::QEntity();
    entity->addComponent(mesh);
    entity->addComponent(material);
    entity->addComponent(transform);
    auto* cameraController = new TrackballCameraController(
        entity); //new Qt3DExtras::QFirstPersonCameraController(entity); // new Qt3DExtras::QOrbitCameraController(entity);
    // need set window size here, beter way may be bind to resize event
    cameraController->setWindowSize(w.size());
    cameraController->setCamera(w.camera());

    Qt3DExtras::QForwardRenderer* forwardRenderer = new Qt3DExtras::QForwardRenderer;
    forwardRenderer->setCamera(w.camera());
    forwardRenderer->setClearColor(Qt::black);
    w.setActiveFrameGraph(forwardRenderer);

    w.camera()->lens()->setPerspectiveProjection(45.0f, 1200.0f / 800.0f, 0.1f, 1000.0f);
    w.camera()->setPosition(QVector3D(0, 0, 20.0f));
    w.camera()->setUpVector(QVector3D(0, 1, 0));
    w.camera()->setViewCenter(QVector3D(0, 0, 0));

    Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(entity);
    Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    lightEntity->addComponent(light);
    Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(w.camera()->position());
    lightEntity->addComponent(lightTransform);

    //QTimer animationTimer;
    //animationTimer.setInterval(10);
    //QObject::connect(&animationTimer, &QTimer::timeout,
    //                 [transform]() { transform->setRotationX(transform->rotationX() + 1); });
    //animationTimer.start();

    w.setRootEntity(entity);

    w.show();

    return app.exec();
}