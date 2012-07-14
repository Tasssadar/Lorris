/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include <vector>
#include <QHash>
#include "../misc/singleton.h"

class DataWidget;
class DataWidgetAddBtn;
class QWidget;

#define REGISTER_DATAWIDGET(id, n) DataWidget *n##WidgetInst(QWidget *parent) { return new n##Widget(parent); } \
       DataWidgetAddBtn *n##BtnInst(QWidget *parent) { return new n##WidgetAddBtn(parent); } \
       struct n##WidgetInit { \
            n##WidgetInit() { \
                sWidgetFactory.addWidgetInit(id, &n##WidgetInst); \
                sWidgetFactory.addBtnInit(&n##BtnInst); \
            } \
        }; \
        static const n##WidgetInit n##widgetinit;

#define REGISTER_DATAWIDGET_NOBTN(id, n) DataWidget *n##WidgetInst(QWidget *parent) { return new n##Widget(parent); } \
       struct n##WidgetInit { \
            n##WidgetInit() { \
                sWidgetFactory.addWidgetInit(id, &n##WidgetInst); \
            } \
        }; \
        static const n##WidgetInit n##widgetinit;

class WidgetFactory : public Singleton<WidgetFactory>
{
public:
    typedef DataWidget* (*widgetInit)(QWidget *);
    typedef DataWidgetAddBtn* (*btnInit)(QWidget *);

    WidgetFactory();

    void addWidgetInit(quint32 type, widgetInit init);
    void addBtnInit(btnInit init);

    DataWidget *getWidget(quint32 type, QWidget *parent);
    std::vector<DataWidgetAddBtn*> getButtons(QWidget *parent);

    DataWidget *copy(DataWidget *w);

private:
    QHash<quint32, widgetInit> m_widgetInits;
    std::vector<btnInit> m_btnInits;
};

#define sWidgetFactory WidgetFactory::GetSingleton()

#endif // WIDGETFACTORY_H
