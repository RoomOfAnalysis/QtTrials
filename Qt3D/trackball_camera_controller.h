#pragma once

#include <Qt3DExtras/qabstractcameracontroller.h>
#include <QPoint>
#include <QSize>

// code from https://github.com/cjmdaixi/Qt3DTrackball
// https://github.com/rayform/QTrackballCameraController
// https://github.com/qt/qt3d/blob/dev/src/extras/defaults/qorbitcameracontroller.cpp
// https://github.com/unclejimbo/Klein/blob/master/src/Input/TrackballCameraController.cpp
class TrackballCameraController: public Qt3DExtras::QAbstractCameraController
{
    Q_OBJECT
    Q_PROPERTY(QSize windowSize READ windowSize WRITE setWindowSize NOTIFY windowSizeChanged)
    Q_PROPERTY(float trackballSize READ trackballSize WRITE setTrackballSize NOTIFY trackballSizeChanged)
    Q_PROPERTY(float rotationSpeed READ rotationSpeed WRITE setRotationSpeed NOTIFY rotationSpeedChanged)
    Q_PROPERTY(float zoomInLimit READ zoomInLimit WRITE setZoomInLimit NOTIFY zoomInLimitChanged)
public:
    TrackballCameraController(Qt3DCore::QNode* parent = nullptr);

    QSize windowSize() const;
    float trackballSize() const;
    float rotationSpeed() const;
    float zoomInLimit() const;

public slots:
    void setWindowSize(QSize windowSize);
    void setTrackballSize(float trackballSize);
    void setRotationSpeed(float rotationSpeed);
    void setZoomInLimit(float zoomInLimit);

signals:
    void windowSizeChanged(QSize windowSize);
    void trackballSizeChanged(float trackballSize);
    void rotationSpeedChanged(float rotationSpeed);
    void zoomInLimitChanged();

protected:
    void moveCamera(const Qt3DExtras::QAbstractCameraController::InputState& state, float dt) override;

private:
    QVector3D projectToTrackball(const QPoint& screenCoords) const;
    void createRotation(const QPoint& firstPoint, const QPoint& nextPoint, QVector3D& dir, float& angle);

private:
    QPoint m_mouseLastPosition{}, m_mouseCurrentPosition{};
    QSize m_windowSize{1920, 1080};
    float m_rotationSpeed = 1.0f;
    float m_trackballSize = 1.0f;
    float m_zoomInLimit = 2.f;
};