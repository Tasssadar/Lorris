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
    class SliderWidget_horizontal;
    class SliderWidget_vertical;
}

class QwtSlider;
class QLineEdit;
class QShortcut;

enum orientation
{
    ORI_HOR_L_R = 0,
    ORI_VER_B_T,
    ORI_HOR_R_L,
    ORI_VER_T_B,

    ORI_MAX
};

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

    /// \deprecated Use setRange(min, max);
    void setRange(double min, double max, double step);
    void setRange(double min, double max);

    void setOrientation(int ori);
    int getOrientation() const { return m_orientation; }

    void hideMinMax(bool hide);
    bool isMinMaxVisible() const;

    void setShortcut(const QString& shortcut);
    void showShortcutDialog();

private slots:
    void on_minEdit_textChanged(const QString& text);
    void on_maxEdit_textChanged(const QString& text);
    void on_curEdit_textEdited(const QString& text);
    void on_slider_valueChanged(double val);

    void intAct(bool checked);
    void doubleAct(bool checked);

private:
    void p_setRange(double min, double max);

    QwtSlider *slider() const;
    QLineEdit *maxEdit() const;
    QLineEdit *minEdit() const;
    QLineEdit *curEdit() const;

    QString fixValueToInt(const QString &val);
    void parseMinMax(bool isMax, const QString& text);

    quint8 m_orientation;
    Ui::SliderWidget_horizontal *ui_hor;
    Ui::SliderWidget_vertical *ui_ver;
    QWidget *m_widget;
    double m_min;
    double m_max;
    QShortcut *m_shortcut;

    QAction *m_int_act;
    QAction *m_double_act;
    QAction *m_ori_act[ORI_MAX];
    QAction *m_hide_act;
};

#endif // SLIDERWIDGET_H
