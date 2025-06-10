#include <QtWebEngineWidgets/QWebEngineView>
#include <QApplication>
#include <QLoggingCategory>
#include <QDir>
#include <QStringLiteral>

#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineProfile>
#include <QWebChannel>
#include <QDebug>
#include <QImage>
#include <QBuffer>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlScheme>

#include <openslide.h>
#include <pugixml.hpp>

#include <memory>
#include <sstream>

QString imageToBase64Url(QImage image);

// tiles viewer based on `OpenSeaDragon`, displayed on `QwebEngineView`
// use `openslide` to support more WSI formats

class M_WebUrlRequestInterceptor: public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    M_WebUrlRequestInterceptor(QObject* p = Q_NULLPTR): QWebEngineUrlRequestInterceptor(p) {}
    void interceptRequest(QWebEngineUrlRequestInfo& info) override
    {
        QString rsrct = "";
        // https://doc.qt.io/qt-6/qwebengineurlrequestinfo.html#ResourceType-enum
        switch (info.resourceType())
        {
        case QWebEngineUrlRequestInfo::ResourceTypeMainFrame:
            rsrct = "ResourceTypeMainFrame = 0, // top level page";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeSubFrame:
            rsrct = "ResourceTypeSubFrame, // frame or iframe";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeStylesheet:
            rsrct = "ResourceTypeStylesheet, // a CSS stylesheet";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeScript:
            rsrct = "ResourceTypeScript, // an external script";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeImage:
            rsrct = "ResourceTypeImage, // an image (jpg/gif/png/etc)";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeFontResource:
            rsrct = "ResourceTypeFontResource, // a font";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeSubResource:
            rsrct = "ResourceTypeSubResource, // an other subresource.";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeObject:
            rsrct = "ResourceTypeObject, // an object (or embed) tag for a plugin,";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeMedia:
            rsrct = "ResourceTypeMedia, // a media resource.";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeWorker:
            rsrct = "ResourceTypeWorker, // the main resource of a dedicated worker.";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeSharedWorker:
            rsrct = "ResourceTypeSharedWorker, // the main resource of a shared worker.";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypePrefetch:
            rsrct = "ResourceTypePrefetch, // an explicitly requested prefetch";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeFavicon:
            rsrct = "ResourceTypeFavicon, // a favicon";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeXhr:
            rsrct = "ResourceTypeXhr, // a XMLHttpRequest";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypePing:
            rsrct = "ResourceTypePing, // a ping request for <a ping>";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeServiceWorker:
            rsrct = "ResourceTypeServiceWorker, // the main resource of a service worker.";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeCspReport:
            rsrct = "ResourceTypeCspReport, // a report of a Content Security Policy (CSP)";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypePluginResource:
            rsrct = "ResourceTypePluginResource, // a resource requested by a plugin";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeNavigationPreloadMainFrame:
            rsrct =
                "ResourceTypeNavigationPreloadMainFrame, // a  main-frame service worker navigation preload request";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeNavigationPreloadSubFrame:
            rsrct = "ResourceTypeNavigationPreloadSubFrame, // a sub-frame service worker navigation preload request";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeWebSocket:
            rsrct = "ResourceTypeWebSocket, // a WebSocket request";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeJson:
            rsrct = "ResourceTypeJson, // a JSON module";
            break;
        case QWebEngineUrlRequestInfo::ResourceTypeUnknown:
            rsrct = "ResourceTypeUnknown, // unknown request type";
            break;

        default:
            rsrct = "unknown";
            break;
        }
        qDebug() << Q_FUNC_INFO << ": " << info.requestMethod() << info.requestUrl().toString() << rsrct;

        // // seems not work
        // if (info.requestUrl().scheme() == "myscheme") qDebug() << Q_FUNC_INFO << ": " << info.requestUrl().toString();

        // if (info.requestMethod() == "GET" && info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeImage &&
        //     info.requestUrl().toString().contains("_files"))
        // {
        //     auto url = info.requestUrl().toString().split("_files")[0];
        //     qDebug() << Q_FUNC_INFO << ": " << url;
        //     auto file = url.split("///")[1];
        //     qDebug() << Q_FUNC_INFO << ": " << file;

        //     // QWebEngineUrlRequestInterceptor only allows you to block or redirect a request to another HTTP/HTTPS URL, not a custom scheme directly
        //     // info.redirect("myscheme://" + file);

        //     // openslide_t* slide = openslide_open(url.toUtf8().constData());
        //     // if (!slide)
        //     // {
        //     //     qFatal() << "Failed to open slide: " << openslide_get_error(slide);
        //     //     return;
        //     // }
        //     // qDebug() << Q_FUNC_INFO << ": opened";
        //     // int64_t width, height;
        //     // openslide_get_level_dimensions(slide, 0, &width, &height);
        //     // qDebug() << Q_FUNC_INFO << ": " << width << height;
        //     // auto buf = std::make_unique<uint32_t[]>(width * width);
        //     // openslide_read_region(slide, buf.get(), 0, 0, 0, width, width);
        //     // QImage image(reinterpret_cast<const uchar*>(buf.get()), width, width, QImage::Format_ARGB32);
        //     // qDebug() << imageToBase64Url(image);
        //     // //info.redirect(imageToBase64Url(image));
        //     // openslide_close(slide);
        // }
    }
};

