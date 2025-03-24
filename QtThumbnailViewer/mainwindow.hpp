#pragma once

#include <QApplication>
#include <QMainWindow>
#include <QTranslator>
#include <QMap>

namespace Ui
{
    class MainWindow;
}

class MainWindow final: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    MainWindow(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;
    ~MainWindow() override;

    static [[nodiscard]] MainWindow* GetInstance();

    void SetTextToLogConsole(QString const& text, bool isErrMsg = false);
    void AppendTextToLogConsole(QString const& text, bool isErrMsg = false);
    void ClearLogConsole();

    void SetDarkTheme();
    void SetLightTheme();
    void SetTheme(QString const& qssFilePath);

    void SetHist(QImage const& img);
    void SetPathInfo(QString const& path);

public slots:
    /// @brief slot for language switching
    void languageChanged(QString lan);

protected:
    /// @brief handle language change
    void changeEvent(QEvent*) override;

private:
    void setup_connections();
    /// @brief load translation (`.qm` file)
    void load_lang(QString const& lang);
    void switch_translator(QTranslator& translator, QString const& filename);
    void createAndAppendItemToImageList(QString const& caseCachedDirName, QPixmap img, QString const& title);
    void updateImageListItem(QString const& oldK, QString const& newK, QPixmap img, QString const& title);
    void updateImageListItem(QString const& k, QPixmap img);
    void systemInfo();

private:
    Ui::MainWindow* ui = nullptr;
    QMap<QString, QPixmap> m_images{};

    QString m_curr_lang{};
    QTranslator m_translator{};
    QTranslator m_translatorQt{};

    QString m_theme_qss_path{};
    QString m_sysinfo{};
};