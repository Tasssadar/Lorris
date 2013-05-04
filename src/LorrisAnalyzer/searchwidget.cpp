/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QVBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QApplication>
#include <QBitmap>
#include <QListWidget>
#include <QKeyEvent>
#include <QProxyModel>

#include "searchwidget.h"
#include "widgetfactory.h"
#include "lorrisanalyzer.h"
#include "widgetarea.h"
#include "../ui/hookedlineedit.h"

#define SORT_DATA     Qt::UserRole
#define TARGET_DATA   Qt::UserRole+1
#define SLOT_DATA     Qt::UserRole+2
#define ARG1_DATA     Qt::UserRole+3
#define ARG2_DATA     Qt::UserRole+4
#define ARG3_DATA     Qt::UserRole+5

#define ARG_COUNT 3

SearchWidget::SearchWidget(WidgetArea *area, LorrisAnalyzer *analyzer) :
    FloatingWidget(NULL)
{
    m_area = area;
    m_analyzer = analyzer;
    m_lastLen = 0;

    setFixedSize(195, 250);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_line = new HookedLineEdit(this);
    m_list = new QListWidget(this);

    m_list->setFont(Utils::getMonospaceFont());
    m_list->setFont(Utils::getMonospaceFont());

    m_line->setStyleSheet("background-color: #1b1b1b; color: #ffffff; border: 1px solid #FF4444;");
    QPalette p;
    p.setColor(QPalette::Highlight, Qt::darkGray);
    m_line->setPalette(p);

    m_list->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
    setStyleSheet("QListWidget::item { padding-top: 3px; padding-bottom:3px;}"
                  "QListWidget::item:hover { background-color: transparent; }"
                  "QListWidget::item:selected { background-color: #FF4444; color: white; }");
    m_list->setStyleSheet("background-color: #000000; color: #ffffff;");
    m_list->setUniformItemSizes(true);
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->installEventFilter(this);

    QVBoxLayout *v = new QVBoxLayout(this);
    v->addWidget(m_line);
    v->addWidget(m_list, 1);

    QLabel *icon = new QLabel(this);
    icon->setPixmap(QIcon(":/actions/search").pixmap(20, 20));
    int frameW = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(styleSheet() + QString("QLineEdit { padding-left: %1px; };").arg(icon->sizeHint().width() + frameW));

    QSize msz = m_line->minimumSizeHint();
    m_line->setMinimumSize(std::max(msz.width(), icon->sizeHint().height() + frameW * 2 + 2),
                           std::max(msz.height(), icon->sizeHint().height() + frameW * 2 + 2));
    icon->show();

    connect(m_list, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(itemActivated(QListWidgetItem*)));
    connect(m_line, SIGNAL(keyPressed(int)),                 SLOT(lineKeyPressed(int)));
    connect(m_line, SIGNAL(textChanged(QString)),            SLOT(filterChanged(QString)));

    initItems();

    show();
    move(QCursor::pos());

    icon->move(m_line->x() + frameW, m_line->y() + (m_line->height() - icon->height())/2);

    m_line->setFocus();
    m_line->grabKeyboard();
    m_line->setText(m_area->getLastSearch());
    m_line->selectAll();
}

SearchWidget::~SearchWidget()
{
    m_line->releaseKeyboard();
    delete_vect(m_items);
}

void SearchWidget::initItems()
{
    // widgets
    const std::vector<QString>& n = sWidgetFactory.getWidgetNames();
    for(size_t i = 0; i < n.size(); ++i)
        addItem(tr("Add %1Widget").arg(n[i]), this, "actionAddWidget", QVariant((int)i));

    addItem(tr("Clear data"), m_analyzer, "clearData");
    addItem(tr("Change structure"), m_analyzer, "editStructure");

    addItem(tr("Create bookmark"), m_area, "addBookmark");
    addItem(tr("Lock all widgets"), m_area, "lockAll");
    addItem(tr("Unlock all widgets"), m_area, "unlockAll");
    addItem(tr("Toggle grid visibility"), m_area, "toggleGrid");
    addItem(tr("Show/hide bookmarks"), m_area, "toggleBookmarks");
    addItem(tr("Align widgets to grid"), m_area, "alignWidgets");
    addItem(tr("Toggle area preview"), m_area, "togglePreview");
    addItem(tr("Toggle widget titles"), m_area, "toggleWidgetTitles");

    m_list->sortItems();
}