class M_WebEnginePage: public QWebEnginePage
{
protected:
    //https://doc.qt.io/qt-6.8/qml-qtwebengine-webengineview.html#javaScriptConsoleMessage-signal
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber,
                                  const QString& sourceID) override
    {
        qDebug() << Q_FUNC_INFO << ": " << level << message << lineNumber << sourceID;
    }
};

class SlideHandler: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString mpp MEMBER m_mpp NOTIFY mppChanged)
    Q_PROPERTY(QString url MEMBER m_url NOTIFY urlChanged)
    Q_PROPERTY(QString dzi MEMBER m_dzi NOTIFY dziChanged)
public:
    SlideHandler(openslide_t* slide, QObject* p): m_slide(slide), QObject(p) {}
    ~SlideHandler()
    {
        openslide_close(m_slide);
        m_slide = nullptr;
    }

public slots:
    void openfile(QString msg) { qDebug() << "Msg:" << msg; }
    QString getTileUrl(QString url)
    {
        qDebug() << url;
        auto lxy = url.split("/");
        auto level = lxy[0].toInt();
        auto xy = lxy[1].split("_");
        auto x = xy[0].toInt();
        auto y = xy[1].toInt();
        int width = 254, height = 254;
        auto buf = std::make_unique<uint32_t[]>(width * height);
        openslide_read_region(m_slide, buf.get(), x, y, level, width, height);
        // FIXME: incorrect conversion
        QImage img((const uchar*)buf.get(), width, height, QImage::Format_ARGB32);
        img.fill(Qt::white);
        auto dataUrl = imageToBase64Url(img);
        //qDebug() << dataUrl;
        return dataUrl;
    }
signals:
    void sigNotify(QString msg);
    void mppChanged(QString mpp);
    void urlChanged(QString url);
    void dziChanged(QString dzi);

private:
    openslide_t* m_slide;
    QString m_mpp = "1e-6";
    QString m_url = "D:/volumes/tusd/upload_data/dzi/CMU-1-Small-Region.svs";
    QString m_dzi = m_url + ".dzi";
};

