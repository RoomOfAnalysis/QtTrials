#include "QtPyConsole.h"

#include <QApplication>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextCursor>
#include <QClipboard>

QtPyConsole::QtPyConsole(QWidget* parent): QPlainTextEdit(parent)
{
    QPalette p = palette();
    p.setColor(QPalette::Active, QPalette::Base, Qt::black);
    p.setColor(QPalette::Inactive, QPalette::Base, Qt::black);
    p.setColor(QPalette::Active, QPalette::Text, Qt::white);
    p.setColor(QPalette::Inactive, QPalette::Text, Qt::white);
    setPalette(p);

    mPrefixFont.setFamily("Lucida Console");
    mPrefixFont.setPointSize(10);
    mPrefixFont.setFixedPitch(true);
    mPrefixFont.setBold(true);
    setFont(mPrefixFont);
    setPrefixFont(mPrefixFont);

    auto charWidth = QFontMetrics(font()).horizontalAdvance(QChar('x'));

    setCursorWidth(charWidth);
    setTabStopDistance(charWidth * 4);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setUndoRedoEnabled(false);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setWordWrapMode(QTextOption::NoWrap);
    setBackgroundVisible(false);
    setFrameStyle(QFrame::NoFrame);
    setAcceptDrops(false);
    setPrefix(">>> ");
    setPrefixColor(QColor(140, 255, 50));

    //prepareCommandLine();
}

QtPyConsole::~QtPyConsole() = default;

void QtPyConsole::setPrefix(const QString& prefix)
{
    mPrefix = prefix;
}

void QtPyConsole::setPrefixColor(const QColor& color)
{
    mPrefixColor = color;
}

void QtPyConsole::setPrefixFont(const QFont& font)
{
    mPrefixFont = font;
}

void QtPyConsole::print(QString str)
{
    QTextCursor cur(document()->lastBlock());
    cur.setCharFormat(QTextCharFormat());
    cur.insertText(str);
    cur.movePosition(QTextCursor::EndOfBlock);
    cur.insertBlock();
    setTextCursor(cur);
    mCommandLineReady = false;
}

void QtPyConsole::prepareCommandLine()
{
    QTextCursor cur(document()->lastBlock());
    if (!document()->lastBlock().text().isEmpty())
    {
        cur.movePosition(QTextCursor::EndOfBlock);
        cur.insertBlock();
    }
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(mPrefixColor));
    fmt.setFont(mPrefixFont);
    cur.setCharFormat(fmt);
    cur.insertText(mPrefix);
    cur.setCharFormat(QTextCharFormat());
    setTextCursor(cur);
    mCommandLineReady = true;
}

bool QtPyConsole::inCommandLine() const
{
    return mCommandLineReady && document()->blockCount() - 1 == textCursor().blockNumber() &&
           textCursor().positionInBlock() >= mPrefix.length();
}

void QtPyConsole::processCommand()
{
    QString inputString = document()->lastBlock().text();
    if (inputString.startsWith(mPrefix)) inputString.remove(0, mPrefix.length());
    if (!inputString.trimmed().isEmpty()) mHistory.append(inputString);
    mHistoryPos = -1;

    QTextCursor cur(document()->lastBlock());
    cur.movePosition(QTextCursor::EndOfBlock);
    cur.insertBlock();
    setTextCursor(cur);
    mCommandLineReady = false;
    emit command(inputString);
}