QListWidgetItem* SearchWidget::addItem(const QString& text, QObject *target, const char *slot,
              const QVariant& arg1, const QVariant& arg2, const QVariant& arg3)
{
    QListWidgetItem *it = new QListWidgetItem(text, m_list);
    it->setData(TARGET_DATA, QVariant::fromValue((void*)target));
    it->setData(SLOT_DATA, QVariant(slot));
    it->setData(ARG1_DATA, arg1);
    it->setData(ARG2_DATA, arg2);
    it->setData(ARG3_DATA, arg3);
    m_items.push_back(it);
    m_itemVisibility.push_back(true);
    return it;
}

void SearchWidget::itemActivated(QListWidgetItem *it)
{
    QObject *obj = (QObject*)it->data(TARGET_DATA).value<void*>();
    QString slot = it->data(SLOT_DATA).toString();

    QGenericArgument args[ARG_COUNT];
    for(int i = 0; i < ARG_COUNT; ++i)
    {
        QVariant v = it->data(ARG1_DATA+i);
        switch(v.type())
        {
            case QVariant::Invalid:
                break;
            case QVariant::Int:
                args[i] = Q_ARG(int, v.toInt());
                break;
            case QVariant::UInt:
                args[i] = Q_ARG(uint, v.toUInt());
                break;
            case QVariant::String:
                args[i] = Q_ARG(QString, v.toString());
                break;
            default:
                qWarning("Unsupported action arg type in SearchWidget::itemActivated");
                break;
        }
    }

    m_line->releaseKeyboard();

    QMetaObject::invokeMethod(obj, slot.toStdString().c_str(), Qt::DirectConnection,
                              args[0], args[1], args[2]);

    m_area->setLastSearch(it->text());
    deleteLater();
}

void SearchWidget::actionAddWidget(int type)
{
    QPoint p = pos();
    p = m_area->mapFromGlobal(p);
    m_area->addWidget(p, type);
}

void SearchWidget::lineKeyPressed(int key)
{
    switch(key)
    {
        case Qt::Key_Escape:
            deleteLater();
            break;
        case Qt::Key_Up:
        {
            int cur = m_list->currentRow();
            if(cur <= 0)
                m_list->setCurrentRow(-1);
            else
                m_list->setCurrentRow(cur-1);
            break;
        }
        case Qt::Key_Down:
        {
            int next = m_list->currentRow()+1;
            if(next < m_list->count())
                m_list->setCurrentRow(next);
            break;
        }
        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            int cur = m_list->currentRow();
            if(cur == -1)
            {
                m_area->setLastSearch(QString());
                deleteLater();
            }
            else
                itemActivated(m_list->item(cur));
            break;
        }
    }
}

bool SearchWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() != QEvent::HoverMove)
        return false;

    QHoverEvent *ev = (QHoverEvent*)event;
    QListWidgetItem *it = m_list->itemAt(ev->pos());
    if(it)
        m_list->setCurrentItem(it);
    return true;
}

static bool compareItems(QListWidgetItem *a, QListWidgetItem *b)
{
    int ia = a->data(SORT_DATA).toInt();
    int ib = b->data(SORT_DATA).toInt();

    if(ia != ib)
        return ia < ib;

    return a->text() < b->text();
}

void SearchWidget::filterChanged(const QString &f)
{
    QListWidgetItem *it = NULL;
    if(f.length() == 0)
    {
        for(size_t i = 0; i < m_items.size(); ++i)
        {
            it = m_items[i];
            if(!m_itemVisibility[i])
            {
                m_itemVisibility[i] = true;
                m_list->addItem(it);
            }
        }
        m_list->sortItems();
        m_list->setCurrentRow(-1);
    }
    else
    {
        std::vector<QListWidgetItem*> add;
        bool added = (f.length() > m_lastLen);
        int idx = 0;

        while(m_list->count() > 0)
            m_list->takeItem(0);

        for(size_t i = 0; i < m_items.size(); ++i)
        {
            if(!m_itemVisibility[i] && added)
                continue;

            it = m_items[i];
            idx = it->text().indexOf(f, 0, Qt::CaseInsensitive);

            m_itemVisibility[i] = (idx != -1);

            if(m_itemVisibility[i])
            {
                add.push_back(it);
                it->setData(SORT_DATA, QVariant(idx));
            }
        }

        std::sort(add.begin(), add.end(), compareItems);

        for(size_t i = 0; i < add.size(); ++i)
            m_list->addItem(add[i]);

        if(add.size() == 1)
            m_list->setCurrentRow(0);
        else
            m_list->setCurrentRow(-1);
    }

    m_lastLen = f.length();
}
