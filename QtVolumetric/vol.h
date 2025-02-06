#pragma once

#include <Q3DScatterWidgetItem>
#include <QCustom3DVolume>
#include <QLabel>
#include <QSlider>
#include <QTimer>

class Vol: public QObject
{
    Q_OBJECT

public:
    explicit Vol(Q3DScatterWidgetItem* scatter);
    ~Vol();

    void setFpsLabel(QLabel* fpsLabel);
    void setSliceLabels(QLabel* xLabel, QLabel* yLabel, QLabel* zLabel);
    void setSliceSliders(QSlider* sliderX, QSlider* sliderY, QSlider* sliderZ);

    void loadVolumeData(QList<QImage*> const& volumeData);
    void loadColorTable(QList<QRgb> const& colorTable);

public slots:
    void fpsChange(qreal fps);

    void sliceX(int value);
    void sliceY(int value);
    void sliceZ(int value);

private:
    Q3DScatterWidgetItem* m_graph = nullptr;
    QCustom3DVolume* m_volumeItem = nullptr;
    int m_sliceIndexX = 0;
    int m_sliceIndexY = 0;
    int m_sliceIndexZ = 0;

    QLabel* m_fpsLabel = nullptr;
    QLabel* m_sliceLabelX = nullptr;
    QLabel* m_sliceLabelY = nullptr;
    QLabel* m_sliceLabelZ = nullptr;
    QSlider* m_sliceSliderX = nullptr;
    QSlider* m_sliceSliderY = nullptr;
    QSlider* m_sliceSliderZ = nullptr;
};