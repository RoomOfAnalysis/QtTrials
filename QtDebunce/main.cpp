#include <QApplication>
#include <QElapsedTimer>
#include <QTimer>
#include <QMessageBox>
#include <QAbstractButton>

//#define USE_EVENT_FILTER

#ifndef USE_EVENT_FILTER
#include "QtDebouncer.h"
#endif

// debounce through event filter
class mApp: public QApplication
{
public:
    mApp(int argc, char** argv): QApplication(argc, argv) {}

    void enableDebounced(bool flag = true, int msec = 20)
    {
        m_bounced = flag;
        m_bt_msec = msec;
        if (flag)
            installEventFilter(this);
        else
            removeEventFilter(this);
    }

private:
    bool eventFilter(QObject* o, QEvent* e)
    {
        static int pressed = 0;
        switch (e->type())
        {
        case QEvent::MouseButtonPress:
            qDebug() << "mApp:" << ++pressed;
            if (m_timer.elapsed() < m_bt_msec) return true;
            break;
        case QEvent::MouseButtonRelease:
            m_timer.restart();
            break;
        default:
            break;
        }
        return QApplication::eventFilter(o, e);
    }

private:
    QElapsedTimer m_timer{};
    bool m_bounced{false};
    int m_bt_msec = 20;
};

int main(int argc, char* argv[])
{
#ifdef USE_EVENT_FILTER
    mApp app(argc, argv);
    app.enableDebounced(true, 200);
    // disable debounce after 10s
    QTimer::singleShot(10000, [&app]() { app.enableDebounced(false); });
#else
    QApplication app(argc, argv);
    QtDebouncer debouncer;
    debouncer.setTimeout(200);
#endif

    QMessageBox msgBox;
    msgBox.setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    msgBox.setText("Debounce Demo");
    msgBox.setInformativeText("Please click the buttons");
    msgBox.setStandardButtons(QMessageBox::Ok);
    auto* btn = msgBox.button(QMessageBox::Ok);
    QObject::disconnect(btn, &QAbstractButton::clicked, nullptr, nullptr);

#ifdef USE_EVENT_FILTER
    QObject::connect(btn, &QAbstractButton::clicked, []() {
        static int pressed = 0;
        qDebug() << ++pressed;
    });
#else
    QObject::connect(btn, &QAbstractButton::clicked, &debouncer, &QtDebouncer::debounce);
    QObject::connect(&debouncer, &QtDebouncer::triggered, []() {
        static int pressed = 0;
        qDebug() << ++pressed;
    });
#endif
    msgBox.show();

    return app.exec();
}