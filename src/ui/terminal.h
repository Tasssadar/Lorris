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
#include <unordered_map>
#include <map>

class QMenu;
class QByteArray;
class QFile;
struct terminal_settings;

enum settings
{
    SET_REPLACE_TAB = 0,
    SET_ALARM,
    SET_NEWLINE,
    SET_RETURN,
    SET_BACKSPACE,
    SET_FORMFEED,
    SET_IGNORE_NULL,
    SET_ENTER_SEND,
    SET_HANDLE_ESCAPE,

    SET_MAX
};

enum set_colors
{
    COLOR_BG = 0,
    COLOR_TEXT,
    COLOR_CURSOR,

    COLOR_MAX
};

enum newlineBehavior
{
    NL_NEWLINE_RETURN = 0,
    NL_NEWLINE,
    NL_RETURN,
    NL_NOTHING
};

enum newlineSend
{
    NLS_RN = 0,
    NLS_N,
    NLS_R,
    NLS_NR,

    NLS_MAX
};

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


struct terminal_settings
{
    terminal_settings()
    {
        chars[SET_REPLACE_TAB] = 0;
        chars[SET_ALARM] = 1;
        chars[SET_FORMFEED] = 1;
        chars[SET_BACKSPACE] = 1;
        chars[SET_NEWLINE] = NL_NEWLINE_RETURN;
        chars[SET_RETURN] = NL_RETURN;
        chars[SET_IGNORE_NULL] = 1;
        chars[SET_ENTER_SEND] = NLS_RN;
        chars[SET_HANDLE_ESCAPE] = 1;
        tabReplace = 4;

        colors[COLOR_BG] = Qt::black;
        colors[COLOR_TEXT] = Qt::white;
        colors[COLOR_CURSOR] = Qt::green;
    }

    void copy(const terminal_settings& set)
    {
        for(int i = 0; i < SET_MAX; ++i)
            chars[i] = set.chars[i];

        for(int i = 0; i < COLOR_MAX; ++i)
            colors[i] = set.colors[i];

        tabReplace = set.tabReplace;
        font = set.font;
    }

    quint8 chars[SET_MAX];
    quint8 tabReplace;
    QColor colors[COLOR_MAX];
    QFont font;
};

class Terminal : public QAbstractScrollArea
{
    Q_OBJECT

Q_SIGNALS:
    void keyPressed(QString key);
    void settingsChanged();
    void fmtSelected(int fmt);
    void paused(bool pause);

public:
    Terminal(QWidget *parent);
    ~Terminal();

    void writeToFile(QFile *file);

    QByteArray getData()
    {
        return QByteArray(m_data.data(), m_data.size());
    }

    int getFmt() { return m_fmt; }
    int getInput() { return m_input; }
    void setFont(const QFont &f);
    QString getSettingsData();
    void loadSettings(const QString& data);

    bool isPaused() const { return m_paused; }

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
    void showSettings();
    void applySettings(const terminal_settings& set);
    void blink(const QColor& color);
    void blink(const QString& color) { blink(QColor(color)); }

protected:
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    bool event(QEvent *event);
    void inputMethodEvent(QInputMethodEvent *e);

private slots:
    void copyToClipboard();
    void pasteFromClipboard();
    void updateScrollBars();
    void endBlink();

private:
    void handleInput(const QString &data, int key = 0);
    void addLine(quint32 pos, QChar *&line_start, QChar *&line_end);
    void newlineChar(quint8 option, quint32& pos);
    void addLines(const QString& text);
    void addHex();
    void redrawAll();
    QPoint mouseToTextPos(const QPoint& pos);
    QString getCurrNewlineStr(Qt::KeyboardModifiers modifiers);
    void handleEscSeq();

    void selectAll();

    inline void adjustSelectionWidth(int &w, quint32 i, quint32 max, int len);

    inline std::vector<QString>& lines()
    {
        return m_paused ? m_pause_lines : m_lines;
    }

    enum EscFlags {
        ESC_BOLD      = 0x01,
    };

    struct EscBlock {
        EscFlags flags;
        QColor color;
        QColor background;

        bool isEmpty() const {
            return !color.isValid() && !background.isValid() && flags == 0;
        }
    };

    std::vector<QString> m_lines;
    std::vector<QString> m_pause_lines;
    std::vector<char> m_data;
    std::unordered_map<int, std::map<int, EscBlock> > m_escapes;
    QString m_esc_seq;
    EscBlock *m_last_esc;

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
    QAction *m_pauseAct;

    QTimer m_updateTimer;

    bool m_changed;

    terminal_settings m_settings;
};

#endif // TERMINAL_H
