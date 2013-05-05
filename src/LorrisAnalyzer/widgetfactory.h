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
       struct n##WidgetInit { \
            n##WidgetInit() { \
                sWidgetFactory.addWidgetName(id, #n); \
                sWidgetFactory.addWidgetInit(id, &n##WidgetInst); \
                sWidgetFactory.addScriptEnum(#id, id); \
                void (*func)() = en; \
                if(func) \
                    func(); \
            } \
        }; \
        static const n##WidgetInit n##widgetinit;

#define REGISTER_ENUM(val) sWidgetFactory.addScriptEnum(#val, val);

// To fool qt linguist - it needs the translate macro,
// but we don't need the string it returns
// Example: W_TR(QT_TRANSLATE_NOOP("Number", "Number"))
#define W_TR(x) ;

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
    WIDGET_ROTATION,

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
    void addWidgetName(quint32 id, const char* name);
    void translateWidgetNames();

    DataWidget *getWidget(quint32 type, QWidget *parent);
    std::vector<DataWidgetAddBtn*> getButtons(QWidget *parent);

    const std::vector<QString>& getWidgetNames() const;
    const std::vector<QString>& getTranslatedWidgetNames();

    DataWidget *copy(DataWidget *w);

    const QHash<QString, quint32>& getScriptEnums() const
    {
        return m_scriptEnums;
    }

private:
    widgetInit m_widgetInits[WIDGET_MAX];
    QHash<QString, quint32> m_scriptEnums;
    std::vector<QString> m_names;
    std::vector<QString> m_namesTranslated;
};

#define sWidgetFactory WidgetFactory::GetSingleton()

#endif // WIDGETFACTORY_H
