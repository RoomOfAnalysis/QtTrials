#include "trackball_camera_controller.h"

#include <Qt3DInput/QMouseHandler>
#include <Qt3DRender/QCamera>

TrackballCameraController::TrackballCameraController(Qt3DCore::QNode* parent)
    : Qt3DExtras::QAbstractCameraController(parent)
{
    // handle mouse
    Qt3DInput::QMouseHandler* mouseHandler = new Qt3DInput::QMouseHandler(this);
    mouseHandler->setSourceDevice(mouseDevice());
    QObject::connect(mouseHandler, &Qt3DInput::QMouseHandler::pressed, [this](Qt3DInput::QMouseEvent* pressedEvent) {
        pressedEvent->setAccepted(true);
        m_mouseLastPosition = QPoint(pressedEvent->x(), pressedEvent->y());
        m_mouseCurrentPosition = m_mouseLastPosition;
    });
    QObject::connect(mouseHandler, &Qt3DInput::QMouseHandler::positionChanged,
                     [this](Qt3DInput::QMouseEvent* positionChangedEvent) {
                         positionChangedEvent->setAccepted(true);
                         m_mouseCurrentPosition = QPoint(positionChangedEvent->x(), positionChangedEvent->y());
                     });
    // keyboardDevice()
}

QSize TrackballCameraController::windowSize() const
{
    return m_windowSize;
}

float TrackballCameraController::trackballSize() const
{
    return m_trackballSize;
}

float TrackballCameraController::rotationSpeed() const
{
    return m_rotationSpeed;
}

void TrackballCameraController::setWindowSize(QSize windowSize)
{
    if (m_windowSize != windowSize)
    {
        m_windowSize = windowSize;
        emit windowSizeChanged(m_windowSize);
    }
}

void TrackballCameraController::setTrackballSize(float trackballSize)
{
    if (!qFuzzyCompare(m_trackballSize, trackballSize))
    {
        m_trackballSize = trackballSize;
        emit trackballSizeChanged(m_trackballSize);
    }
}

void TrackballCameraController::setRotationSpeed(float rotationSpeed)
{
    if (!qFuzzyCompare(m_rotationSpeed, rotationSpeed))
    {
        m_rotationSpeed = rotationSpeed;
        emit rotationSpeedChanged(m_rotationSpeed);
    }
}

float TrackballCameraController::zoomInLimit() const
{
    return m_zoomInLimit;
}

void TrackballCameraController::setZoomInLimit(float zoomInLimit)
{
    if (!qFuzzyCompare(m_zoomInLimit, zoomInLimit))
    {
        m_zoomInLimit = zoomInLimit;
        emit zoomInLimitChanged();
    }
}

void TrackballCameraController::moveCamera(const Qt3DExtras::QAbstractCameraController::InputState& state, float dt)
{
    auto theCamera = camera();
    if (theCamera == nullptr) return;
    auto ls = linearSpeed();

    //qDebug() << state.rxAxisValue << state.ryAxisValue << state.txAxisValue << state.tyAxisValue << state.tzAxisValue << dt;

    if (state.leftMouseButtonActive)
    {
        QVector3D dir;
        float angle;
        createRotation(m_mouseLastPosition, m_mouseCurrentPosition, dir, angle);
        auto currentRotation = theCamera->transform()->rotation();
        auto rotatedAxis = currentRotation.rotatedVector(dir);
        angle *= m_rotationSpeed;
        theCamera->rotateAboutViewCenter(QQuaternion::fromAxisAndAngle(rotatedAxis, angle * M_1_PI * 180));
    }
    else if (state.middleMouseButtonActive)
    {
        auto offset = m_mouseLastPosition - m_mouseCurrentPosition;
        theCamera->translate(
            QVector3D(offset.x() / float(m_windowSize.width()) * ls, offset.y() / float(m_windowSize.height()) * ls, 0),
            Qt3DRender::QCamera::TranslateViewCenter);
    }
    else if (state.rightMouseButtonActive)
    {
        if ((theCamera->position() - theCamera->viewCenter()).lengthSquared() > m_zoomInLimit * m_zoomInLimit)
            theCamera->translate(QVector3D(0, 0, state.ryAxisValue), theCamera->DontTranslateViewCenter);
        else
            theCamera->translate(QVector3D(0, 0, -0.5), theCamera->DontTranslateViewCenter);
    }
    else
        theCamera->translate(QVector3D(state.txAxisValue * ls, state.tyAxisValue * ls, state.tzAxisValue * ls) * dt,
                             Qt3DRender::QCamera::DontTranslateViewCenter);
    m_mouseLastPosition = m_mouseCurrentPosition;
}

QVector3D TrackballCameraController::projectToTrackball(const QPoint& screenCoords) const
{
    // here y in inversed in my case...
    float sx = screenCoords.x(), sy = screenCoords.y(); // m_windowSize.height() - 1 - screenCoords.y();

    QVector2D p2d(sx / m_windowSize.width() - 0.5f, sy / m_windowSize.height() - 0.5f);

    float z = 0.0f;
    float r2 = m_trackballSize * m_trackballSize;
    if (p2d.lengthSquared() <= r2 * 0.5f)
        z = sqrt(r2 - p2d.lengthSquared());
    else
        z = r2 * 0.5f / p2d.length();
    QVector3D p3d(p2d, z);
    return p3d;
}

void TrackballCameraController::createRotation(const QPoint& firstPoint, const QPoint& nextPoint, QVector3D& dir,
                                               float& angle)
{
    auto lastPos3D = projectToTrackball(firstPoint).normalized();
    auto currentPos3D = projectToTrackball(nextPoint).normalized();

    // Compute axis of rotation:
    dir = QVector3D::crossProduct(currentPos3D, lastPos3D);

    // Approximate rotation angle:
    angle = acos(std::clamp(QVector3D::dotProduct(currentPos3D, lastPos3D), -1.f, 1.f));
}