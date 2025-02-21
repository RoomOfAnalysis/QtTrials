#include "QtUtils.h"

#include <QApplication>
#include <QTextCodec>
#include <QFontDatabase>
#include <QDebug>
#include <QScreen>
#include <QWidget>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#endif

QString QtUtils::compilerInfo()
{
#if defined(Q_CC_CLANG)
    QString isAppleString = QLatin1String();
#if defined(__apple_build_version__)
    isAppleString = QLatin1String(" (Apple)");
#endif
    return QLatin1String("Clang ") + QString::number(__clang_major__) + QLatin1Char('.') +
           QString::number(__clang_minor__) + isAppleString;
#elif defined(Q_CC_GNU)
    return QLatin1String("GCC ") + QLatin1String(__VERSION__);
#elif defined(Q_CC_MSVC)
    return QString("MSVC Version: %1").arg(_MSC_VER);
#endif
    return QLatin1String("<unknown compiler>");
}

QString QtUtils::systemInfo()
{
    return QString("%1 (%2) on %3 (%4) with CPU Cores: %5")
               .arg(QSysInfo::prettyProductName(), QSysInfo::kernelVersion(), QSysInfo::currentCpuArchitecture(),
                    QSysInfo::machineHostName(), QString::number(QThread::idealThreadCount())) +
           "\n" + QString("Build with: Qt %1 (%2, %3)").arg(qVersion(), compilerInfo(), QSysInfo::buildAbi());
}

bool QtUtils::isMainThread()
{
    return QThread::currentThread() == qApp->thread();
}

void QtUtils::setUTF8Codec()
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
}

void QtUtils::loadFonts(QStringList const& paths)
{
    // QFontDatabase::removeAllApplicationFonts();
    for (auto const& p : std::as_const(paths))
        // frome Qt6.8 DOC: Currently only TrueType fonts, TrueType font collections, and OpenType fonts are supported.
        if (auto id = QFontDatabase::addApplicationFont(p); id == -1)
            qWarning() << QString("Loading Font file: %1 Failed.").arg(p);
        else
            qInfo() << QString("Loaded Font file: %1.").arg(p) << QFontDatabase::applicationFontFamilies(id);
}

void QtUtils::loadQSS(QStringList const& paths)
{
    QStringList qss;
    for (auto const& p : std::as_const(paths))
    {
        QFile file(p);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << QString("Failed to open the QSS file: %1").arg(p) << file.errorString();
            continue;
        }
        qInfo() << QString("Loaded QSS: %1").arg(p);
        qss.append(QLatin1String(file.readAll()));
        file.close();
    }
    if (qss.isEmpty()) return;
    qApp->setStyleSheet(qss.join("\n"));
}

QRect QtUtils::desktopRect()
{
    QRect geometry;
    auto screens = QGuiApplication::screens();
    for (auto* const screen : std::as_const(screens))
    {
        QRect scrRect = screen->geometry();
        scrRect.moveTo(scrRect.x() / screen->devicePixelRatio(), scrRect.y() / screen->devicePixelRatio());
        geometry = geometry.united(scrRect);
    }
    return geometry;
}

QPixmap QtUtils::grabDefaultScreen()
{
    return QApplication::primaryScreen()->grabWindow();
}

QPixmap QtUtils::grabCombinedScreens()
{
    auto rect = desktopRect();
    return QApplication::primaryScreen()->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
}

void QtUtils::moveToCenter(QWidget* w)
{
    auto const rect = QApplication::primaryScreen()->availableGeometry();
    auto x = qMax(0, (rect.width() - w->width()) / 2 + rect.x());
    auto y = qMax(0, (rect.height() - w->height()) / 2 + rect.y());
    w->move(x, y);
}

void QtUtils::moveToCenter(QWidget* child, QWidget* parent)
{
    auto const size = parent->size() - child->size();
    auto x = qMax(0, size.width() / 2);
    auto y = qMax(0, size.height() / 2);
    child->move(x, y);
}

void QtUtils::quitApp()
{
    QMetaObject::invokeMethod(qApp, &QApplication::quit, Qt::QueuedConnection);
}

void QtUtils::rebootApp()
{
    QProcess::startDetached(QApplication::applicationFilePath(), QApplication::arguments(), QDir::currentPath());
    QApplication::exit();
}

qint64 QtUtils::getPidByProcessName(QString const& process_name)
{
#if defined(Q_OS_WIN)
    auto* hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return -1;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnapshot, &pe))
    {
        CloseHandle(hSnapshot);
        return -1;
    }
    do
    {
        if (QString::fromWCharArray(pe.szExeFile) == process_name)
        {
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe));
    CloseHandle(hSnapshot);
    return -1;
#else
    QProcess process;
    process.start("ps -A");
    process.waitForFinished();
    auto lines = QString::fromLocal8Bit(process.readAllStandardOutput()).split("\n");
    QRegularExpression re("\\s" + process_name + "$");
    for (const auto& line : lines)
    {
        if (re.match(line).hasMatch())
        {
            auto parts = line.split(" ");
            return parts.at(0).toLongLong();
        }
    }
    return -1;
#endif
}

bool QtUtils::killProcess(qint64 pid)
{
    qInfo() << "kill process: " << pid;
#if defined(Q_OS_WIN)
    auto* hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == nullptr) return false;
    auto result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result != 0;
#else
    return QProcess::execute("kill", QStringList() << QString::number(pid)) == 0;
#endif
}
