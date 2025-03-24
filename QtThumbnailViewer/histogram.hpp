#pragma once

#include <QImage>

struct Histogram
{
    float red[256]{};
    float green[256]{};
    float blue[256]{};
};

Histogram CalcHist(QImage const& img);