class M_WebEngineUrlSchemeHandler: public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    M_WebEngineUrlSchemeHandler(QObject* parent = nullptr): QWebEngineUrlSchemeHandler(parent) {}
public slots:
    void requestStarted(QWebEngineUrlRequestJob* job) override
    {

        const QByteArray method = job->requestMethod();
        const QUrl url = job->requestUrl();
        qDebug() << Q_FUNC_INFO << ": " << method << url.toString();

        QBuffer* buffer = new QBuffer;
        connect(job, SIGNAL(destroyed()), buffer, SLOT(deleteLater()));

        buffer->open(QIODevice::WriteOnly);
        QImage image(url.path());
        image.save(buffer, "JPEG");
        buffer->close();

        job->reply("image/jpeg", buffer);
    }
};
int main(int argc, char* argv[])
{
    using namespace Qt::StringLiterals;

    // https://doc.qt.io/qt-6/qtwebengine-debugging.html#using-command-line-arguments
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--enable-logging --log-level=0");
    qputenv("QTWEBENGINE_LOCALES_PATH", "./translations/Qt6/qtwebengine_locales");
    QLoggingCategory::setFilterRules(u"qt.webenginecontext.debug=true"_s);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // QWebEngineUrlScheme scheme("myscheme");
    // scheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    // scheme.setDefaultPort(QWebEngineUrlScheme::PortUnspecified);
    // scheme.setFlags(QWebEngineUrlScheme::SecureScheme);
    // QWebEngineUrlScheme::registerScheme(scheme);

    QApplication app(argc, argv);

    auto webInterceptor = QSharedPointer<M_WebUrlRequestInterceptor>::create();
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(webInterceptor.get());
    // QWebEngineProfile::defaultProfile()->installUrlSchemeHandler("myscheme", new M_WebEngineUrlSchemeHandler);

    auto page = QSharedPointer<M_WebEnginePage>::create();
    auto view = QSharedPointer<QWebEngineView>::create();
    view->setPage(page.get());

    auto channel = QSharedPointer<QWebChannel>::create();
    page->setWebChannel(channel.get());

    float mpp = 1e-6;
    QString url = "D:/volumes/tusd/upload_data/orgin/CMU-1-Small-Region.svs";
    QString dzi = url + ".dzi";
    openslide_t* slide = openslide_open(url.toUtf8().constData());
    if (!slide)
    {
        qFatal() << "Failed to open slide: " << openslide_get_error(slide);
        return -1;
    }
    else
    {
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_VENDOR); p)
            qDebug() << "Slide Vendor: " << p;
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_COMMENT); p)
            qDebug() << "Slide Comment: " << p;
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_BACKGROUND_COLOR); p)
            qDebug() << "Background Color: " << p;
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_MPP_X); p)
            qDebug() << "MPP_X: " << p;
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_MPP_Y); p)
            qDebug() << "MPP_Y: " << p;
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_BOUNDS_WIDTH); p)
            qDebug() << "Bounds Width: " << p;
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_BOUNDS_HEIGHT); p)
            qDebug() << "Bounds Height: " << p;
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_BOUNDS_X); p)
            qDebug() << "Bounds X: " << p;
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_BOUNDS_Y); p)
            qDebug() << "Bounds Y: " << p;
        if (auto const* p = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_OBJECTIVE_POWER); p)
            qDebug() << "Objective Power: " << p;

        mpp = (std::strtof(openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_MPP_X), nullptr) +
               std::strtof(openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_MPP_Y), nullptr)) /
              2.f;

        auto levels = openslide_get_level_count(slide);

        int64_t width, height;
        openslide_get_level0_dimensions(slide, &width, &height);

        pugi::xml_document xmlDoc;
        pugi::xml_node pre = xmlDoc.prepend_child(pugi::node_declaration);
        pre.append_attribute("version") = "1.0";
        pre.append_attribute("encoding") = "utf-8";

        pugi::xml_node nodeImage = xmlDoc.append_child("Image");
        nodeImage.append_attribute("TileSize") = "254";
        nodeImage.append_attribute("Overlap") = "1";
        nodeImage.append_attribute("Format") = "jpg";
        nodeImage.append_attribute("xmlns") = "http://schemas.microsoft.com/deepzoom/2008";

        pugi::xml_node nodeSize = nodeImage.append_child("Size");
        nodeSize.append_attribute("Width") = std::to_string(width).c_str();
        nodeSize.append_attribute("Height") = std::to_string(height).c_str();

        std::stringstream ss;
        xmlDoc.save(ss, "\t", 1U, pugi::encoding_utf8);
        dzi = QString::fromStdString(ss.str());
    }

    auto slide_handler = QSharedPointer<SlideHandler>::create(slide, view.get());
    slide_handler->setProperty("mpp", QString::number(mpp));
    slide_handler->setProperty("url", url);
    slide_handler->setProperty("dzi", dzi);

    channel->registerObject("slide_handler", slide_handler.get());

    emit slide_handler->sigNotify("I come from Cpp call qt sig whcih connect js func! ");

    QString localHtmlPath = QDir::currentPath() + "/viewer2/demo.html";
    view->setUrl(QUrl::fromLocalFile(localHtmlPath));
    view->resize(800, 600);
    view->show();

    return app.exec();
}

QString imageToBase64Url(QImage image)
{
    if (image.isNull())
    {
        qFatal() << "image is null";
        return "";
    }

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPEG");
    QString base64Url = "data:image/jpeg;base64," + QString::fromLatin1(byteArray.toBase64());
    return base64Url;
}

#include "main2.moc"