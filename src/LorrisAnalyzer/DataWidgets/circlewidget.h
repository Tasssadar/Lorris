/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include "datawidget.h"

class CircleDraw;

enum angleType
{
    ANG_RAD = 0,
    ANG_DEG,
    ANG_RANGE,

    ANG_MAX
};

class CircleWidget : public DataWidget
{
    Q_OBJECT
public:
    explicit CircleWidget(QWidget *parent = 0);    
    ~CircleWidget();

    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

    void changeAngType(int i, int min = -1, int max = -1);

public slots:
    void setValue(const QVariant &var);

protected:
     void processData(analyzer_data *data);

private slots:
     void angTypeChanged(int i);
     void setNumType(int i);
     void clockwiseTriggered(bool checked);

private:
     float toRad(const QVariant& var);

     CircleDraw *m_circle;
     QAction *m_type_act[ANG_MAX];
     QAction *m_bits_act[NUM_COUNT];
     QAction *m_clockwiseAct;
     qint32 m_range_min;
     qint32 m_range_max;
     quint8 m_ang_type;
     quint8 m_num_type;
};

class CircleDraw : public QWidget
{
    Q_OBJECT
public:
    CircleDraw(QWidget *parent = 0);

    void setAngle(float ang);

protected:
    void paintEvent(QPaintEvent *);

private:
    float m_angle;
};

class CircleWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    CircleWidgetAddBtn(QWidget *parent = 0);
};

#endif // CIRCLEWIDGET_H
