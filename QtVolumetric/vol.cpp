#include "vol.h"

Vol::Vol(Q3DScatterWidgetItem* scatter): m_graph(scatter)
{
    m_graph->activeTheme()->setTheme(QGraphsTheme::Theme::QtGreenNeon);
    m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::None);
    m_graph->setCameraPreset(QtGraphs3D::CameraPreset::IsometricLeft);
    m_graph->setOrthoProjection(false); // perspective
    m_graph->activeTheme()->setPlotAreaBackgroundVisible(false);

    // Only allow zooming at the center and limit the zoom to 200% to avoid clipping issues
    m_graph->setZoomAtTargetEnabled(false);
    m_graph->setMaxCameraZoomLevel(200.0f);
    m_graph->setMeasureFps(true);

    m_graph->axisX()->setRange(0.0f, 1000.0f);
    m_graph->axisY()->setRange(-600.0f, 600.0f);
    m_graph->axisZ()->setRange(0.0f, 1000.0f);
    m_graph->axisX()->setSegmentCount(5);
    m_graph->axisY()->setSegmentCount(6);
    m_graph->axisZ()->setSegmentCount(5);

    m_volumeItem = new QCustom3DVolume(this);
    m_volumeItem->setScaling(QVector3D(m_graph->axisX()->max() - m_graph->axisX()->min(),
                                       (m_graph->axisY()->max() - m_graph->axisY()->min()) * 0.91f,
                                       m_graph->axisZ()->max() - m_graph->axisZ()->min()));
    m_volumeItem->setPosition(QVector3D((m_graph->axisX()->max() + m_graph->axisX()->min()) / 2.0f,
                                        -0.045f * (m_graph->axisY()->max() - m_graph->axisY()->min()) +
                                            (m_graph->axisY()->max() + m_graph->axisY()->min()) / 2.0f,
                                        (m_graph->axisZ()->max() + m_graph->axisZ()->min()) / 2.0f));
    m_volumeItem->setScalingAbsolute(false);
    m_volumeItem->setSliceFrameGaps(QVector3D(0.01f, 0.02f, 0.01f));
    m_volumeItem->setSliceFrameThicknesses(QVector3D(0.0025f, 0.005f, 0.0025f));
    m_volumeItem->setSliceFrameWidths(QVector3D(0.0025f, 0.005f, 0.0025f));
    m_volumeItem->setDrawSliceFrames(false);

    QObject::connect(m_graph, &Q3DGraphsWidgetItem::currentFpsChanged, this, &Vol::fpsChange);
}

Vol::~Vol()
{
    delete m_graph;
}

void Vol::setFpsLabel(QLabel* fpsLabel)
{
    m_fpsLabel = fpsLabel;
}

void Vol::setSliceLabels(QLabel* xLabel, QLabel* yLabel, QLabel* zLabel)
{
    m_sliceLabelX = xLabel;
    m_sliceLabelY = yLabel;
    m_sliceLabelZ = zLabel;
}

void Vol::sliceX(int value)
{
    if (m_volumeItem)
    {
        m_sliceIndexX = value;
        if (m_volumeItem->sliceIndexX() != -1)
        {
            m_volumeItem->setSliceIndexX(m_sliceIndexX);
            m_sliceLabelX->setPixmap(QPixmap::fromImage(m_volumeItem->renderSlice(Qt::XAxis, m_sliceIndexX))
                                         .scaled(256, 256, Qt::KeepAspectRatio));
        }
    }
}

void Vol::sliceY(int value)
{
    if (m_volumeItem)
    {
        m_sliceIndexY = value;
        if (m_volumeItem->sliceIndexY() != -1)
        {
            m_volumeItem->setSliceIndexY(m_sliceIndexY);
            m_sliceLabelY->setPixmap(QPixmap::fromImage(m_volumeItem->renderSlice(Qt::YAxis, m_sliceIndexY))
                                         .scaled(256, 256, Qt::KeepAspectRatio));
        }
    }
}

void Vol::sliceZ(int value)
{
    if (m_volumeItem)
    {
        m_sliceIndexZ = value;
        if (m_volumeItem->sliceIndexZ() != -1)
        {
            m_volumeItem->setSliceIndexZ(m_sliceIndexZ);
            m_sliceLabelZ->setPixmap(QPixmap::fromImage(m_volumeItem->renderSlice(Qt::ZAxis, m_sliceIndexZ))
                                         .scaled(256, 256, Qt::KeepAspectRatio));
        }
    }
}

void Vol::fpsChange(qreal fps)
{
    const QString fpsFormat = QStringLiteral("FPS: %1");
    int fps10 = int(fps * 10.0);
    m_fpsLabel->setText(fpsFormat.arg(qreal(fps10) / 10.0));
}

void Vol::setSliceSliders(QSlider* sliderX, QSlider* sliderY, QSlider* sliderZ)
{
    m_sliceSliderX = sliderX;
    m_sliceSliderY = sliderY;
    m_sliceSliderZ = sliderZ;

    m_sliceSliderX->setValue(0);
    m_sliceSliderY->setValue(0);
    m_sliceSliderZ->setValue(0);
}

void Vol::loadVolumeData(QList<QImage*> const& volumeData)
{
    // https://doc.qt.io/qt-6/qcustom3dvolume.html#createTextureData
    if (m_volumeItem->createTextureData(volumeData))
    {
        qDebug() << m_volumeItem->textureWidth() << m_volumeItem->textureHeight() << m_volumeItem->textureDepth()
                 << m_volumeItem->textureFormat();
        m_sliceSliderX->setMaximum(m_volumeItem->textureWidth() - 1);
        m_sliceSliderY->setMaximum(m_volumeItem->textureHeight() - 1);
        m_sliceSliderZ->setMaximum(m_volumeItem->textureDepth() - 1);

        m_volumeItem->setPosition(QVector3D(0, 0, 0));

        m_volumeItem->setDrawSlices(false);
        m_volumeItem->setDrawSliceFrames(true);
        m_volumeItem->setSliceIndexX(m_sliceIndexX);
        m_volumeItem->setSliceIndexY(m_sliceIndexY);
        m_volumeItem->setSliceIndexZ(m_sliceIndexZ);

        // FIXME: Failed to create 3D texture: COM error 0x80070057: The parameter is incorrect.
        // the problem is the image size is too large... width * height * depth should < INT_MAX
        m_graph->addCustomItem(m_volumeItem);
    }
    else
        qCritical() << "failed to load volume data";
}

void Vol::loadColorTable(QList<QRgb> const& colorTable)
{
    m_volumeItem->setColorTable(colorTable);
}
