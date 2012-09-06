/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef NUMBERWIDGET_H
#define NUMBERWIDGET_H

#include <QDialog>
#include "datawidget.h"

#include "ui_formuladialog.h"

class QMenu;
class QScriptEngine;

enum NumberFormats
{
    FMT_DECIMAL,
    FMT_EXPONENT,
    FMT_HEX,
    FMT_BINARY,
    FMT_COUNT
};

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

    void showFormulaDialog();

protected:
     void processData(analyzer_data *data);
     void resizeEvent ( QResizeEvent * event );

private slots:
     void fmtSelected(int i);
     void levelSelected();

private:
     void prependZeros(QString& n, quint8 len);

     QLabel *num;
     quint8 numberType;
     quint8 format;
     bool level;

     QAction *bitsAction[NUM_COUNT];
     QAction *fmtAction[FMT_COUNT];
     QAction *levelAction;
     QScriptEngine *m_script_eng;
     QString m_formula;
};

class NumberWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    NumberWidgetAddBtn(QWidget *parent = 0);
};

class FormulaDialog : public QDialog, private Ui::FormulaDialog
{
    Q_OBJECT
public:
    FormulaDialog(QString formula, QWidget *parent = NULL);
    ~FormulaDialog();

    QString getFormula() const;

public slots:
    void accept();

private:
    Ui::FormulaDialog *ui;
};

#endif // NUMBERWIDGET_H
