#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QShortcut>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QProcess>
#include <QScrollBar>
#include <QImageReader>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QMessageBox>

#include "image2dviewer.hpp"
#include "viewer.hpp"
#include "cmpviewer.hpp"

QString EncodePath(QString const& origin_path)
{
    return origin_path.toUtf8().toBase64(QByteArray::Base64Option::Base64UrlEncoding);
}

QString DecodePath(QString const& cached_dir)
{
    return QByteArray::fromBase64(cached_dir.toUtf8(), QByteArray::Base64Option::Base64UrlEncoding);
}

template <typename T>
concept ImagesLoadable = requires(T t, QList<QImage> const& images, QStringList const& paths) {
    { t.setImages(images, paths) } -> std::same_as<void>;
    { T::loadImages(paths) } -> std::same_as<QList<QImage>>;
};

template <typename T>
concept ImageRenderable = requires(T t) {
    { t.render() } -> std::same_as<void>;
};

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->imageList->setContextMenuPolicy(Qt::CustomContextMenu);

    setup_connections();
    SetDarkTheme();
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow* MainWindow::GetInstance()
{
    const QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget* w : topLevelWidgets)
        if (MainWindow* mainWin = qobject_cast<MainWindow*>(w)) return mainWin;
    qCritical() << "MainWindow::GetInstance returns nullptr";
    return nullptr;
}

