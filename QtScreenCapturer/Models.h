#pragma once

#include <QAbstractListModel>

class QScreen;

class ScreenListModel: public QAbstractListModel
{
    Q_OBJECT
public:
    ScreenListModel(QList<QScreen*> screen_list = QList<QScreen*>{}, QObject* parent = nullptr);
    ~ScreenListModel() = default;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QScreen* screen(const QModelIndex& index) const;

private:
    QList<QScreen*> m_screen_list{};
};