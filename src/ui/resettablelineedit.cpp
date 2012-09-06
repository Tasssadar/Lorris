/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QStyle>

#include "resettablelineedit.h"

ResettableLineEdit::ResettableLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    init();
}

ResettableLineEdit::ResettableLineEdit(const QString &defValue, QWidget *parent) :
    QLineEdit(parent)
{
    init(defValue);
}

void ResettableLineEdit::init(QString defValue)
{
    m_clear_btn = new QToolButton(this);
    m_clear_btn->setIcon(QIcon(":/actions/red-cross"));
    m_clear_btn->setCursor(Qt::ArrowCursor);
    m_clear_btn->setToolTip(tr("Reset to default value"));
    m_clear_btn->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_clear_btn->hide();

    int frameW = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(m_clear_btn->sizeHint().width() + frameW + 1));
    QSize msz = minimumSizeHint();
    setMinimumSize(std::max(msz.width(), m_clear_btn->sizeHint().height() + frameW * 2 + 2),
                   std::max(msz.height(), m_clear_btn->sizeHint().height() + frameW * 2 + 2));

    m_clear_btn->adjustSize();

    m_def_value = defValue;
    setText(defValue);

    connect(m_clear_btn, SIGNAL(clicked()),            SLOT(reset()));
    connect(this,        SIGNAL(textChanged(QString)), SLOT(updateClearBtn(QString)));
}

void ResettableLineEdit::updateClearBtn(const QString &text)
{
    m_clear_btn->setVisible(text != m_def_value);
}

void ResettableLineEdit::resizeEvent(QResizeEvent *)
{
    QSize s = m_clear_btn->size();
    int frameW = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    m_clear_btn->move(rect().right() - frameW - s.width(),
                      (rect().bottom() + 1 - s.height())/2);
}

void ResettableLineEdit::reset()
{
    setText(m_def_value);
}