void MainWindow::setup_connections()
{
    new QShortcut(QKeySequence(Qt::Key_F11), this, [this] {
        auto window_state = windowState();
        qDebug() << window_state;
        setWindowState(window_state == Qt::WindowFullScreen ? Qt::WindowMaximized : Qt::WindowFullScreen);
    });

    auto LoadImages = [this]<typename Viewer>
        requires(ImagesLoadable<Viewer> && ImageRenderable<Viewer>)
        (QString formats = tr(";;")) -> void {
            auto paths = QFileDialog::getOpenFileNames(this, tr("Open Images"), QDir::homePath(), formats);
            if (paths.isEmpty()) return;

            if (auto k = EncodePath(QFileInfo(paths[0]).absoluteDir().absolutePath()); m_images.contains(k))
            {
                SetTextToLogConsole("Image folder has been opened, please remove it from thumbnail list and reopen");
                return;
            }

            SetTextToLogConsole("Loading Images...");

            // TODO: use thread pool (QRunnable)
            auto* watcher = new QFutureWatcher<QList<QImage>>();
            connect(watcher, &QFutureWatcher<QList<QImage>>::finished, [this, watcher, paths]() {
                SetTextToLogConsole("Images Loaded");

                auto dir = QFileInfo(paths[0]).absoluteDir();
                auto dir_path = dir.absolutePath();
                auto const& imgs = watcher->result();
                if (auto k = EncodePath(dir_path); !m_images.contains(k))
                {
                    auto img = imgs[imgs.size() / 2];
                    if (img.isNull())
                    {
                        SetTextToLogConsole(QString("No valid series images in %1").arg(dir_path));
                        return;
                    }
                    createAndAppendItemToImageList(k, QPixmap::fromImage(img),
                                                   QString::fromStdString(dir.filesystemPath().filename().string()));

                    auto* viewer = new Viewer();
                    viewer->setImages(imgs, paths);
                    auto* sub_window = ui->mdiArea->addSubWindow(viewer);
                    sub_window->setWindowTitle(dir_path);
                    sub_window->setAttribute(Qt::WA_DeleteOnClose);
                    sub_window->setContentsMargins({0, 0, 0, 0});
                    sub_window->hide();
                }

                watcher->deleteLater();
            });
            watcher->setFuture(QtConcurrent::run([this, paths]() { return Viewer::loadImages(paths); }));
        };

    connect(ui->imageList, &QListWidget::customContextMenuRequested, [this](QPoint const& pos) {
        QPoint global_pos = ui->imageList->mapToGlobal(pos);
        auto* remove_action = new QAction(QIcon(":/ImageTool/UI_Icons/delete.png"), tr("remove"));
        QMenu menu;
        menu.addAction(remove_action);
        QAction* action = menu.exec(global_pos);
        if (action == remove_action)
        {
            auto* item = ui->imageList->takeItem(ui->imageList->indexAt(pos).row());
            auto k = item->data(Qt::UserRole).toString();
            auto title = DecodePath(k);
            for (auto* w : ui->mdiArea->subWindowList())
                if (w->windowTitle() == title)
                {
                    w->close();
                    w->deleteLater();
                }
            delete item;
            if (m_images.contains(k)) m_images.remove(k);
        }
    });

    connect(ui->actionOpenFile, &QAction::triggered, [this]() {
        static auto const supported_formats = QImageReader::supportedImageFormats();
        static QStringList formats(supported_formats.size());
        std::transform(supported_formats.cbegin(), supported_formats.cend(), formats.begin(),
                       [](auto const& f) { return QString("*.") + f; });

        auto path = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), formats.join("*;;"));
        if (path.isEmpty()) return;

        QFileInfo fi(path);
        if (auto k = EncodePath(path); !m_images.contains(k))
        {
            auto img = QImage(path);
            if (img.isNull())
            {
                SetTextToLogConsole(QString("No valid image in %1").arg(path));
                return;
            }
            createAndAppendItemToImageList(k, QPixmap::fromImage(img), fi.fileName());
        }
    });

    connect(ui->actionOpenFolder, &QAction::triggered, [this, LoadImages]() {
        static auto const supported_formats = QImageReader::supportedImageFormats();
        static QStringList formats(supported_formats.size());
        std::transform(supported_formats.cbegin(), supported_formats.cend(), formats.begin(),
                       [](auto const& f) { return QString("*.") + f; });
        LoadImages.template operator()<Viewer>(formats.join(";;"));
    });

    connect(ui->imageList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        auto k = item->data(Qt::UserRole).toString();
        auto title = DecodePath(k);
        for (auto* w : ui->mdiArea->subWindowList())
            if (w->windowTitle() == title)
            {
                ui->mdiArea->setActiveSubWindow(w);
                if (w->isHidden()) w->showMaximized();
                return;
            }
        auto* viewer = new Viewer();
        viewer->loadImage(QImage(title));
        auto* sub_window = ui->mdiArea->addSubWindow(viewer);
        sub_window->setWindowTitle(title);
        sub_window->setAttribute(Qt::WA_DeleteOnClose);
        sub_window->setContentsMargins({0, 0, 0, 0});
        sub_window->resize(450, 500);
        sub_window->show();
    });

    connect(ui->mdiArea, &QMdiArea::subWindowActivated, [this](QMdiSubWindow* w) {
        if (w == nullptr)
        {
            SetHist(QImage());
            SetPathInfo("");
            return;
        }

        auto path = w->windowTitle();
        SetPathInfo(path);

        const QSignalBlocker _(ui->imageList);
        if (auto items = ui->imageList->findItems(ui->info->getFileName(), Qt::MatchExactly); items.size() == 1)
            ui->imageList->setCurrentItem(items.first());
        else
        {
            for (auto* item : items)
                if (auto title = DecodePath(item->data(Qt::UserRole).toString()); title == path)
                    ui->imageList->setCurrentItem(item);
        }

        SetHist(static_cast<Viewer*>(w->widget())->getImage());
    });

    connect(ui->actionLight, &QAction::triggered, [this]() { SetLightTheme(); });

    connect(ui->actionDark, &QAction::triggered, [this]() { SetDarkTheme(); });

    connect(ui->actionSystemInfo, &QAction::triggered, [this]() { systemInfo(); });
}

void MainWindow::languageChanged(QString lang)
{
    if (!lang.isEmpty()) load_lang(lang);
}

void MainWindow::load_lang(QString const& lang)
{
    if (m_curr_lang != lang)
    {
        m_curr_lang = lang;
        QLocale locale = QLocale(m_curr_lang);
        QLocale::setDefault(locale);
        QString languageName = QLocale::languageToString(locale.language());
        switch_translator(m_translator, QString("ImageViewer_%1.qm").arg(lang));
        switch_translator(m_translatorQt, QString("qt_%1.qm").arg(lang));
        qDebug() << tr("Current Language changed from %1 to %2(%3)").arg(m_curr_lang).arg(lang).arg(languageName);
    }
}

void MainWindow::switch_translator(QTranslator& translator, QString const& filename)
{
    // remove the old translator
    qApp->removeTranslator(&translator);

    // load the new translator
    QString path = QApplication::applicationDirPath();
    path.append("/translations/");
    if (translator.load(path + filename)) qApp->installTranslator(&translator);
}

