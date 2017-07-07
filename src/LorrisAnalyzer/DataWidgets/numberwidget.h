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

#define PREC_COUNT 8

class NumberWidget : public DataWidget
{
    Q_OBJECT
public:
    /// \brief To be used with \c setFormat()
    enum NumberFormats
    {
        FMT_DECIMAL,
        FMT_EXPONENT,
        FMT_HEX,
        FMT_BINARY,
        FMT_COUNT
    };

    static void addEnum();

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
    void setFormat(int fmt);

    void showFormulaDialog();

protected:
     void processData(analyzer_data *data);
     void resizeEvent ( QResizeEvent * event );

private slots:
     void levelSelected();
     void setPrecisionAct(int idx);

private:
     void prependZeros(QString& n, quint8 len);

     QLabel *m_num;
     quint8 m_numberType;
     quint8 m_format;
     bool m_level;
     quint8 m_digits;

     QAction *m_bitsAction[NUM_COUNT_WITH_STRING];
     QAction *m_fmtAction[FMT_COUNT];
     QAction *m_precAct[PREC_COUNT];
     QAction *m_levelAction;

     FormulaEvaluation m_eval;
};

#endif // NUMBERWIDGET_H
