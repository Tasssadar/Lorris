/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include "datawidget.h"
#include "../../misc/formulaevaluation.h"

class CircleDraw;

class CircleWidget : public DataWidget
{
    Q_OBJECT
public:
    /// \brief To be used with \c setAngType()
    enum angleType
    {
        ANG_RAD = 0,
        ANG_DEG,
        ANG_RANGE,

        ANG_MAX
    };

    static void addEnum();

    explicit CircleWidget(QWidget *parent = 0);    
    ~CircleWidget();

    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void setValue(QVariant var);
    void setClockwise(bool clockwise);
    void drawAngle(bool draw);
    void setDataType(int i);
    void setAngType(int i, double min, double max);
    void setAngType(int i)
    {
        setAngType(i, m_range_min, m_range_max);
    }

protected:
     void processData(analyzer_data *data);

private slots:
     void angTypeChanged(int i);
     void showFormulaDialog();

private:
     float toRad(const QVariant& var);

     FormulaEvaluation m_eval;
     CircleDraw *m_circle;
     QAction *m_type_act[ANG_MAX];
     QAction *m_bits_act[NUM_COUNT];
     QAction *m_clockwiseAct;
     QAction *m_drawAngAct;
     double m_range_min;
     double m_range_max;
     quint8 m_ang_type;
     quint8 m_num_type;
};

class CircleDraw : public QWidget
{
    Q_OBJECT
public:
    CircleDraw(QWidget *parent = 0);

    QVariant& userVal() { return m_userVal; }
    void setAngle(float ang);
    void setClockwise(bool clockwise);

    void setDrawAngle(bool draw)
    {
        m_draw_angle = draw;
        update();
    }

protected:
    void paintEvent(QPaintEvent *);

private:
    float m_angle;
    QVariant m_userVal;
    bool m_clockwise;
    bool m_draw_angle;
};

#endif // CIRCLEWIDGET_H
