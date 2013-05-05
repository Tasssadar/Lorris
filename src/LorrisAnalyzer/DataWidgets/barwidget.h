/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef BARWIDGET_H
#define BARWIDGET_H

#include <QDialog>

#include "datawidget.h"
#include "../../misc/formulaevaluation.h"

class QSpinBox;
class QwtThermo;

class BarWidget : public DataWidget
{
    Q_OBJECT
public:
    BarWidget(QWidget *parent);

    void setUp(Storage *);
    void processData(analyzer_data *data);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void setValue(const QVariant &var)
    {
        setValuePrivate(var.toDouble());
    }

    void setRange(double min, double max);

    void setRotation(int i);
    void rotationSelected(int i)
    {
        setRotation(i);
    }

    void setDataType(int i);
    double getValue() const;
    double getMin() const;
    double getMax() const;
    void setAlarmEnabled(bool enable);
    void setAlarmLevel(double val);
    bool isAlarmEnabled() const;
    double getAlarmLevel() const;

    void setFormula(const QString& formula);
    void showFormulaDialog();

private slots:
    void rangeSelected();
    void showScale(bool show);
    void showVal(bool show);
    void alarmLevelAct();
    void showColorsDialog();

private:
    void setValuePrivate(double value);
    void rotate(int i);
    int getScalePos();

    QwtThermo *m_bar;
    QLabel *m_label;
    qint64 m_min;
    qint64 m_max;
    quint8 m_numberType;

    QAction *m_bitsAct[NUM_COUNT];
    QAction *m_rotAct[2];
    QAction *m_rangeAct;
    QAction *m_showScaleAct;
    QAction *m_showValAct;
    QAction *m_alarmEnable;
    QAction *m_alarmLevel;
    quint8 m_rotation;

    FormulaEvaluation m_eval;
};

#define COLOR_COUNT 3

class BarWidgetClrDialog : public QDialog
{
    Q_OBJECT
public:
    BarWidgetClrDialog(const QPalette& curPalette, QWidget *parent);

    void updatePalette(QPalette& p);

private slots:
    void btnClicked(int role);

private:
    void setColor(int role, const QColor& clr);

    QColor m_colors[COLOR_COUNT];
    QPushButton *m_colorBtns[COLOR_COUNT];
    QwtThermo *m_bar;
};

#endif // BARWIDGET_H
