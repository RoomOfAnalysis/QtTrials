#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCoreApplication>

#include <sqlite3.h>

#include <exception>

int main(int argc, char* argv[])
{
    qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));

    qDebug() << SQLITE_VERSION;

    QCoreApplication app(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setDatabaseName("test.db");

    if (!db.open()) throw std::exception("db not open");

    QSqlQuery query;
    query.exec("create table person "
               "(id integer primary key, "
               "firstname varchar(20), "
               "lastname varchar(30), "
               "age integer)");

    return app.exec();
}