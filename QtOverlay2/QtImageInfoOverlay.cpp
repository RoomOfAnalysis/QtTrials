#include "QtImageInfoOverlay.h"
#include "ui_QtImageInfoOverlay.h"

#include <QStyleOption>
#include <QPainter>
#include <QWheelEvent>

KVItem::KVItem(QWidget* parent): QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(9, 0, 9, 0);
    layout->setSpacing(0);
    layout->addWidget(&m_key_label);
    layout->addWidget(&m_value_label);
    setLayout(layout);

    m_key_label.setFixedSize(120, 30);
    m_value_label.setFixedSize(142, 30);

    m_value_label.setContentsMargins(3, 0, 0, 0);
    m_value_label.setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_value_label.setCursor(Qt::IBeamCursor);
}

void KVItem::setKV(QString key, QString value)
{
    m_key_label.setText(key);
    m_value_label.setText(value);
};

void KVItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QtImageInfoOverlay::QtImageInfoOverlay(QtOverlayWidgetContainer* parent)
    : QtOverlayWidget(parent), ui(new Ui::QtImageInfoOverlay)
{
    ui->setupUi(this);
    m_item_stub.setFixedSize(280, 48);
    m_item_stub.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    connect(ui->closeButton, &QPushButton::clicked, this, &QtImageInfoOverlay::hide);
    this->setPosition(QtOverlayWidgetPosition::RIGHT);

    if (parent) setContainerSize(parent->size());
}

QtImageInfoOverlay::~QtImageInfoOverlay()
{
    delete ui;
    for (auto i = m_items.count() - 1; i >= 0; i--)
        delete m_items.takeAt(i);
}

void QtImageInfoOverlay::setExifInfo(QMap<QString, QString> const& info)
{
    // remove/add entries
    int items_count = m_items.count();
    int info_count = info.count();
    if (items_count > info_count)
    {
        for (auto i = items_count - 1; i >= info_count; i--)
        {
            ui->itemsLayout->removeWidget(m_items.last());
            delete m_items.takeLast();
        }
    }
    else if (items_count < info_count)
    {
        for (auto i = items_count; i < info_count; i++)
        {
            m_items.append(new KVItem(this));
            ui->itemsLayout->addWidget(m_items.last());
        }
    }

    int item_idx = 0;
    for (auto i = info.constBegin(); i != info.constEnd(); i++)
        m_items.at(item_idx++)->setKV(i.key(), i.value());

    if (m_items.count())
    {
        ui->itemsLayout->removeWidget(&m_item_stub);
        m_item_stub.setText("");
    }
    else
    {
        ui->itemsLayout->addWidget(&m_item_stub);
        m_item_stub.setText("<no metadata found>");
    }

    if (!isHidden() && items_count != info.count())
    {
        // wait for layout change
        qApp->processEvents();
        recalculateGeometry();
    }
}

void QtImageInfoOverlay::show()
{
    QtOverlayWidget::show();
    adjustSize();
    recalculateGeometry();
}

void QtImageInfoOverlay::wheelEvent(QWheelEvent* event)
{
    event->accept();
}