/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include "../datawidget.h"

namespace Ui {
    class JoystickWidget;
}

class JoystickWidget : public DataWidget
{
    Q_OBJECT
public:
    JoystickWidget(QWidget *parent = 0);
    ~JoystickWidget();

    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void refreshJoysticks();
    void showStructDialog();

protected:
     void processData(analyzer_data *data);

private:
     void setEmpty(bool empty);

     Ui::JoystickWidget *ui;
};

class JoystickWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    JoystickWidgetAddBtn(QWidget *parent = 0);
};

class JoyStructElement
{
public:
    virtual ~JoyStructElement();

    void addRef();
    void rmRef();

    int getType() const { return m_type; }
    quint32 getLen() const { return m_len; }

    virtual void fill(QByteArray &data) const = 0;

protected:
    enum Type
    {
        STATIC = 0,
        JOYSTICK,

        MAX
    };

    JoyStructElement(Type t);

    Type m_type;
    quint32 m_len;
    int m_refCnt;
};

class JoyStruct
{
public:
    JoyStruct();
    JoyStruct(const JoyStruct& str);
    virtual ~JoyStruct();

private:
    QString m_name;
    quint32 m_len;
    std::vector<JoyStructElement*> m_elements;
};


#endif // JOYSTICKWIDGET_H
