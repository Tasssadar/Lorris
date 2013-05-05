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
#include "confirmwidget.h"

enum ItemDataTypes
{
    DATA_SORT      = Qt::UserRole,
    DATA_CONFIRM,
    DATA_TARGET,
    DATA_SLOT,

    // args must be after each other
    DATA_ARG1,
    DATA_ARG2,
    DATA_ARG3
};

#define ARG_COUNT 3

SearchWidget::SearchWidget(WidgetArea *area, LorrisAnalyzer *analyzer) :
    FloatingWidget(NULL)
{
    m_area = area;
    m_analyzer = analyzer;
    m_lastLen = 0;

    setFixedSize(200, 250);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_line = new HookedLineEdit(this);
    m_list = new FlatListWidget(true, this);

    m_line->setFont(Utils::getMonospaceFont());

    m_line->setStyleSheet("background-color: #1b1b1b; color: #ffffff; border: 1px solid #FF4444;");
    QPalette p;
    p.setColor(QPalette::Highlight, Qt::darkGray);
    m_line->setPalette(p);

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
    const std::vector<QString>& n = sWidgetFactory.getTranslatedWidgetNames();
    for(int i = 0; i < (int)n.size(); ++i)
        addItem(tr("Add %1").arg(n[i].toLower()), this, "actionAddWidget", QVariant(i));

    addConfirmItem(tr("Clear data"), m_analyzer, "clearData");
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
    it->setData(DATA_CONFIRM, QVariant(false));
    it->setData(DATA_TARGET, QVariant::fromValue((void*)target));
    it->setData(DATA_SLOT, QVariant(slot));
    it->setData(DATA_ARG1, arg1);
    it->setData(DATA_ARG2, arg2);
    it->setData(DATA_ARG3, arg3);
    m_items.push_back(it);
    m_itemVisibility.push_back(true);
    return it;
}

QListWidgetItem* SearchWidget::addConfirmItem(const QString& text, QObject *target,
              const char *slot, const QVariant& arg1, const QVariant& arg2, const QVariant& arg3)
{
    QListWidgetItem *it = addItem(text, target, slot, arg1, arg2, arg3);
    it->setData(DATA_CONFIRM, QVariant(true));
    return it;
}

void SearchWidget::itemActivated(QListWidgetItem *it)
{
    m_line->releaseKeyboard();
    m_area->setLastSearch(it->text());

    if(!it->data(DATA_CONFIRM).toBool())
        invokeItem(it);
    else
    {
        m_list->takeItem(m_list->row(it));
        for(size_t i = 0; i < m_items.size(); ++i)
        {
            if(it == m_items[i])
            {
                m_items.erase(m_items.begin()+i);
                break;
            }
        }
        new ConfirmWidget(it);
    }
    deleteLater();
}

void SearchWidget::invokeItem(QListWidgetItem *it)
{
    QObject *obj = (QObject*)it->data(DATA_TARGET).value<void*>();
    QString slot = it->data(DATA_SLOT).toString();

    // We need to copy data from QVariant to newly allocated memory,
    // because QGenericArgument stores pointer to data, which get
    // destroyed at the end of the scope
    QGenericArgument args[ARG_COUNT];
    for(int i = 0; i < ARG_COUNT; ++i)
    {
        QVariant v = it->data(DATA_ARG1+i);
        if(v.type() == QVariant::Invalid)
            continue;

        args[i] = QGenericArgument(v.typeName(), QMetaType::construct(v.type(), v.data()));
    }

    QMetaObject::invokeMethod(obj, slot.toStdString().c_str(), Qt::DirectConnection,
                              args[0], args[1], args[2]);

    for(int i = 0; i < ARG_COUNT; ++i)
    {
        QVariant v = it->data(DATA_ARG1+i);
        if(v.type() == QVariant::Invalid)
            continue;

        QMetaType::destroy(v.type(), args[i].data());
    }
}

void SearchWidget::actionAddWidget(int type)
{
    QPoint p = pos();
    p = m_area->mapFromGlobal(p);

    DataWidget *w = m_area->addWidget(p, type);
    w->align();
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

static bool compareItems(QListWidgetItem *a, QListWidgetItem *b)
{
    int ia = a->data(DATA_SORT).toInt();
    int ib = b->data(DATA_SORT).toInt();

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
                it->setData(DATA_SORT, QVariant(idx));
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
