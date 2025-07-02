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

// https://github.com/openslide/openslide-python/blob/main/openslide/deepzoom.py
class SlideHandler: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString mpp MEMBER m_mpp NOTIFY mppChanged)
    Q_PROPERTY(QString url MEMBER m_url NOTIFY urlChanged)
    Q_PROPERTY(QString dzi MEMBER m_dzi NOTIFY dziChanged)
public:
    SlideHandler(openslide_t* slide, int tile_size = 254, int overlap = 1, QObject* p = nullptr)
        : m_slide(slide), m_tile_size(tile_size), m_overlap(overlap), QObject(p)
    {
        if (auto mpp_x = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_MPP_X); mpp_x)
            if (auto mpp_y = openslide_get_property_value(slide, OPENSLIDE_PROPERTY_NAME_MPP_Y); mpp_y)
                m_mpp = QString::number((std::strtof(mpp_x, nullptr) + std::strtof(mpp_y, nullptr)) / 2.f);

        m_levels = openslide_get_level_count(m_slide);
        m_l_dimensions.reserve(m_levels);
        int64_t w = -1, h = -1;
        for (auto l = 0; l < m_levels; l++)
        {
            openslide_get_level_dimensions(m_slide, l, &w, &h);
            m_l_dimensions.push_back({w, h});
        }

        // qDebug() << "l_dimensions" << m_l_dimensions;

        m_dzl_dimensions.push_back(m_l_dimensions[0]);
        while (m_dzl_dimensions.back().first > 1 || m_dzl_dimensions.back().second > 1)
            m_dzl_dimensions.push_back({std::max(int64_t{1}, (m_dzl_dimensions.back().first + 1) / 2),
                                        std::max(int64_t{1}, (m_dzl_dimensions.back().second + 1) / 2)});
        std::reverse(m_dzl_dimensions.begin(), m_dzl_dimensions.end());
        m_dz_levels = m_dzl_dimensions.size();

        // qDebug() << "dzl_dimensions:" << m_dzl_dimensions;

        m_t_dimensions.reserve(m_dz_levels);
        for (const auto& d : m_dzl_dimensions)
            m_t_dimensions.push_back({static_cast<int64_t>(std::ceil(static_cast<double>(d.first) / m_tile_size)),
                                      static_cast<int64_t>(std::ceil(static_cast<double>(d.second) / m_tile_size))});

        // qDebug() << "t_dimensions:" << m_t_dimensions;

        std::vector<double> level_0_dz_downsamples;
        level_0_dz_downsamples.reserve(m_dz_levels);
        m_preferred_slide_levels.reserve(m_dz_levels);
        for (auto l = 0; l < m_dz_levels; l++)
        {
            auto d = std::pow(2, (m_dz_levels - l - 1));
            level_0_dz_downsamples.push_back(d);
            m_preferred_slide_levels.push_back(openslide_get_best_level_for_downsample(m_slide, d));
        }

        // qDebug() << "preferred_slide_levels:" << m_preferred_slide_levels;

        m_level_downsamples.reserve(m_levels);
        for (auto l = 0; l < m_levels; l++)
            m_level_downsamples.push_back(openslide_get_level_downsample(m_slide, l));

        // qDebug() << "level_downsamples:" << m_level_downsamples;

        m_level_dz_downsamples.reserve(m_dz_levels);
        for (auto l = 0; l < m_dz_levels; l++)
            m_level_dz_downsamples.push_back(level_0_dz_downsamples[l] /
                                             m_level_downsamples[m_preferred_slide_levels[l]]);

        // qDebug() << "level_dz_downsamples:" << m_level_dz_downsamples;

        if (auto bg_color = openslide_get_property_value(m_slide, OPENSLIDE_PROPERTY_NAME_BACKGROUND_COLOR); bg_color)
            m_background_color = QString("#") + bg_color;

        genDzi();
    }
    ~SlideHandler()
    {
        openslide_close(m_slide);
        m_slide = nullptr;
    }

