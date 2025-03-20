#include "RawIOHandler.hpp"

class RawPlugin: public QImageIOPlugin
{
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "raw.json")
#endif

public:
    Capabilities capabilities(QIODevice* device, const QByteArray& format) const override;
    QImageIOHandler* create(QIODevice* device, const QByteArray& format = QByteArray()) const override;
};

QImageIOPlugin::Capabilities RawPlugin::capabilities(QIODevice* device, const QByteArray& format) const
{
    static const QSet<QString> keys = {"arw", "crw", "cr2", "dng",  "nef", "raf", "kderaw", "dcr", "k25", "kdc", "mrw",
                                       "nrw", "orf", "raw", "raw2", "rw",  "rw2", "pef",    "srw", "x3f", "sr2", "srf"};
    if (keys.contains(format) || format == "tif" || format == "tiff") return Capabilities(CanRead);
    if (!format.isEmpty()) return Capabilities();

    Capabilities cap;
    if (device && device->isReadable() && RawIOHandler::canRead(device)) cap |= CanRead;
    return cap;
}

QImageIOHandler* RawPlugin::create(QIODevice* device, const QByteArray& format) const
{
    RawIOHandler* handler = new RawIOHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "RawPlugin.moc"