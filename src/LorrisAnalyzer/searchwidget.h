/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include "floatingwidget.h"

class HookedLineEdit;
class QListWidget;
class QListWidgetItem;
class WidgetArea;
class LorrisAnalyzer;

class SearchWidget : public FloatingWidget
{
    Q_OBJECT
public:
    explicit SearchWidget(WidgetArea *area, LorrisAnalyzer *analyzer);
    ~SearchWidget();

    static void invokeItem(QListWidgetItem *it);

private slots:
    void itemActivated(QListWidgetItem *it);
    void actionAddWidget(int type);
    void lineKeyPressed(int key);
    void filterChanged(const QString& f);

private:
    QListWidgetItem *addItem(const QString &text, QObject *target, const char *slot,
                 const QVariant& arg1 = QVariant(), const QVariant& arg2 = QVariant(),
                 const QVariant& arg3 = QVariant());
    QListWidgetItem *addConfirmItem(const QString &text, QObject *target, const char *slot,
                 const QVariant& arg1 = QVariant(), const QVariant& arg2 = QVariant(),
                 const QVariant& arg3 = QVariant());

    void initItems();

    HookedLineEdit *m_line;
    QListWidget *m_list;
    WidgetArea *m_area;
    LorrisAnalyzer *m_analyzer;
    int m_lastLen;
    std::vector<QListWidgetItem*> m_items;
    std::vector<bool> m_itemVisibility;
};

#endif // SEARCHWIDGET_H
