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

    m_volumeItem = new QCustom3DVolume(this);

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

    sliceX(m_sliceSliderX->value());
    sliceY(m_sliceSliderY->value());
    sliceZ(m_sliceSliderZ->value());
}

void Vol::sliceX(int value)
{
    if (m_volumeItem)
    {
        m_sliceIndexX = value / (1024 / m_volumeItem->textureWidth());
        if (m_sliceIndexX == m_volumeItem->textureWidth()) m_sliceIndexX--;
        if (m_volumeItem->sliceIndexX() != -1) m_volumeItem->setSliceIndexX(m_sliceIndexX);
        m_sliceLabelX->setPixmap(QPixmap::fromImage(m_volumeItem->renderSlice(Qt::XAxis, m_sliceIndexX)));
    }
}

void Vol::sliceY(int value)
{
    if (m_volumeItem)
    {
        m_sliceIndexY = value / (1024 / m_volumeItem->textureHeight());
        if (m_sliceIndexY == m_volumeItem->textureHeight()) m_sliceIndexY--;
        if (m_volumeItem->sliceIndexY() != -1) m_volumeItem->setSliceIndexY(m_sliceIndexY);
        m_sliceLabelY->setPixmap(QPixmap::fromImage(m_volumeItem->renderSlice(Qt::YAxis, m_sliceIndexY)));
    }
}

void Vol::sliceZ(int value)
{
    if (m_volumeItem)
    {
        m_sliceIndexZ = value / (1024 / m_volumeItem->textureDepth());
        if (m_sliceIndexZ == m_volumeItem->textureDepth()) m_sliceIndexZ--;
        if (m_volumeItem->sliceIndexZ() != -1) m_volumeItem->setSliceIndexZ(m_sliceIndexZ);
        m_sliceLabelZ->setPixmap(QPixmap::fromImage(m_volumeItem->renderSlice(Qt::ZAxis, m_sliceIndexZ)));
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
        m_sliceSliderX->setMaximum(m_volumeItem->textureWidth());
        m_sliceSliderY->setMaximum(m_volumeItem->textureHeight());
        m_sliceSliderZ->setMaximum(m_volumeItem->textureDepth());
    }
}

void Vol::loadColorTable(QList<QRgb> const& colorTable)
{
    m_volumeItem->setColorTable(colorTable);
}
