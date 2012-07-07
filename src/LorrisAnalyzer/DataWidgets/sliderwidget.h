/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include "datawidget.h"

namespace Ui {
    class SliderWidget;
}

class SliderWidget : public DataWidget
{
    Q_OBJECT
public:
    SliderWidget(QWidget *parent);
    ~SliderWidget();

    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void setInteger() { setType(false); }
    void setDouble() { setType(true); }
    void setType(bool isDouble);

    bool isInteger() const;
    bool isDouble() const;

    double getValue();
    void setValue(double val);

    double getMin() const;
    double getMax() const;
    void setMin(double min);
    void setMax(double max);

private slots:
    void on_minEdit_textChanged(const QString& text);
    void on_maxEdit_textChanged(const QString& text);
    void on_slider_valueChanged(int val);

private:
    QString fixValueToInt(const QString &val);
    void parseMinMax(bool isMax, const QString& text);

    Ui::SliderWidget *ui;
};

class SliderWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    SliderWidgetAddBtn(QWidget *parent = 0);

};
#endif // SLIDERWIDGET_H
