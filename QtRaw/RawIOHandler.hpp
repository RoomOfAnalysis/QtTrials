// code from https://github.com/sandsmark/qtraw

#pragma once

#include <QImageIOHandler>

class RawIOHandler: public QImageIOHandler
{
public:
    RawIOHandler();
    ~RawIOHandler();

    static bool canRead(QIODevice* device);
    bool canRead() const override;
    bool read(QImage* image) override;

    QVariant option(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant& value) override;
    bool supportsOption(ImageOption option) const override;

    QSize thumbnailSize() const;
    QSize fullSize() const;

private:
    class RawIOHandlerPrivate;
    RawIOHandlerPrivate* d;
};
