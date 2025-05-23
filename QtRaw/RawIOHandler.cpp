#include "RawIOHandler.hpp"

#include <QDebug>
#include <QBuffer>
#include <QImageReader>

#include <libraw/libraw.h>
#include <libraw/libraw_datastream.h>

constexpr int JPEG_HIGH_QUALITY_THRESHOLD = 50;
constexpr int JPEG_DEFAULT_QUALITY = 75;

class RawDataStream: public LibRaw_abstract_datastream
{
public:
    RawDataStream(QIODevice* device): io_device(device) {}
    ~RawDataStream() = default;

    int valid() override { return io_device->isReadable(); }
    int read(void* ptr, size_t size, size_t nmemb) override
    {
        auto amount = io_device->read((char*)ptr, size * nmemb);
        if (amount < 0) qDebug() << io_device->errorString();
        return static_cast<int>(amount / static_cast<qint64>(size));
    }
    int seek(INT64 offset, int whence) override
    {
        qint64 pos{};
        switch (whence)
        {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = io_device->pos() + offset;
            break;
        case SEEK_END:
            pos = io_device->size();
            break;
        default:
            return -1;
        }
        if (pos < 0) pos = 0;

        if (io_device->seek(pos)) return 0;
        qDebug() << io_device->errorString();
        return -1;
    }
    INT64 tell() override { return io_device->pos(); }
    INT64 size() override { return io_device->size(); }
    int get_char() override
    {
        if (char c{}; io_device->getChar(&c)) return (unsigned char)c;
        qDebug() << io_device->errorString();
        return -1;
    }
    char* gets(char* s, int n) override
    {
        if (io_device->readLine(s, n) < 0)
        {
            qDebug() << io_device->errorString();
            return nullptr;
        }
        return s;
    }
    int scanf_one(const char* fmt, void* val) override
    {
        QTextStream stream(io_device);
        if (qstrcmp(fmt, "%d") == 0)
        {
            int d;
            stream >> d;
            *(static_cast<int*>(val)) = d;
        }
        else if (qstrcmp(fmt, "%f") == 0)
        {
            float f;
            stream >> f;
            *(static_cast<float*>(val)) = f;
        }
        else
            return 0;
        if (stream.status() != QTextStream::Ok)
        {
            qDebug() << io_device->errorString();
            return EOF;
        }
        return 1;
    }
    int eof() override { return io_device->atEnd(); }

private:
    QIODevice* io_device;
};

static void printError(const char* what, int error)
{
    if (error < 0)
        qWarning() << what << libraw_strerror(error);
    else if (error > 0)
        qWarning() << what << strerror(error);
}

class RawIOHandler::RawIOHandlerPrivate
{
public:
    RawIOHandlerPrivate(RawIOHandler* qq): raw(0), stream(0), q(qq) {}

    ~RawIOHandlerPrivate()
    {
        delete raw;
        raw = 0;
        delete stream;
        stream = 0;
    }

    bool load(QIODevice* device)
    {
        if (device == nullptr) return false;
        if (device->isSequential())
        {
            qDebug() << "Can't read sequential";
            return false;
        }

        if (raw != nullptr) return true;

        qint64 pos = device->pos();
        device->seek(0);

        stream = new RawDataStream(device);
        raw = new LibRaw;

        int ret = raw->open_datastream(stream);
        if (ret != LIBRAW_SUCCESS)
        {
            printError("Opening file failed", ret);
            device->seek(pos);
            delete raw;
            raw = 0;
            delete stream;
            stream = 0;
            return false;
        }

        thumbnailSize = QSize(raw->imgdata.thumbnail.twidth, raw->imgdata.thumbnail.theight);
        defaultSize = QSize(raw->imgdata.sizes.width, raw->imgdata.sizes.height);
        if (raw->imgdata.sizes.flip == 5 || raw->imgdata.sizes.flip == 6)
        {
            thumbnailSize.transpose();
            defaultSize.transpose();
        }
        return true;
    }

    LibRaw* raw;
    RawDataStream* stream;
    QSize thumbnailSize;
    QSize defaultSize;
    QSize scaledSize;
    mutable RawIOHandler* q;

    int quality = JPEG_DEFAULT_QUALITY;
};

RawIOHandler::RawIOHandler(): d(new RawIOHandlerPrivate(this)) {}

RawIOHandler::~RawIOHandler()
{
    delete d;
}

bool RawIOHandler::canRead(QIODevice* device)
{
    if (!device) return false;
    RawIOHandler handler;
    return handler.d->load(device);
}

bool RawIOHandler::canRead() const
{
    if (canRead(device()))
    {
        setFormat("raw");
        return true;
    }
    return false;
}

