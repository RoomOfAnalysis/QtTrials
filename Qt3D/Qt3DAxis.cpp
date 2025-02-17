#include "Qt3DAxis.h"

#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QSortPolicy>
#include <Qt3DRender/QClearBuffers>

#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QConeMesh>

#include <QFontDatabase>

Qt3DAxis::Qt3DAxis(Qt3DCore::QEntity* parent, Qt3DExtras::QAbstractCameraController* camera_controller)
    : m_parent(parent), m_camera_controller(camera_controller)
{
    constructAxisScene(m_parent);
    constructLabelScene(m_parent);
    connect(m_camera_controller, &Qt3DExtras::QAbstractCameraController::cameraChanged, this,
            &Qt3DAxis::onCameraChanged);
}

Qt3DAxis::~Qt3DAxis()
{
    delete m_axis_root;
}

void Qt3DAxis::setAxisLengths(float x, float y, float z)
{
    m_axis_x_length = x;
    m_axis_y_length = y;
    m_axis_z_length = z;
}

void Qt3DAxis::setAxisRadius(float r)
{
    m_cylinder_length = r / 0.05f;
}

void Qt3DAxis::init()
{
    initAxisScene();
}

void Qt3DAxis::constructAxisScene(Qt3DCore::QEntity* parent)
{
    Qt3DRender::QViewport* axis_viewport = new Qt3DRender::QViewport;

    m_axis_scene = new Qt3DCore::QEntity;
    m_axis_scene->setParent(parent);
    m_axis_scene->setObjectName("Qt3DAxis_SceneEntity");

    m_axis_object_layer = new Qt3DRender::QLayer;
    m_axis_object_layer->setObjectName("Qt3DAxis_ObjectLayer");
    m_axis_object_layer->setParent(m_axis_scene);
    m_axis_object_layer->setRecursive(true);

    m_axis_camera = new Qt3DRender::QCamera;
    m_axis_camera->setParent(m_axis_scene);
    m_axis_camera->setProjectionType(m_camera_controller->camera()->projectionType());
    m_axis_camera->lens()->setFieldOfView(m_camera_controller->camera()->lens()->fieldOfView() * 0.5f);

    m_axis_camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
    m_axis_camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    Qt3DRender::QLayerFilter* axis_layer_filter = new Qt3DRender::QLayerFilter(axis_viewport);
    axis_layer_filter->addLayer(m_axis_object_layer);

    Qt3DRender::QCameraSelector* axis_camera_selector = new Qt3DRender::QCameraSelector;
    axis_camera_selector->setParent(axis_layer_filter);
    axis_camera_selector->setCamera(m_axis_camera);

    Qt3DRender::QSortPolicy* sort_policy = new Qt3DRender::QSortPolicy(axis_camera_selector);
    QVector<Qt3DRender::QSortPolicy::SortType> sort_types = QVector<Qt3DRender::QSortPolicy::SortType>();
    sort_types << Qt3DRender::QSortPolicy::BackToFront;
    sort_policy->setSortTypes(sort_types);

    Qt3DRender::QClearBuffers* clear_buffers = new Qt3DRender::QClearBuffers(sort_policy);
    clear_buffers->setBuffers(Qt3DRender::QClearBuffers::DepthBuffer);

    m_viewport = axis_viewport;
    m_viewport->setParent(parent);
    m_viewport->setNormalizedRect(QRectF(0.f, 0.f, 0.1f, 0.1f));
}

void Qt3DAxis::initAxisScene()
{
    if (!m_axis_root)
    {
        m_axis_root = new Qt3DCore::QEntity;
        m_axis_root->setParent(m_axis_scene);
        m_axis_root->setObjectName("Qt3DAxis_AxisRoot");
        m_axis_root->addComponent(m_axis_object_layer);

        createAxis(Axis::X);
        createAxis(Axis::Y);
        createAxis(Axis::Z);
    }

    m_text_x->setText("X");
    m_text_y->setText("Y");
    m_text_z->setText("Z");

    updateAxisLabelPosition();
}

