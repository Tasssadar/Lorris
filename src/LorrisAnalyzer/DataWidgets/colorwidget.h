/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include "datawidget.h"

class QSlider;
class ColorDisplay;

class ColorWidget : public DataWidget
{
    Q_OBJECT
public:
    ColorWidget(QWidget *parent = 0);
    ~ColorWidget();

    void setUp(Storage *);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void setValue(int r, int g, int b);
    void setValue(QString hex);
    void setValueAr(QList<int> val);
    void showValues(bool show);

protected:
     void processData(analyzer_data *data);

private slots:
     void brightTriggered();
     void colorTriggered();
     void brightChanged(int value);
     void colorChangedR(int value);
     void colorChangedG(int value);
     void colorChangedB(int value);

private:
     void updateColor();

     ColorDisplay *m_widget;
     QHBoxLayout *m_brightness_layout;
     QHBoxLayout *m_color_layout[3];
     qint16 m_brightness;
     qint16 m_color_cor[3];

     QAction *brightAct;
     QAction *colorAct;
     QAction *textAct;
};

class ColorDisplay : public QWidget
{
    Q_OBJECT
public:
    ColorDisplay(QWidget *parent);

    quint8 *color() { return m_color; }

    void setDrawNums(bool draw) { m_drawNums = draw; }

protected:
    void paintEvent(QPaintEvent * ev);

private:
    quint8 m_color[3];
    bool m_drawNums;
};

class ColorWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    ColorWidgetAddBtn(QWidget *parent = 0);

};
#endif // COLORWIDGET_H
