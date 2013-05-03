/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef NUMBERWIDGET_H
#define NUMBERWIDGET_H

#include "datawidget.h"
#include "../../misc/formulaevaluation.h"

class QMenu;

enum NumberFormats
{
    FMT_DECIMAL,
    FMT_EXPONENT,
    FMT_HEX,
    FMT_BINARY,
    FMT_COUNT
};

#define PREC_COUNT 8

class NumberWidget : public DataWidget
{
    Q_OBJECT
public:
    NumberWidget(QWidget *parent = 0);
    ~NumberWidget();

    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void setValue(QVariant var);
    void setDataType(int i);
    void setFormula(const QString& formula);
    void setPrecision(quint8 digits);

    void showFormulaDialog();

protected:
     void processData(analyzer_data *data);
     void resizeEvent ( QResizeEvent * event );

private slots:
     void fmtSelected(int i);
     void levelSelected();
     void setPrecisionAct(int idx);

private:
     void prependZeros(QString& n, quint8 len);

     QLabel *m_num;
     quint8 m_numberType;
     quint8 m_format;
     bool m_level;
     quint8 m_digits;

     QAction *m_bitsAction[NUM_COUNT];
     QAction *m_fmtAction[FMT_COUNT];
     QAction *m_precAct[PREC_COUNT];
     QAction *m_levelAction;

     FormulaEvaluation m_eval;
};

class NumberWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    NumberWidgetAddBtn(QWidget *parent = 0);
};

#endif // NUMBERWIDGET_H
