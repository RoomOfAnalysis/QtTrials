#include <inja/inja.hpp>
#include <QtWidgets/QApplication>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QUrl>
#include <QPageLayout>
#include <QMessageBox>

// 1) template file; 2) media folder; 3) output pdf file
int main(int argc, char* argv[])
{
    inja::Environment env;
    auto tmp = env.parse_template(argv[1]);

    inja::json data;
    data["user"]["name"] = "Harold";
    data["media_dir"] = argv[2];
    auto html = env.render(tmp, data);

    QApplication app(argc, argv);
    qputenv("QTWEBENGINEPROCESS_PATH",
            QString("C:/vcpkg/installed/x64-windows/tools/Qt6/bin/QtWebEngineProcess.exe").toLocal8Bit());
    qputenv("QTWEBENGINE_RESOURCES_PATH", QString("C:/vcpkg/installed/x64-windows/resources").toLocal8Bit());
    qputenv("QTWEBENGINE_LOCALES_PATH",
            QString("C:/vcpkg/installed/x64-windows/translations/Qt6/qtwebengine_locales").toLocal8Bit());
    QWebEngineView web;
    auto url = QUrl::fromLocalFile(argv[2]);
    web.setHtml(html.c_str(), url);
    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF());
    QMessageBox msgBox;
    msgBox.setText("Pdf Generating...");
    QObject::connect(&web, &QWebEngineView::loadFinished, [&]() {
        web.page()->printToPdf(argv[3], layout);
        msgBox.setText("Pdf Generated!");
    });
    //web.show();

    return msgBox.exec();
}