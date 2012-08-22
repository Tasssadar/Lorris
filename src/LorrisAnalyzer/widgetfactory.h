/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include <vector>
#include "../misc/singleton.h"

class QWidget;
class DataWidget;
class DataWidgetAddBtn;

#define REGISTER_DATAWIDGET(id, n) DataWidget *n##WidgetInst(QWidget *parent) { return new n##Widget(parent); } \
       DataWidgetAddBtn *n##BtnInst(QWidget *parent) { return new n##WidgetAddBtn(parent); } \
       struct n##WidgetInit { \
            n##WidgetInit() { \
                sWidgetFactory.addWidgetInit(id, &n##WidgetInst); \
                sWidgetFactory.addBtnInit(&n##BtnInst); \
            } \
        }; \
        static const n##WidgetInit n##widgetinit;

/*
#define REGISTER_DATAWIDGET_NOBTN(id, n) DataWidget *n##WidgetInst(QWidget *parent) { return new n##Widget(parent); } \
       struct n##WidgetInit { \
            n##WidgetInit() { \
                sWidgetFactory.addWidgetInit(id, &n##WidgetInst); \
            } \
        }; \
        static const n##WidgetInit n##widgetinit;
*/

enum WidgetTypes
{
    WIDGET_NUMBERS,
    WIDGET_BAR,
    WIDGET_COLOR,
    WIDGET_GRAPH,
    WIDGET_SCRIPT,
    WIDGET_INPUT,
    WIDGET_TERMINAL,
    WIDGET_BUTTON,
    WIDGET_CIRCLE,
    WIDGET_SLIDER,
    WIDGET_CANVAS,

    WIDGET_MAX
    //TODO: X Y mapa, rafickovej ukazatel, timestamp, bool, binarni cisla
};

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
    widgetInit m_widgetInits[WIDGET_MAX];
    std::vector<btnInit> m_btnInits;
};

#define sWidgetFactory WidgetFactory::GetSingleton()

#endif // WIDGETFACTORY_H