void MainWindow::createAndAppendItemToImageList(QString const& caseCachedDirName, QPixmap img, QString const& title)
{
    assert(!img.isNull());

    m_images[caseCachedDirName] = img.scaledToWidth(ui->imageList->width());
    auto* item = new QListWidgetItem(QIcon(m_images[caseCachedDirName]), title);
    item->setData(Qt::UserRole, caseCachedDirName);
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    ui->imageList->insertItem(ui->imageList->count(), item);
}

void MainWindow::updateImageListItem(QString const& oldK, QString const& newK, QPixmap img, QString const& title)
{
    assert(!img.isNull());

    m_images.remove(oldK);
    if (!m_images.contains(newK))
    {
        m_images[newK] = img.scaledToWidth(ui->imageList->width());
        for (auto i = 0; i < ui->imageList->count(); i++)
            if (ui->imageList->item(i)->data(Qt::UserRole).toString() == oldK)
            {
                auto* item = ui->imageList->item(i);
                item->setIcon(QIcon(m_images[newK]));
                item->setText(title);
                item->setData(Qt::UserRole, newK);
                break;
            }
    }
}

void MainWindow::updateImageListItem(QString const& k, QPixmap img)
{
    assert(!img.isNull());

    if (m_images.contains(k))
    {
        m_images[k] = img.scaledToWidth(ui->imageList->width());
        for (auto i = 0; i < ui->imageList->count(); i++)
            if (ui->imageList->item(i)->data(Qt::UserRole).toString() == k)
            {
                ui->imageList->item(i)->setIcon(QIcon(m_images[k]));
                break;
            }
    }
}

void MainWindow::systemInfo()
{
    if (m_sysinfo.isEmpty())
    {
        QProcess wmicProcess;
        wmicProcess.start("wmic", {"cpu", "get", "Name"});
        wmicProcess.waitForFinished();
        QString cpuInfo =
            "CPU Information:\nModel: " + wmicProcess.readAllStandardOutput().split('\n').at(1).trimmed() + "\n";

        QString gpuInfo = "GPU Information:\n";
        wmicProcess.start("wmic", {"path", "win32_VideoController", "get", "Name"});
        wmicProcess.waitForFinished();
        QString gpuResult = wmicProcess.readAllStandardOutput();
        QStringList gpuList = gpuResult.split("\n", Qt::SkipEmptyParts);
        for (int i = 1; i < gpuList.size(); i++)
        {
            QString gpuName = gpuList.at(i).trimmed();
            if (!gpuName.isEmpty()) gpuInfo += "GPU " + QString::number(i) + ": " + gpuName + "\n";
        }
        m_sysinfo = cpuInfo + "\n" + gpuInfo;
    }

    QMessageBox::about(this, tr("System Info"), m_sysinfo);
}

void MainWindow::SetTextToLogConsole(QString const& text, bool isErrMsg)
{
    ui->output_log->clear();
    QString style = isErrMsg ? "font-weight:bold;color:red" : "";
    ui->output_log->appendHtml("<p style=\"" + style + ";white-space:pre\">" + text + "</p>");
}

void MainWindow::AppendTextToLogConsole(QString const& text, bool isErrMsg)
{
    QString style = isErrMsg ? "font-weight:bold;color:red" : "";
    ui->output_log->appendHtml("<p style=\"" + style + ";white-space:pre\">" + text + "</p>");
    ui->output_log->verticalScrollBar()->setValue(ui->output_log->verticalScrollBar()->maximum());
}

void MainWindow::ClearLogConsole()
{
    ui->output_log->clear();
}

void MainWindow::SetDarkTheme()
{
    SetTheme(":qdarkstyle/dark/darkstyle.qss");
    ui->hist->setDarkTheme();
}

void MainWindow::SetLightTheme()
{
    SetTheme(":qdarkstyle/light/lightstyle.qss");
    ui->hist->setLightTheme();
}

void MainWindow::SetTheme(QString const& qssFilePath)
{
    if (qssFilePath == m_theme_qss_path) return;
    QFile f(qssFilePath);
    if (!f.exists())
        qCritical() << "Unable to set stylesheet, file not found";
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
        m_theme_qss_path = qssFilePath;
    }
}

void MainWindow::SetHist(QImage const& img)
{
    ui->hist->setImage(img);
}

void MainWindow::SetPathInfo(QString const& path)
{
    ui->info->setPath(path);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event && event->type() == QEvent::LanguageChange) ui->retranslateUi(this);
    QMainWindow::changeEvent(event);
}