public slots:
    QString getTileUrl(QString url)
    {
        // qDebug() << url;
        auto lxy = url.split("/");
        auto level = lxy[0].toInt();
        auto xy = lxy[1].split("_");
        auto x = xy[0].toInt();
        auto y = xy[1].toInt();

        auto [info, z_size] = getTileInfo(level, x, y);
        auto [l0_location, slide_level, l_size] = info;
        auto [width, height] = l_size;
        auto [xx, yy] = l0_location;

        // qDebug() << "level:" << level << "x:" << x << "y:" << y << "l0_location:" << l0_location
        //          << "slide_level:" << slide_level << "l_size:" << l_size << "z_size:" << z_size;

        // https://openslide.org/docs/premultiplied-argb/
        auto buf = std::make_unique<uint32_t[]>(width * height);
        openslide_read_region(m_slide, buf.get(), xx, yy, slide_level, width, height);
        auto img = QImage((const uchar*)buf.get(), width, height, QImage::Format_ARGB32_Premultiplied)
                       .scaled(z_size.first, z_size.second, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        auto dataUrl = imageToBase64Url(img);
        // qDebug() << dataUrl;
        return dataUrl;
    }
signals:
    void mppChanged(QString mpp);
    void urlChanged(QString url);
    void dziChanged(QString dzi);

private:
    void genDzi()
    {
        int64_t width, height;
        openslide_get_level0_dimensions(m_slide, &width, &height);

        pugi::xml_document xmlDoc;
        pugi::xml_node pre = xmlDoc.prepend_child(pugi::node_declaration);
        pre.append_attribute("version") = "1.0";
        pre.append_attribute("encoding") = "utf-8";

        pugi::xml_node nodeImage = xmlDoc.append_child("Image");
        nodeImage.append_attribute("TileSize") = QString::number(m_tile_size).toUtf8();
        nodeImage.append_attribute("Overlap") = QString::number(m_overlap).toUtf8();
        nodeImage.append_attribute("Format") = "jpg";
        nodeImage.append_attribute("xmlns") = "http://schemas.microsoft.com/deepzoom/2008";

        pugi::xml_node nodeSize = nodeImage.append_child("Size");
        nodeSize.append_attribute("Width") = std::to_string(width).c_str();
        nodeSize.append_attribute("Height") = std::to_string(height).c_str();

        std::stringstream ss;
        xmlDoc.save(ss, "\t", 1U, pugi::encoding_utf8);
        m_dzi = QString::fromStdString(ss.str());
    }

    auto getTileInfo(int dz_level, int col, int row)
    {
        // qDebug() << dz_level << col << row << m_t_dimensions[dz_level].first << m_t_dimensions[dz_level].second;
        // assert((dz_level >= 0 && dz_level < m_dz_levels), "invalid dz level");
        // assert((col >= 0 && col < m_t_dimensions[dz_level].first), "invalid dz col");
        // assert((row >= 0 && row < m_t_dimensions[dz_level].second), "invalid dz row");

        auto slide_level = m_preferred_slide_levels[dz_level];
        auto z_overlap_tl = std::make_pair(m_overlap * int(col != 0), m_overlap * int(row != 0));
        auto z_overlap_br = std::make_pair(m_overlap * int(col != m_t_dimensions[dz_level].first - 1),
                                           m_overlap * int(row != m_t_dimensions[dz_level].second - 1));
        auto z_location = std::make_pair(m_tile_size * col, m_tile_size * row);
        auto z_size = std::make_pair(std::min(m_tile_size, m_dzl_dimensions[dz_level].first - z_location.first) +
                                         z_overlap_tl.first + z_overlap_br.first,
                                     std::min(m_tile_size, m_dzl_dimensions[dz_level].second - z_location.second) +
                                         z_overlap_tl.second + z_overlap_br.second);
        auto l_dz_downsample = m_level_dz_downsamples[dz_level];
        auto l_location = std::make_pair(l_dz_downsample * (z_location.first - z_overlap_tl.first),
                                         l_dz_downsample * (z_location.second - z_overlap_tl.second));
        auto l_downsample = m_level_downsamples[slide_level];
        auto l0_location = std::make_pair(static_cast<int64_t>(l_downsample * l_location.first),
                                          static_cast<int64_t>(l_downsample * l_location.second));
        auto l_size = std::make_pair(
            std::min(static_cast<int64_t>(std::ceil(l_dz_downsample * z_size.first)),
                     m_l_dimensions[slide_level].first - static_cast<int64_t>(std::ceil(l_location.first))),
            std::min(static_cast<int64_t>(std::ceil(l_dz_downsample * z_size.second)),
                     m_l_dimensions[slide_level].second - static_cast<int64_t>(std::ceil(l_location.second))));
        return std::make_pair(std::make_tuple(l0_location, slide_level, l_size), z_size);
    }

private:
    openslide_t* m_slide = nullptr;
    int64_t m_tile_size = 512;
    int m_overlap = 1;
    int m_levels = 0;
    int m_dz_levels = 0;
    std::vector<std::pair<int64_t, int64_t>> m_l_dimensions;   // level dimensions
    std::vector<std::pair<int64_t, int64_t>> m_dzl_dimensions; // deepzoom level dimensions
    std::vector<std::pair<int64_t, int64_t>> m_t_dimensions;   // tile dimensions
    std::vector<int> m_preferred_slide_levels;
    std::vector<double> m_level_downsamples;
    std::vector<double> m_level_dz_downsamples;
    QColor m_background_color = Qt::black;

    QString m_mpp = "1e-6";
    QString m_url;
    QString m_dzi;
};

int main(int argc, char* argv[])
{
    using namespace Qt::StringLiterals;

    // https://doc.qt.io/qt-6/qtwebengine-debugging.html#using-command-line-arguments
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--enable-logging --log-level=0");
    qputenv("QTWEBENGINE_LOCALES_PATH", "./translations/Qt6/qtwebengine_locales");
    QLoggingCategory::setFilterRules(u"qt.webenginecontext.debug=true"_s);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    if (argc < 2)
    {
        qFatal() << "Usage: " << argv[0] << ": <slide path>";
        return -1;
    }

    QApplication app(argc, argv);

    auto webInterceptor = QSharedPointer<M_WebUrlRequestInterceptor>::create();
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(webInterceptor.get());

    auto page = QSharedPointer<M_WebEnginePage>::create();
    auto view = QSharedPointer<QWebEngineView>::create();
    view->setPage(page.get());

    auto channel = QSharedPointer<QWebChannel>::create();
    page->setWebChannel(channel.get());

    QString url = argv[1];
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
    }

    auto slide_handler = QSharedPointer<SlideHandler>::create(slide, 254, 1, view.get());
    slide_handler->setProperty("url", url);

    channel->registerObject("slide_handler", slide_handler.get());

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
    image.save(&buffer, "JPG", 75);
    QString base64Url = "data:image/jpg;base64," + QString::fromLatin1(byteArray.toBase64());
    return base64Url;
}

#include "main2.moc"