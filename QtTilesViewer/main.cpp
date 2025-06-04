#include <QtWebEngineWidgets/QWebEngineView>
#include <QApplication>
#include <QLoggingCategory>
#include <QDir>
#include <QStringLiteral>

#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineProfile>

using namespace Qt::StringLiterals;

// tiles viewer based on `OpenSeaDragon`, displayed on `QwebEngineView`
// TODO: use `openslide` instead of `geotiff` to support more WSI formats

class M_WebUrlRequestInterceptor: public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    M_WebUrlRequestInterceptor(QObject* p = Q_NULLPTR): QWebEngineUrlRequestInterceptor(p) {}
    void interceptRequest(QWebEngineUrlRequestInfo& info)
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
    }
};
int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QLoggingCategory::setFilterRules(u"qt.webenginecontext.debug=true"_s);

    auto webInterceptor = QSharedPointer<M_WebUrlRequestInterceptor>::create();
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(webInterceptor.get());

    QWebEngineView view;

    QString localHtmlPath = QDir::currentPath() + "/viewer/demo.html";
    view.setUrl(QUrl::fromLocalFile(localHtmlPath));
    view.resize(800, 600);
    view.show();

    return app.exec();
}

#include "main.moc"