void Qt3DAxis::createAxis(Axis axis)
{
    float const cylinder_radius = 0.05f * m_cylinder_length;
    float const cone_length = 0.3f * m_cylinder_length;
    float const cone_bottom_radius = 0.1f * m_cylinder_length;
    float m_axis_length = m_cylinder_length;

    QQuaternion rotation;
    QColor color;
    QString name;

    Qt3DExtras::QText2DEntity* text = new Qt3DExtras::QText2DEntity();
    text->setParent(m_label_scene);
    Qt3DCore::QTransform* text_transform = new Qt3DCore::QTransform();
    // FIXME: i have to add this rotation to make the text in the right direction when looking from +z
    text_transform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 180.0f));
    text->setColor(Qt::white);
    text->addComponent(text_transform);

    switch (axis)
    {
    case Axis::X:
        rotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, 1.0f), -90.0f);
        color = Qt::red;
        name = "Qt3DAxis_axisX";
        m_axis_length = m_axis_x_length;

        m_text_x = text;
        connect(m_text_x, &Qt3DExtras::QText2DEntity::textChanged, this,
                [this](const QString& text) { updateAxisLabelText(m_text_x, text); });
        m_text_transform_x = text_transform;
        m_text_coord_x = QVector3D(m_axis_length + cone_length / 2.0f, 0.0f, 0.0f);
        break;
    case Axis::Y:
        color = Qt::green;
        name = "Qt3DAxis_axisY";
        m_axis_length = m_axis_y_length;

        m_text_y = text;
        connect(m_text_y, &Qt3DExtras::QText2DEntity::textChanged, this,
                [this](const QString& text) { updateAxisLabelText(m_text_y, text); });
        m_text_transform_y = text_transform;
        m_text_coord_y = QVector3D(0.0f, m_axis_length + cone_length / 2.0f, 0.0f);
        break;
    case Axis::Z:
        rotation = QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 90.0f);
        color = Qt::blue;
        name = "Qt3DAxis_axisZ";
        m_axis_length = m_axis_z_length;

        m_text_z = text;
        connect(m_text_z, &Qt3DExtras::QText2DEntity::textChanged, this,
                [this](const QString& text) { updateAxisLabelText(m_text_z, text); });
        m_text_transform_z = text_transform;
        m_text_coord_z = QVector3D(0.0f, 0.0f, m_axis_length + cone_length / 2.0f);
        break;
    default:
        return;
    }

    // cylinder
    Qt3DCore::QEntity* cylinder = new Qt3DCore::QEntity(m_axis_root);
    cylinder->setObjectName(name);

    Qt3DExtras::QCylinderMesh* cylinder_mesh = new Qt3DExtras::QCylinderMesh;
    cylinder_mesh->setRadius(cylinder_radius);
    cylinder_mesh->setLength(m_axis_length);
    cylinder_mesh->setRings(10);
    cylinder_mesh->setSlices(4);
    cylinder->addComponent(cylinder_mesh);

    Qt3DExtras::QPhongMaterial* cylinder_material = new Qt3DExtras::QPhongMaterial(cylinder);
    cylinder_material->setAmbient(color);
    cylinder_material->setShininess(0);
    cylinder->addComponent(cylinder_material);

    Qt3DCore::QTransform* cylinder_transform = new Qt3DCore::QTransform;
    QMatrix4x4 cylinder_transform_matrix;
    cylinder_transform_matrix.rotate(rotation);
    cylinder_transform_matrix.translate(QVector3D(0.0f, m_axis_length / 2.0f, 0.0f));
    cylinder_transform->setMatrix(cylinder_transform_matrix);
    cylinder->addComponent(cylinder_transform);

    // cone
    Qt3DCore::QEntity* cone = new Qt3DCore::QEntity(m_axis_root);
    cone->setObjectName(name);
    Qt3DExtras::QConeMesh* cone_mesh = new Qt3DExtras::QConeMesh;
    cone_mesh->setLength(cone_length);
    cone_mesh->setBottomRadius(cone_bottom_radius);
    cone_mesh->setTopRadius(0.0f);
    cone_mesh->setRings(10);
    cone_mesh->setSlices(4);
    cone->addComponent(cone_mesh);

    Qt3DExtras::QPhongMaterial* cone_material = new Qt3DExtras::QPhongMaterial(cone);
    cone_material->setAmbient(color);
    cone_material->setShininess(0);
    cone->addComponent(cone_material);

    Qt3DCore::QTransform* cone_transform = new Qt3DCore::QTransform;
    QMatrix4x4 cone_transform_matrix;
    cone_transform_matrix.rotate(rotation);
    cone_transform_matrix.translate(QVector3D(0.0f, m_axis_length, 0.0f));
    cone_transform->setMatrix(cone_transform_matrix);
    cone->addComponent(cone_transform);
}

