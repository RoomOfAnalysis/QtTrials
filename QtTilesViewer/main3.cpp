#include <QApplication>
#include <QLoggingCategory>
#include <QDir>

#include <QDebug>
#include <QImage>
#include <QBuffer>

#include <QMainWindow>
#include <QToolBar>
#include <QFileDialog>

#include <QCefContext.h>
#include <QCefSetting.h>
#include <QCefView.h>
#include <QCefQuery.h>

#include <openslide.h>

#include <memory>
#include <sstream>

QString imageToBase64Url(QImage image);

class SlideHandler: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString mpp MEMBER m_mpp NOTIFY mppChanged)
    Q_PROPERTY(QString dzi MEMBER m_dzi NOTIFY dziChanged)
public:
    SlideHandler(QObject* p = nullptr): QObject(p) {}

    void setSlide(QString url, int tile_size = 254, int overlap = 1)
    {
        m_slide = openslide_open(url.toStdString().c_str());
        if (!m_slide)
        {
            printf("Failed to open slide: %s\n", openslide_get_error(m_slide));
            return;
        }
        m_tile_size = tile_size;
        m_overlap = overlap;

        if (auto mpp_x = openslide_get_property_value(m_slide, OPENSLIDE_PROPERTY_NAME_MPP_X); mpp_x)
            if (auto mpp_y = openslide_get_property_value(m_slide, OPENSLIDE_PROPERTY_NAME_MPP_Y); mpp_y)
                m_mpp = QString::number((std::strtof(mpp_x, nullptr) + std::strtof(mpp_y, nullptr)) / 2.f);

        m_levels = openslide_get_level_count(m_slide);
        m_l_dimensions.reserve(m_levels);
        int64_t w = -1, h = -1;
        for (auto l = 0; l < m_levels; l++)
        {
            openslide_get_level_dimensions(m_slide, l, &w, &h);
            m_l_dimensions.push_back({w, h});
        }

        m_dzl_dimensions.push_back(m_l_dimensions[0]);
        while (m_dzl_dimensions.back().first > 1 || m_dzl_dimensions.back().second > 1)
            m_dzl_dimensions.push_back({std::max(int64_t{1}, (m_dzl_dimensions.back().first + 1) / 2),
                                        std::max(int64_t{1}, (m_dzl_dimensions.back().second + 1) / 2)});
        std::reverse(m_dzl_dimensions.begin(), m_dzl_dimensions.end());
        m_dz_levels = m_dzl_dimensions.size();

        m_t_dimensions.reserve(m_dz_levels);
        for (const auto& d : m_dzl_dimensions)
            m_t_dimensions.push_back({static_cast<int64_t>(std::ceil(static_cast<double>(d.first) / m_tile_size)),
                                      static_cast<int64_t>(std::ceil(static_cast<double>(d.second) / m_tile_size))});

        std::vector<double> level_0_dz_downsamples;
        level_0_dz_downsamples.reserve(m_dz_levels);
        m_preferred_slide_levels.reserve(m_dz_levels);
        for (auto l = 0; l < m_dz_levels; l++)
        {
            auto d = std::pow(2, (m_dz_levels - l - 1));
            level_0_dz_downsamples.push_back(d);
            m_preferred_slide_levels.push_back(openslide_get_best_level_for_downsample(m_slide, d));
        }

        m_level_downsamples.reserve(m_levels);
        for (auto l = 0; l < m_levels; l++)
            m_level_downsamples.push_back(openslide_get_level_downsample(m_slide, l));

        m_level_dz_downsamples.reserve(m_dz_levels);
        for (auto l = 0; l < m_dz_levels; l++)
            m_level_dz_downsamples.push_back(level_0_dz_downsamples[l] /
                                             m_level_downsamples[m_preferred_slide_levels[l]]);

        if (auto bg_color = openslide_get_property_value(m_slide, OPENSLIDE_PROPERTY_NAME_BACKGROUND_COLOR); bg_color)
            m_background_color = QString("#") + bg_color;

        m_dzi = genDzi();
    }
    ~SlideHandler()
    {
        if (m_slide)
        {
            openslide_close(m_slide);
            m_slide = nullptr;
        }
    }

public:
    QString getDzi() { return m_dzi; }
    QString getMpp() { return m_mpp; }

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

        auto buf = std::make_unique<uint32_t[]>(width * height);
        openslide_read_region(m_slide, buf.get(), xx, yy, slide_level, width, height);
        auto img = QImage((const uchar*)buf.get(), width, height, QImage::Format_ARGB32_Premultiplied)
                       .scaled(z_size.first, z_size.second, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        auto dataUrl = imageToBase64Url(img);
        return dataUrl;
    }