void QtPyConsole::keyPressEvent(QKeyEvent* event)
{
    if (inCommandLine())
    {
        // clear selection that spans multiple blocks (or prefix characters) (would overwrite previous command lines):
        QTextCursor cur = textCursor();
        if (cur.hasSelection())
        {
            if (document()->findBlock(cur.selectionStart()) !=
                    document()->findBlock(cur.selectionEnd()) || // spans multiple blocks (including command line)
                cur.selectionStart() - cur.block().position() < mPrefix.length() || // spans prefix
                cur.selectionEnd() - cur.block().position() < mPrefix.length())
            { // spans prefix

                cur.clearSelection();
                if (cur.positionInBlock() < mPrefix.length())
                    cur.setPosition(cur.block().position() + mPrefix.length());
                setTextCursor(cur);
            }
        }
        if (cur.positionInBlock() == mPrefix.length())
        {
            cur.setCharFormat(QTextCharFormat()); // make sure we don't pick up format from prefix
            setTextCursor(cur);
        }

        // react to keystroke:
        if (event->matches(QKeySequence::MoveToPreviousLine))
        { // history up

            if (mHistory.isEmpty() || mHistoryPos >= mHistory.size() - 1) return;
            ++mHistoryPos;
            int index = mHistory.size() - mHistoryPos - 1;
            QTextCursor cur(document()->lastBlock());
            cur.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, mPrefix.length());
            cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cur.removeSelectedText();
            cur.setCharFormat(QTextCharFormat());
            cur.insertText(mHistory.at(index));
            setTextCursor(cur);
        }
        else if (event->matches(QKeySequence::MoveToNextLine))
        { // history down

            if (mHistory.isEmpty() || mHistoryPos <= 0) return;
            --mHistoryPos;
            int index = mHistory.size() - mHistoryPos - 1;
            QTextCursor cur(document()->lastBlock());
            cur.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, mPrefix.length());
            cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cur.removeSelectedText();
            cur.setCharFormat(QTextCharFormat());
            cur.insertText(mHistory.at(index));
            setTextCursor(cur);
        }
        else if (event->matches(QKeySequence::Paste))
        { // paste text, do it manually to remove text char formatting and newlines
            QString pasteText = QApplication::clipboard()->text();
            pasteText.replace("\n", "").replace("\r", "");
            cur.setCharFormat(QTextCharFormat());
            cur.insertText(pasteText);
            setTextCursor(cur);
        }
        else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        { // process command
            processCommand();
        }
        else if (event->key() == Qt::Key_Backspace)
        { // only allow backspace if we wouldn't delete last char of prefix
            if (cur.positionInBlock() > mPrefix.length()) QPlainTextEdit::keyPressEvent(event);
        }
        else if (!event->matches(QKeySequence::Close) && !event->matches(QKeySequence::New) &&
                 !event->matches(QKeySequence::Open) && !event->matches(QKeySequence::Preferences) &&
                 !event->matches(QKeySequence::Bold) && !event->matches(QKeySequence::Italic) &&
                 !event->matches(QKeySequence::InsertLineSeparator) &&
                 !event->matches(QKeySequence::InsertParagraphSeparator) && !event->matches(QKeySequence::Redo) &&
                 !event->matches(QKeySequence::Undo) && !event->matches(QKeySequence::DeleteStartOfWord))
        {
            QPlainTextEdit::keyPressEvent(event);
        }
    }
    else
    { // cursor position not in command line
        if (event->matches(QKeySequence::MoveToEndOfDocument) || event->matches(QKeySequence::MoveToEndOfBlock) ||
            event->matches(QKeySequence::MoveToEndOfLine) || event->matches(QKeySequence::MoveToStartOfDocument) ||
            event->matches(QKeySequence::MoveToStartOfBlock) || event->matches(QKeySequence::MoveToStartOfLine) ||
            event->matches(QKeySequence::MoveToNextLine) || event->matches(QKeySequence::MoveToNextWord) ||
            event->matches(QKeySequence::MoveToNextChar) || event->matches(QKeySequence::MoveToPreviousLine) ||
            event->matches(QKeySequence::MoveToPreviousWord) || event->matches(QKeySequence::MoveToPreviousChar) ||
            event->matches(QKeySequence::SelectAll) || event->matches(QKeySequence::SelectEndOfDocument) ||
            event->matches(QKeySequence::SelectEndOfBlock) || event->matches(QKeySequence::SelectEndOfLine) ||
            event->matches(QKeySequence::SelectStartOfDocument) || event->matches(QKeySequence::SelectStartOfBlock) ||
            event->matches(QKeySequence::SelectStartOfLine) || event->matches(QKeySequence::SelectNextLine) ||
            event->matches(QKeySequence::SelectNextWord) || event->matches(QKeySequence::SelectNextChar) ||
            event->matches(QKeySequence::SelectPreviousLine) || event->matches(QKeySequence::SelectPreviousWord) ||
            event->matches(QKeySequence::SelectPreviousChar) || event->matches(QKeySequence::Copy))
            QPlainTextEdit::keyPressEvent(event);
    }
}
