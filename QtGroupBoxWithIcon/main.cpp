#include <QApplication>

#include <QMainWindow>
#include <QBoxLayout>
#include <QRadioButton>

#include "DecoratedGroupBox.h"

class MainWindow: public QMainWindow
{
public:
    explicit MainWindow(QWidget* parent = nullptr): QMainWindow(parent)
    {
        auto* groupBox = new DecoratedGroupBox(tr("Test"), this);
        auto* l = new QVBoxLayout(groupBox);
        auto* radio1 = new QRadioButton(tr("Icon 1"), this);
        auto* radio2 = new QRadioButton(tr("Icon 2"), this);
        auto* radio3 = new QRadioButton(tr("Icon 3"), this);

        radio1->setChecked(true);

        l->addWidget(radio1);
        l->addWidget(radio2);
        l->addWidget(radio3);

        groupBox->setPixmap(QPixmap(":/images/home.png"));
        //	groupBox->setOffset(27);
        //	groupBox->setSpaces(15, 15);

        setCentralWidget(groupBox);
        setContentsMargins(12, 12, 12, 12);
        resize(300, 200);

        connect(radio1, &QRadioButton::toggled, [groupBox](bool checked) {
            if (checked) groupBox->setPixmap(QPixmap(":/images/home.png"));
        });

        connect(radio2, &QRadioButton::toggled, [groupBox](bool checked) {
            if (checked) groupBox->setPixmap(QPixmap(":/images/next.png"));
        });

        connect(radio3, &QRadioButton::toggled, [groupBox](bool checked) {
            if (checked) groupBox->setPixmap(QPixmap(":/images/export.png"));
        });
    }
};

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}