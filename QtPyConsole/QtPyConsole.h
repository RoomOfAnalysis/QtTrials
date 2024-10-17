#pragma once

// code from https://github.com/roozbehg/QPyConsole

#include <QPlainTextEdit>

class QtPyConsole: public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit QtPyConsole(QWidget* parent = nullptr);
    ~QtPyConsole();
    void setPrefix(const QString& prefix);
    void setPrefixColor(const QColor& color);
    void setPrefixFont(const QFont& font);
    QString prefix() const { return mPrefix; }
    QColor prefixColor() const { return mPrefixColor; }
    QFont prefixFont() const { return mPrefixFont; }

signals:
    void command(QString cmd);

public slots:
    void print(QString str);
    void prepareCommandLine();

protected:
    bool inCommandLine() const;
    void processCommand();
    virtual void keyPressEvent(QKeyEvent* event);

    int mHistoryPos{-1};
    QStringList mHistory{};
    QString mPrefix{};
    QColor mPrefixColor{};
    QFont mPrefixFont{};
    bool mCommandLineReady{false};
};