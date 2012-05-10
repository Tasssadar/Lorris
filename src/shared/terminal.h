/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef TERMINAL_H
#define TERMINAL_H

#include <QString>
#include <QAbstractScrollArea>
#include <vector>
#include <QPoint>
#include <QTime>
#include <QTimer>

class QMenu;
class QByteArray;
class QFile;

enum term_fmt
{
    FMT_TEXT,
    FMT_HEX,
    FMT_MAX
};

enum term_input
{
    INPUT_SEND_KEYPRESS,
    INPUT_SEND_COMMAND,
    INPUT_MAX
};

class Terminal : public QAbstractScrollArea
{
    Q_OBJECT

Q_SIGNALS:
    void keyPressed(QString key);
    void fontChanged(const QString& fontData);

public:
    Terminal(QWidget *parent);
    ~Terminal();

    void writeToFile(QFile *file);

    QByteArray getData()
    {
        return QByteArray(m_data.data(), m_data.size());
    }

    int getFmt() { return m_fmt; }
    void setFont(const QFont &f);
    void loadFont(const QString& str);
    QString getFontData();

public slots:
    void clear();
    void pause(bool pause);
    void appendText(const QString& text)
    {
        appendText(text.toUtf8());
    }

    void appendText(const QByteArray& text);
    void setFmt(int fmt);
    void setInput(quint8 input);
    void showFontDialog();

protected:
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

private slots:
    void copyToClipboard();
    void pasteFromClipboard();
    void updateScrollBars();

private:
    void handleInput(const QString &data, int key = 0);
    void addLine(quint32 pos, QChar *&line_start, QChar *&line_end);
    void addLines(const QString& text);
    void addHex();
    QPoint mouseToTextPos(const QPoint& pos);

    void selectAll();

    inline void adjustSelectionWidth(int &w, quint32 i, quint32 max, int len);

    inline std::vector<QString>& lines()
    {
        return m_paused ? m_pause_lines : m_lines;
    }

    std::vector<QString> m_lines;
    std::vector<QString> m_pause_lines;
    std::vector<char> m_data;

    QString m_command;

    bool m_paused;
    quint8 m_fmt;
    quint8 m_input;
    int m_hex_pos;

    int m_char_height;
    int m_char_width;

    QPoint m_cursor_pos;
    QPoint m_cursor_pause_pos;
    QRect m_cursor;

    QPoint m_sel_start;
    QPoint m_sel_begin;
    QPoint m_sel_stop;

    QMenu *m_context_menu;
    QAction *m_fmt_act[FMT_MAX];

    QTimer m_updateTimer;

    bool m_changed;
};

#endif // TERMINAL_H
