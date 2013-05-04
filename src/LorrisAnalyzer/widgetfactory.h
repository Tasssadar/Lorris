/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include <QTypeInfo>
#include <vector>
#include "../misc/singleton.h"
#include <QHash>

class QWidget;
class DataWidget;
class DataWidgetAddBtn;

#define REGISTER_DATAWIDGET(id, n, en) DataWidget *n##WidgetInst(QWidget *parent) { return new n##Widget(parent); } \
       DataWidgetAddBtn *n##BtnInst(QWidget *parent) { return new n##WidgetAddBtn(parent); } \
       struct n##WidgetInit { \
            n##WidgetInit() { \
                sWidgetFactory.addWidgetName(id, #n); \
                sWidgetFactory.addWidgetInit(id, &n##WidgetInst); \
                sWidgetFactory.addBtnInit(&n##BtnInst); \
                sWidgetFactory.addScriptEnum(#id, id); \
                void (*func)() = en; \
                if(func) \
                    func(); \
            } \
        }; \
        static const n##WidgetInit n##widgetinit;

#define REGISTER_ENUM(val) sWidgetFactory.addScriptEnum(#val, val);

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
    WIDGET_NUMBER,
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
    WIDGET_STATUS,
    WIDGET_OPENGL,

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
    void addScriptEnum(const char *text, quint32 val);
    void addWidgetName(quint32 id, const char *name);

    DataWidget *getWidget(quint32 type, QWidget *parent);
    std::vector<DataWidgetAddBtn*> getButtons(QWidget *parent);

    const std::vector<QString>& getWidgetNames()
    {
        return m_names;
    }

    DataWidget *copy(DataWidget *w);

    const QHash<QString, quint32>& getScriptEnums() const
    {
        return m_scriptEnums;
    }

private:
    widgetInit m_widgetInits[WIDGET_MAX];
    QHash<QString, quint32> m_scriptEnums;
    std::vector<btnInit> m_btnInits;
    std::vector<QString> m_names;
};

#define sWidgetFactory WidgetFactory::GetSingleton()

#endif // WIDGETFACTORY_H
