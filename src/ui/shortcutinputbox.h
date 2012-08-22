/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SHORTCUTINPUTBOX_H
#define SHORTCUTINPUTBOX_H

#include <QLineEdit>
#include <QShortcut>

class QToolButton;

class ShortcutInputBox : public QLineEdit
{
    Q_OBJECT
    
public:
    explicit ShortcutInputBox(QWidget *parent = 0);
    ShortcutInputBox(const QKeySequence& seq, QWidget *parent = 0);

    QKeySequence getKeySequence() { return m_sequence; }
    void setKeySequence(const QKeySequence& seq);

protected:
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void updateClearBtn(const QString& text);
    void clearSeq();

private:
    void init();

    QToolButton *m_clear_btn;
    QKeySequence m_sequence;
};

#endif // SHORTCUTINPUTBOX_H
