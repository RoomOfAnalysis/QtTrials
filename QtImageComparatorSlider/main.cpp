#include <QApplication>
#include <QMainWindow>
#include <QSplitter>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

class QtImageLabel: public QLabel
{
public:
    explicit QtImageLabel(QWidget* parent = nullptr): QLabel(parent)
    {
        setMinimumSize(1, 1);
        setScaledContents(false);
    }
    ~QtImageLabel() = default;

    void setPixmap(QPixmap const& p)
    {
        m_pix = p;
        QLabel::setPixmap(p);
    }

    void setLeft(bool is_left) { m_is_left = is_left; }
public slots:
    void resizeEvent(QResizeEvent* ev) override
    {
        if (m_is_left)
            QLabel::setPixmap(m_pix.copy(0, 0, ev->size().width(), ev->size().height()));
        else
            QLabel::setPixmap(
                m_pix.copy(m_pix.width() - ev->size().width(), 0, ev->size().width(), ev->size().height()));
    }

private:
    QPixmap m_pix;
    bool m_is_left = true;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto mw = new QMainWindow();

    auto img1 = new QPixmap(argv[1]);
    auto img2 = new QPixmap(argv[2]);

    auto label1 = new QtImageLabel();
    auto label2 = new QtImageLabel();

    label1->setPixmap(*img1);
    label2->setPixmap(*img2);
    label2->setLeft(false);

    label1->setStyleSheet("background-color: red");
    label2->setStyleSheet("background-color: green");

    auto splitter = new QSplitter();
    splitter->setOrientation(Qt::Orientation::Horizontal);
    splitter->addWidget(label1);
    splitter->addWidget(label2);

    QObject::connect(splitter, &QSplitter::splitterMoved, [=](int pos, int index) {
        // qDebug() << pos << index;
        auto sz = splitter->sizes();
        qDebug() << sz;
    });

    mw->setCentralWidget(splitter);
    // TODO: resizable
    mw->setFixedSize(img1->size());
    mw->show();

    return app.exec();
}