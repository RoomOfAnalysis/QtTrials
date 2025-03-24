#pragma once

#include <QWidget>
#include <QLabel>
#include <QDateTime>
#include <QFileInfo>

#include <QDomNode>

class QTextBrowser;

class KVItem: public QWidget
{
    Q_OBJECT
public:
    explicit KVItem(QWidget* parent = nullptr);
    void setKV(QString key, QString value);

protected:
    void paintEvent(QPaintEvent* event);

private:
    QLabel m_key_label{}, m_value_label{};
};

class FileInfoViewer: public QWidget
{
    Q_OBJECT
public:
    explicit FileInfoViewer(QWidget* parent = nullptr);
    ~FileInfoViewer();

    void setPath(QString const& path);
    QString getFileName() const;

    void setExtra(QString const& xml_str);
    void setExtra(int series_no);

private:
    QString parse_node(QDomNode node, int indent = 0);
    std::pair<QSize, int> getImageSizeAndDepth(QString const& path);

private:
    QFileInfo m_fi{};
    KVItem* m_path_item = nullptr;
    KVItem* m_isdir_item = nullptr;
    KVItem* m_size_item = nullptr;
    KVItem* m_lastmodified_item = nullptr;
    KVItem* m_img_sz_item = nullptr;
    KVItem* m_img_bd_item = nullptr;
    QTextBrowser* m_extra = nullptr;

    QString m_xml_str{};
    QList<QString> m_instrument_strs{};
    int m_series_no = -1;
};