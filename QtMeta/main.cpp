#include <QCoreApplication>
#include <QMetaType>
#include <QMetaProperty>

class MetaTest: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int Val READ getVal WRITE setVal)
public:
    int getVal() const { return m_val; }
    void setVal(int val) { m_val = val; }

    Q_INVOKABLE void test_print(QString const& str) { qDebug() << str.toLatin1().data(); }

    enum TestEnum
    {
        Zero,
        One,
        Two
    };
    Q_ENUM(TestEnum)

private:
    int m_val;
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    MetaTest mtObj;

    // meta object
    // https://doc.qt.io/qt-6/qmetaobject.html
    QMetaObject const* metaObjectFromStatic = &MetaTest::staticMetaObject;
    QMetaObject const* metaObjectFromVirtual = mtObj.metaObject();

    // property
    // https://doc.qt.io/qt-6/properties.html
    mtObj.setProperty("Val", QVariant::fromValue<int>(2));
    auto val = mtObj.property("Val");
    qDebug() << "Property:" << val.value<int>();

    // invoke member function
    qDebug() << "Invoke Method Begin:";
    metaObjectFromStatic->invokeMethod(&mtObj, "test_print", Q_ARG(QString, "print testing"));
    qDebug() << "Invoke Method End";

    // meta type
    QMetaType metaType = val.metaType();
    qDebug() << "Meta Type:" << metaType.name() << " ID:" << metaType.id();

    // traverse meta object property
    for (auto i = metaObjectFromStatic->propertyOffset(); i < metaObjectFromStatic->propertyCount(); i++)
    {
        QMetaProperty const& metaProperty = metaObjectFromStatic->property(i);
        metaProperty.write(&mtObj, QVariant::fromValue<int>(3));
        QVariant valFromMetaObject = metaProperty.read(&mtObj);
        qDebug() << "Meta Property:" << metaProperty.name() << metaProperty.typeName() << metaProperty.typeId()
                 << valFromMetaObject;
    }

    // traverse meta object method
    for (auto i = metaObjectFromStatic->methodOffset(); i < metaObjectFromStatic->methodCount(); i++)
    {
        QMetaMethod const& metaMethod = metaObjectFromStatic->method(i);
        QByteArrayList paramNames = metaMethod.parameterNames();
        QStringList paramsDefineList;
        for (auto j = 0; j < metaMethod.parameterCount(); j++)
            paramsDefineList << metaMethod.parameterTypeName(j) + " " + paramNames[j];
        qDebug() << QString("Meta Method : %1 %2(%3)")
                        .arg(metaMethod.typeName())
                        .arg(metaMethod.name())
                        .arg(paramsDefineList.join(','))
                        .toLatin1()
                        .data();
    }

    // traverse meta object enum
    for (auto i = metaObjectFromStatic->enumeratorOffset(); i < metaObjectFromStatic->enumeratorCount(); i++)
    {
        QMetaEnum const& metaEnum = metaObjectFromStatic->enumerator(i);
        qDebug() << "Meta Enum:" << metaEnum.name();
        for (auto j = 0; j < metaEnum.keyCount(); j++)
            qDebug() << "--" << metaEnum.key(j) << ":" << metaEnum.value(j);
    }

    return app.exec();
}

// to enable moc compilation on .cpp file
#include "main.moc"