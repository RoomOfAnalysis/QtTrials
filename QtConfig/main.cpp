#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QXmlStreamReader>
#include <QStringList>
#include <QString>
#include <QStringView>

constexpr auto ini_config_file_path = "config.ini";

static bool readXML(QIODevice& device, QSettings::SettingsMap& map)
{
    // https://doc.qt.io/qt-6/qxmlstreamreader.html
    QXmlStreamReader reader(&device);
    QStringList elements;
    while (!reader.atEnd() && !reader.hasError())
    {
        reader.readNext();
        if (reader.isStartElement() && reader.name() != QString("Settings"))
            elements.append(reader.name().toString());
        else if (reader.isEndElement())
            if (!elements.isEmpty())
                elements.removeLast();
            else if (reader.isCharacters() && !reader.isWhitespace())
            {
                QString key;
                for (int i = 0; i < elements.size(); i++)
                {
                    if (i != 0) key += "/";
                    key += elements.at(i);
                }
                map[key] = reader.text().toString();
            }
    }
    if (reader.hasError())
    {
        qDebug() << reader.errorString();
        return false;
    }
    return true;
}

static bool writeXML(QIODevice& device, QSettings::SettingsMap const& map)
{
    // https://doc.qt.io/qt-6/qxmlstreamwriter.html
    QXmlStreamWriter writer(&device);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("Settings");
    QStringList prev;
    for (auto it = map.begin(); it != map.end(); it++)
    {
        QStringList elements = it.key().split("/");
        int x = 0;
        while (x < prev.size() && elements.at(x) == prev.at(x))
            x++;
        for (int i = prev.size() - 1; i >= x; i--)
            writer.writeEndElement();
        for (int i = x; i < elements.size(); i++)
            writer.writeStartElement(elements.at(i));
        writer.writeCharacters(it.value().toString());
        prev = elements;
    }
    for (int i = 0; i < prev.size(); i++)
        writer.writeEndElement();
    writer.writeEndElement();
    writer.writeEndDocument();
    return true;
}

int main()
{
    // ini
    auto ini_path = QDir(QCoreApplication::applicationDirPath()).filePath(ini_config_file_path);
    auto ini_config_file = QFile(ini_path);
    ini_config_file.open(QIODevice::ReadWrite); // will create if not exist
    ini_config_file.close();

    // https://doc.qt.io/qt-6/qsettings.html
    QSettings ini_w(ini_path, QSettings::IniFormat);
    ini_w.setValue("test/key", "key");
    ini_w.setValue("test/value", 123);

    QSettings ini_r(ini_path, QSettings::IniFormat);
    auto key = ini_r.value("test/key", "default_key_if_unset").toString();
    auto value = ini_r.value("test/value", 321).toInt();
    qDebug() << (key == "key") << (value == 123);

    // xml
    // https://doc.qt.io/qt-6/qsettings.html#setPath
    auto const xml_format = QSettings::registerFormat("xml", readXML, writeXML);
    QSettings::setPath(xml_format, QSettings::UserScope, ".");
    QSettings xml_w(xml_format, QSettings::UserScope, "Org", "config");
    xml_w.setValue("test/key", "key");
    xml_w.setValue("test/value", 123);

    QSettings xml_r(xml_format, QSettings::UserScope, "Org", "config");
    key = xml_r.value("test/key", "default_key_if_unset").toString();
    value = xml_r.value("test/value", 321).toInt();
    qDebug() << (key == "key") << (value == 123);

    return 0;
}