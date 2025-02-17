#pragma once

#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QAbstractCameraController>
#include <Qt3DExtras/QText2DEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QLayer>
#include <QVector3D>

class Qt3DAxis: public QObject
{
    Q_OBJECT
public:
    Qt3DAxis(Qt3DCore::QEntity* parent, Qt3DExtras::QAbstractCameraController* camera_controller);
    ~Qt3DAxis();
    void setAxisLengths(float x, float y, float z);
    void setAxisRadius(float r);
    void init();

private:
    enum class Axis
    {
        X,
        Y,
        Z
    };

    void constructAxisScene(Qt3DCore::QEntity* parent);
    void initAxisScene();
    void createAxis(Axis axis);

    void onCameraChanged();

    void constructLabelScene(Qt3DCore::QEntity* parent);
    void updateAxisLabelText(Qt3DExtras::QText2DEntity* text_entity, const QString& text);
    void updateAxisLabelPosition();

    // FIXME: make it billboard
    QVector3D project3DTo2D(const QVector3D& source_pos, Qt3DRender::QCamera* source_camera,
                            Qt3DRender::QCamera* dest_camera);

private:
    float m_cylinder_length = 20.0f;
    float m_axis_x_length = 20.0f;
    float m_axis_y_length = 20.0f;
    float m_axis_z_length = 20.0f;

    Qt3DCore::QEntity* m_parent = nullptr;
    Qt3DExtras::QAbstractCameraController* m_camera_controller = nullptr;
    Qt3DRender::QViewport* m_viewport = nullptr;

    Qt3DCore::QEntity* m_axis_scene = nullptr;
    Qt3DRender::QLayer* m_axis_object_layer = nullptr;
    Qt3DRender::QCamera* m_axis_camera = nullptr;

    Qt3DCore::QEntity* m_axis_root = nullptr;

    QVector3D m_previous_view_vector{};

    Qt3DCore::QEntity* m_label_scene = nullptr;
    Qt3DRender::QCamera* m_label_camera = nullptr;

    Qt3DExtras::QText2DEntity* m_text_x = nullptr;
    Qt3DExtras::QText2DEntity* m_text_y = nullptr;
    Qt3DExtras::QText2DEntity* m_text_z = nullptr;
    Qt3DCore::QTransform* m_text_transform_x = nullptr;
    Qt3DCore::QTransform* m_text_transform_y = nullptr;
    Qt3DCore::QTransform* m_text_transform_z = nullptr;
    QVector3D m_text_coord_x{};
    QVector3D m_text_coord_y{};
    QVector3D m_text_coord_z{};
};