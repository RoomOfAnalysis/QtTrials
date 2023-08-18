#include "Models.h"

#include <QScreen>

ScreenListModel::ScreenListModel(QList<QScreen*> screen_list, QObject* parent)
    : QAbstractListModel(parent), m_screen_list(screen_list)
{
}

int ScreenListModel::rowCount(const QModelIndex& parent) const
{
    return m_screen_list.count();
}

QVariant ScreenListModel::data(const QModelIndex& index, int role) const
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(index.row() <= m_screen_list.count());

    if (role == Qt::DisplayRole)
    {
        auto screen = m_screen_list.at(index.row());
        return QString("%1: %2").arg(screen->metaObject()->className(), screen->name());
    }
    return {};
}

QScreen* ScreenListModel::screen(const QModelIndex& index) const
{
    return m_screen_list.at(index.row());
}