/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QToolButton>
#include <QKeyEvent>
#include <QStyle>

#include "shortcutinputbox.h"

ShortcutInputBox::ShortcutInputBox(QWidget *parent) : QLineEdit(parent)
{
    init();
}

ShortcutInputBox::ShortcutInputBox(const QKeySequence &seq, QWidget *parent) : QLineEdit(parent)
{
    init();
    setKeySequence(seq);
}

void ShortcutInputBox::init()
{
    setPlaceholderText(tr("Press keys..."));
    setToolTip(tr("Press keys..."));

    m_clear_btn = new QToolButton(this);
    m_clear_btn->setIcon(QIcon(":/actions/red-cross"));
    m_clear_btn->setCursor(Qt::ArrowCursor);
    m_clear_btn->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_clear_btn->hide();

    int frameW = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(m_clear_btn->sizeHint().width() + frameW + 1));
    QSize msz = minimumSizeHint();
    setMinimumSize(std::max(msz.width(), m_clear_btn->sizeHint().height() + frameW * 2 + 2),
                   std::max(msz.height(), m_clear_btn->sizeHint().height() + frameW * 2 + 2));

    m_clear_btn->adjustSize();

    connect(m_clear_btn, SIGNAL(clicked()),            SLOT(clearSeq()));
    connect(this,        SIGNAL(textChanged(QString)), SLOT(updateClearBtn(QString)));
}

void ShortcutInputBox::updateClearBtn(const QString &text)
{
    m_clear_btn->setVisible(!text.isEmpty());
}

void ShortcutInputBox::resizeEvent(QResizeEvent *)
{
    QSize s = m_clear_btn->size();
    int frameW = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    m_clear_btn->move(rect().right() - frameW - s.width(),
                      (rect().bottom() + 1 - s.height())/2);
}

void ShortcutInputBox::keyPressEvent(QKeyEvent *event)
{
    if(event->text().isEmpty())
        return;

    m_sequence = QKeySequence(event->key() | event->modifiers());
    setText(m_sequence.toString(QKeySequence::NativeText));
}

void ShortcutInputBox::clearSeq()
{
    m_sequence = QKeySequence();
    clear();
}

void ShortcutInputBox::setKeySequence(const QKeySequence &seq)
{
    m_sequence = seq;
    setText(m_sequence.toString(QKeySequence::NativeText));
}
