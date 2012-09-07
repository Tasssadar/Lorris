/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef FORMULAEVALUATION_H
#define FORMULAEVALUATION_H

#include <QObject>

class QScriptEngine;

class FormulaEvaluation : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void setError(bool error, QString text = QString());

public:
    FormulaEvaluation(QObject *parent = NULL);

    QVariant evaluate(const QString& val);
    bool isActive() const { return m_script_eng != NULL; }

public slots:
    void setFormula(const QString& formula);
    QString getFormula();
    void showFormulaDialog();

private:
    QScriptEngine *m_script_eng;
    QString m_formula;
};
#endif // FORMULAEVALUATION_H
