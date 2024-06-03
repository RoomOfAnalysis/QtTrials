#include "ImageFormatConvert.h"

#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkImageData.h>
#include <vtkScalarsToColors.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkWindowToImageFilter.h>

#include <QDebug>

#include <cstring>

QImage Utils::GetQImageFromVTKWindowCapture(vtkRenderWindow* render_window)
{
    vtkNew<vtkWindowToImageFilter> filter;
    filter->SetInput(render_window);
    filter->SetViewport(0, 0, 1, 1);
    filter->SetScale(1);
    filter->SetInputBufferTypeToRGB();
    filter->ReadFrontBufferOn();
    filter->Update();
    auto* image_data = filter->GetOutput();
    return GetQImageFromVTKImageData(image_data);
}

QImage Utils::GetQImageFromVTKImageData(vtkImageData* image_data)
{
    if (!image_data || !image_data->GetPointData() || !image_data->GetPointData()->GetScalars() ||
        image_data->GetScalarType() != VTK_UNSIGNED_CHAR)
        return {};

    QImage img{};
    auto width = image_data->GetDimensions()[0];
    auto height = image_data->GetDimensions()[1];
    auto channels = image_data->GetNumberOfScalarComponents();
    switch (channels)
    {
    case 1:
        img = QImage(width, height, QImage::Format_Grayscale8);
        break;
    case 3:
        img = QImage(width, height, QImage::Format_RGB888);
        break;
    case 4:
        img = QImage(width, height, QImage::Format_RGBA8888);
        break;
    default:
        return img;
    }

    memcpy(img.bits(), image_data->GetPointData()->GetScalars()->GetVoidPointer(0),
           static_cast<size_t>(channels) * width * height);
    return img.mirrored(); // QImage is upside-down compared to VTK
}

bool Utils::GetVTKImageDataFromQImage(QImage const& input_img, vtkImageData* output_image_data,
                                      bool force_alpha_channel)
{
    if (!output_image_data)
    {
        qWarning() << Q_FUNC_INFO << " failed: output_image_data is invalid";
        return false;
    }

    auto sz = input_img.size();
    auto width = sz.width();
    auto height = sz.height();
    if (width < 1 || height < 1)
    {
        qWarning() << Q_FUNC_INFO << " failed: input_img is invalid";
        return false;
    }

    QImage img{};
    int channels = 0;
    if (input_img.hasAlphaChannel() || force_alpha_channel)
    {
        img = input_img.convertToFormat(QImage::Format_RGBA8888);
        channels = 4;
    }
    else
    {
        img = input_img.convertToFormat(QImage::Format_RGB888);
        channels = 3;
    }
    img.mirror();

    output_image_data->SetExtent(0, width - 1, 0, height - 1, 0, 0);
    output_image_data->AllocateScalars(VTK_UNSIGNED_CHAR, channels);
    memcpy(output_image_data->GetPointData()->GetScalars()->GetVoidPointer(0), img.bits(),
           static_cast<size_t>(channels) * width * height);
    output_image_data->GetPointData()->GetScalars()->Modified();
    return true;
}