signals:
    void mppChanged(QString mpp);
    void dziChanged(QString dzi);

private:
    QString genDzi()
    {
        auto const& [width, height] = m_l_dimensions[0];
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n \
<Image xmlns = \"http://schemas.microsoft.com/deepzoom/2008\"\n \
  Format=\"jpg\"\n \
  Overlap=\"" + QString::number(m_overlap) +
               "\"\n \
  TileSize=\"" +
               QString::number(m_tile_size) + "\"\n \
  >\n \
  <Size\n \
    Height=\"" +
               QString::number(height) + "\"\n \
    Width=\"" + QString::number(width) +
               "\"\n \
  />\n \
</Image>";
    }

    auto getTileInfo(int dz_level, int col, int row)
    {
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
    QString m_dzi;
};

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString uri, QWidget* parent = nullptr): QMainWindow(parent)
    {
        slide_handler = std::make_unique<SlideHandler>();

        QCefSetting setting;
        setting.setBackgroundColor(QColor::fromRgb(0, 0, 0));
        cefViewWidget = new QCefView(uri, &setting, this);
        setCentralWidget(cefViewWidget);

        connect(cefViewWidget, &QCefView::cefQueryRequest, this, &MainWindow::onQCefQueryRequest);
        connect(cefViewWidget, &QCefView::consoleMessage, this, [&](const QString& message, int level) {
            qDebug() << "js log, level: " << level << ", msg: " << message;
        });

        auto* file_toolbar = addToolBar("File");
        file_toolbar->addAction("Open", [&]() {
            auto url = QFileDialog::getOpenFileName(this, "Open Images", QDir::homePath());
            if (url.isEmpty()) return;

            slide_handler = std::make_unique<SlideHandler>();
            slide_handler->setSlide(url, 254, 1);

            QCefEvent event("UrlChanged");
            event.arguments().append(url);
            event.arguments().append(slide_handler->getDzi());
            event.arguments().append(slide_handler->getMpp());
            cefViewWidget->broadcastEvent(event);
        });
    }

public slots:
    void onQCefQueryRequest(int const& browserId, QString const& frameId, QCefQuery const& query)
    {
        QMetaObject::invokeMethod(
            this,
            [=]() {
                //QString text = QString("Current Thread: QT_UI\r\n"
                //                       "Query: %1")
                //                   .arg(query.request());
                //qDebug() << text;

                QString response = slide_handler->getTileUrl(query.request());
                query.setResponseResult(true, response);
                cefViewWidget->responseQCefQuery(query);
            },
            Qt::QueuedConnection);
    }

private:
    std::unique_ptr<SlideHandler> slide_handler = nullptr;
    QCefView* cefViewWidget = nullptr;
};

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    QCefConfig config;
    // Set user agent
    config.setUserAgent("QCefViewTest");
    // Set log level
    config.setLogLevel(QCefConfig::LOGSEVERITY_DEFAULT);
    // Set JSBridge object name (default value is QCefViewClient)
    config.setBridgeObjectName("CallBridge"); // similar to QWebChannel
    // Port for remote debugging (default is 0, disabling remote debugging)
    config.setRemoteDebuggingPort(9000);
    // Set background color for all browsers
    // (QCefSetting.setBackgroundColor will overwrite this value for a specific browser instance)
    config.setBackgroundColor(Qt::lightGray);

    // WindowlessRenderingEnabled is true by default, disable OSR mode by setting it to false
    config.setWindowlessRenderingEnabled(true);

    // Add command line arguments (any CEF-supported switches or parameters)
    config.addCommandLineSwitch("use-mock-keychain");
    // config.addCommandLineSwitch("disable-spell-checking");
    // config.addCommandLineSwitch("disable-site-isolation-trials");
    // config.addCommandLineSwitch("enable-aggressive-domstorage-flushing");
    config.addCommandLineSwitchWithValue("renderer-process-limit", "1");
    // config.addCommandLineSwitchWithValue("disable-features", "BlinkGenPropertyTrees,TranslateUI,site-per-process");

    // 3. Create the QCefContext instance, passing the QApplication instance and config
    // The lifecycle of cefContext must match the QApplication instance's lifecycle
    QCefContext cefContext(&app, argc, argv, &config);

    QString localHtmlPath = QApplication::applicationDirPath() + "/viewer3/demo.html";
    MainWindow window(QUrl::fromLocalFile(localHtmlPath).toString());
    window.resize(800, 600);
    window.show();

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

#include "main3.moc"