bool RawIOHandler::read(QImage* image)
{
    if (!d->load(device())) return false;

    const QSize finalSize = d->scaledSize.isValid() ? d->scaledSize : d->defaultSize;
    const QSize thumbnailSize = d->thumbnailSize;
    const libraw_data_t& imgdata = d->raw->imgdata;

    const bool useThumbnail =
        ( // Less than 1% difference
            d->quality <= JPEG_DEFAULT_QUALITY && finalSize.width() / 100 == thumbnailSize.width() / 100 &&
            finalSize.height() / 100 == thumbnailSize.height() / 100) ||
        (finalSize.width() <= thumbnailSize.width() && finalSize.height() <= thumbnailSize.height());

    libraw_processed_image_t* output = nullptr;
    if (useThumbnail)
    {
        qDebug() << "Using thumbnail";
        int ret = d->raw->unpack_thumb();
        if (ret != LIBRAW_SUCCESS)
        {
            printError("Unpacking preview failed", ret);
            return false;
        }
        output = d->raw->dcraw_make_mem_thumb();
    }
    else
    {
        qDebug() << "Using full image";
        int ret = d->raw->unpack();
        // https://www.libraw.org/docs/API-datastruct-eng.html#libraw_output_params_t
        //d->raw->imgdata.params.no_auto_bright = 1;
        d->raw->dcraw_process();
        if (ret != LIBRAW_SUCCESS)
        {
            printError("Processing full image failed", ret);
            return false;
        }
        output = d->raw->dcraw_make_mem_image();
    }

    QImage unscaled;
    std::unique_ptr<uchar[]> pixels = nullptr;
    if (output->type == LIBRAW_IMAGE_JPEG)
    {
        // fromRawData does not copy data
        QByteArray bufferData = QByteArray::fromRawData(reinterpret_cast<char*>(output->data), output->data_size);

        QBuffer buffer(&bufferData);
        // We use QImageReader so we can set quality, qjpeg reads faster when lower quality is set
        QImageReader jpegReader(&buffer);
        jpegReader.setAutoDetectImageFormat(false);
        jpegReader.setFormat("JPG");
        jpegReader.setQuality(d->quality);
        jpegReader.setScaledSize(finalSize);
        if (!jpegReader.read(&unscaled)) return false;

        if (imgdata.sizes.flip != 0)
        {
            QTransform rotation;
            int angle = 0;
            if (imgdata.sizes.flip == 3)
                angle = 180;
            else if (imgdata.sizes.flip == 5)
                angle = -90;
            else if (imgdata.sizes.flip == 6)
                angle = 90;
            if (angle != 0)
            {
                rotation.rotate(angle);
                unscaled = unscaled.transformed(rotation);
            }
        }
    }
    else
    {
        int numPixels = output->width * output->height;
        int colorSize = output->bits / 8;
        int pixelSize = output->colors * colorSize;
        pixels.reset(new uchar[numPixels * 4]);
        uchar* data = output->data;
        for (int i = 0; i < numPixels; i++, data += pixelSize)
        {
            if (output->colors == 3)
            {
                pixels[i * 4] = data[2 * colorSize];
                pixels[i * 4 + 1] = data[1 * colorSize];
                pixels[i * 4 + 2] = data[0];
            }
            else
            {
                pixels[i * 4] = data[0];
                pixels[i * 4 + 1] = data[0];
                pixels[i * 4 + 2] = data[0];
            }
        }
        unscaled = QImage(pixels.get(), output->width, output->height, QImage::Format_RGB32);
    }

    if (unscaled.size() != finalSize)
    {
        Qt::TransformationMode mode = Qt::FastTransformation;

        // If we've loaded a thumbnail, do a smooth transformation
        if (d->quality >= JPEG_HIGH_QUALITY_THRESHOLD || useThumbnail) mode = Qt::SmoothTransformation;
        *image = unscaled.scaled(finalSize, Qt::IgnoreAspectRatio, mode);
    }
    else if (output->type == LIBRAW_IMAGE_BITMAP)
    {
        // make sure that the bits are copied
        *image = unscaled.copy();
    }
    else
    {
        *image = unscaled;
    }
    d->raw->dcraw_clear_mem(output);

    return true;
}

QVariant RawIOHandler::option(ImageOption option) const
{
    switch (option)
    {
    case ImageFormat:
        return QImage::Format_RGB32;
    case Size:
        d->load(device());
        return d->defaultSize;
    case ScaledSize:
        return d->scaledSize;
    case Quality:
        return d->quality;
    default:
        break;
    }
    return QVariant();
}

void RawIOHandler::setOption(ImageOption option, const QVariant& value)
{
    switch (option)
    {
    case ScaledSize:
        d->scaledSize = value.toSize();
        break;
    case Quality:
        d->quality = value.toInt();
        if (d->quality == -1) d->quality = JPEG_DEFAULT_QUALITY;
        break;
    default:
        break;
    }
}

bool RawIOHandler::supportsOption(ImageOption option) const
{
    switch (option)
    {
    case ImageFormat:
        [[fallthrough]];
    case Size:
        [[fallthrough]];
    case ScaledSize:
        [[fallthrough]];
    case Quality:
        return true;
    default:
        break;
    }
    return false;
}

QSize RawIOHandler::thumbnailSize() const
{
    if (d->load(device())) return d->thumbnailSize;
    return {};
}

QSize RawIOHandler::fullSize() const
{
    if (d->load(device())) return d->defaultSize;
    return {};
}
