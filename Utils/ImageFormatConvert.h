#pragma once

#include <QImage>

class vtkRenderWindow;
class vtkImageData;
class vtkScalarsToColors;

namespace Utils
{
    QImage GetQImageFromVTKWindowCapture(vtkRenderWindow* render_window);

    QImage GetQImageFromVTKImageData(vtkImageData* image_data);

    bool GetVTKImageDataFromQImage(QImage const& input_img, vtkImageData* output_image_data, bool force_alpha_channel);
} // namespace Utils
