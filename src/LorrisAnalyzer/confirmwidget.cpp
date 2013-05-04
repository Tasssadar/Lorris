/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>

#include "confirmwidget.h"
#include "../misc/utils.h"
#include "searchwidget.h"

ConfirmWidget::ConfirmWidget(QListWidgetItem *it) :
    FloatingWidget(NULL)
{
    setFixedSize(150, 50);

    m_list = new FlatListWidget(false, this);
    m_list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_list->addItem(it);

    QLabel *ic = new QLabel(this);
    ic->setPixmap(QIcon(":/icons/question").pixmap(16, 16));

    QLabel *l = new QLabel(tr("OK?"), this);
    l->setStyleSheet("color: gray");

    QVBoxLayout *main = new QVBoxLayout(this);
    QHBoxLayout *top = new QHBoxLayout;
    top->addWidget(ic);
    top->addWidget(l, 1);
    top->setContentsMargins(5, 0, 0, 0);
    main->addLayout(top);
    main->addWidget(m_list, 1);
    main->setContentsMargins(0, 5, 0, 5);

    show();
    move(QCursor::pos()-m_list->geometry().center());
    setFocus();

    int p = ic->x() + ic->width();
    // padding-left for QListWidgetItem does not work.
    //m_list->setStyleSheet(QString("QListWidget::item { padding-left: %1px; }").arg(p) + list->styleSheet());
    QLabel *it_text = new QLabel(it->text(), this);
    it_text->setStyleSheet(QString("color: white; padding-left: %1px;").arg(p));
    it_text->setFont(Utils::getMonospaceFont());
    m_list->setItemWidget(it, it_text);
    it->setText("");

    connect(m_list, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(itemActivated(QListWidgetItem*)));
}

ConfirmWidget::~ConfirmWidget()
{
}

void ConfirmWidget::itemActivated(QListWidgetItem *it)
{
    SearchWidget::invokeItem(it);
    deleteLater();
}

void ConfirmWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter)
    {
        QPoint p = m_list->mapFromGlobal(QCursor::pos());
        QListWidgetItem *it = m_list->itemAt(p);
        if(it)
            itemActivated(it);
        deleteLater();
    }
    else if(ev->key() == Qt::Key_Escape)
        deleteLater();
}