void Qt3DAxis::onCameraChanged()
{
    Qt3DRender::QCamera* parent_camera = m_camera_controller->camera();

    if (parent_camera->viewVector() != m_previous_view_vector && !std::isnan(parent_camera->viewVector().x()) &&
        !std::isnan(parent_camera->viewVector().y()) && !std::isnan(parent_camera->viewVector().z()))
    {
        m_previous_view_vector = parent_camera->viewVector();
        QQuaternion q = QQuaternion::fromDirection(-parent_camera->viewVector(), parent_camera->upVector());
        m_axis_camera->setPosition(
            q.rotatedVector(QVector3D(0, 0, std::max({m_axis_x_length, m_axis_y_length, m_axis_z_length}) * 9.0f)));
        m_axis_camera->setUpVector(q.rotatedVector(QVector3D(0, 1, 0)));
    }

    updateAxisLabelPosition();
}

void Qt3DAxis::constructLabelScene(Qt3DCore::QEntity* parent)
{
    m_label_scene = new Qt3DCore::QEntity;
    m_label_scene->setParent(parent);
    m_label_scene->setEnabled(true);

    m_label_camera = new Qt3DRender::QCamera;
    m_label_camera->setParent(m_label_scene);
    m_label_camera->setProjectionType(Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection);
    m_label_camera->setUpVector(QVector3D(0.0f, 0.0f, 1.0f));
    m_label_camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    m_label_camera->setPosition(QVector3D(0.0f, 0.0f, 100.0f));

    Qt3DRender::QLayer* label_layer = new Qt3DRender::QLayer;
    label_layer->setObjectName("Qt3DAxis_LabelsLayer");
    label_layer->setRecursive(true);
    m_label_scene->addComponent(label_layer);

    Qt3DRender::QLayerFilter* label_layer_filter = new Qt3DRender::QLayerFilter;
    label_layer_filter->addLayer(label_layer);

    Qt3DRender::QCameraSelector* label_camera_selector = new Qt3DRender::QCameraSelector;
    label_camera_selector->setParent(label_layer_filter);
    label_camera_selector->setCamera(m_label_camera);

    Qt3DRender::QSortPolicy* sortPolicy = new Qt3DRender::QSortPolicy(label_camera_selector);
    QVector<Qt3DRender::QSortPolicy::SortType> sortTypes = QVector<Qt3DRender::QSortPolicy::SortType>();
    sortTypes << Qt3DRender::QSortPolicy::BackToFront;
    sortPolicy->setSortTypes(sortTypes);

    Qt3DRender::QClearBuffers* clearBuffers = new Qt3DRender::QClearBuffers(sortPolicy);
    clearBuffers->setBuffers(Qt3DRender::QClearBuffers::DepthBuffer);

    label_layer_filter->setParent(m_viewport);
}

void Qt3DAxis::updateAxisLabelText(Qt3DExtras::QText2DEntity* text_entity, const QString& text)
{
    const float scaled_font_size =
        std::max({m_axis_x_length, m_axis_y_length, m_axis_z_length}) / m_cylinder_length * 16.f;

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(scaled_font_size);
    font.setWeight(QFont::Weight::Black);
    font.setStyleStrategy(QFont::StyleStrategy::ForceOutline);

    text_entity->setFont(font);
    text_entity->setWidth(scaled_font_size * text.length());
    text_entity->setHeight(1.5f * scaled_font_size);
}

void Qt3DAxis::updateAxisLabelPosition()
{
    if (m_text_transform_x && m_text_transform_y && m_text_transform_z)
    {
        m_text_transform_x->setTranslation(project3DTo2D(m_text_coord_x, m_axis_camera, m_label_camera));
        updateAxisLabelText(m_text_x, m_text_x->text());

        m_text_transform_y->setTranslation(project3DTo2D(m_text_coord_y, m_axis_camera, m_label_camera));
        updateAxisLabelText(m_text_y, m_text_y->text());

        m_text_transform_z->setTranslation(project3DTo2D(m_text_coord_z, m_axis_camera, m_label_camera));
        updateAxisLabelText(m_text_z, m_text_z->text());
    }
}

QVector3D Qt3DAxis::project3DTo2D(const QVector3D& source_pos, Qt3DRender::QCamera* source_camera,
                                  Qt3DRender::QCamera* dest_camera)
{
    QRect viewport_rect(static_cast<int>(std::round(m_label_camera->lens()->left())),
                        static_cast<int>(std::round(m_label_camera->lens()->bottom())),
                        static_cast<int>(std::round(m_label_camera->lens()->right() - m_label_camera->lens()->left())),
                        static_cast<int>(std::round(m_label_camera->lens()->top() - m_label_camera->lens()->bottom())));
    QVector3D dest_pos =
        source_pos.project(source_camera->viewMatrix(), dest_camera->projectionMatrix(), viewport_rect);
    dest_pos.setZ(m_text_coord_z.z());
    return dest_pos;
}
