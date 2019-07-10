/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QScriptEngine>
#include "formulaevaluation.h"
#include "../ui/formuladialog.h"

FormulaEvaluation::FormulaEvaluation(QObject *parent) : QObject(parent)
{
    m_script_eng = NULL;
    m_formula = "%n";
    m_error = false;
}

void FormulaEvaluation::setFormula(const QString &formula)
{
    m_formula = formula.trimmed();

    emit setError(false);
    m_error = false;

    if(m_formula == "%n")
    {
        delete m_script_eng;
        m_script_eng = NULL;
    }
    else if(!m_formula.contains("%n"))
    {
        emit setError(true, tr("Formula must contain \"%n\" expression!"));
        m_error = true;
    }
    else
    {
        m_formula.replace("%1", "%%1");
        m_formula.replace("%n", "%1");

        if(!m_script_eng)
            m_script_eng = new QScriptEngine(this);
    }
}

QString FormulaEvaluation::getFormula()
{
    QString res = m_formula;
    res.replace("%1", "%n");
    return res;
}

void FormulaEvaluation::showFormulaDialog()
{
    FormulaDialog d(m_formula);

    if(d.exec() == QDialog::Rejected)
        return;

    setFormula(d.getFormula());
}

QVariant FormulaEvaluation::evaluate(const QString& val)
{
    if(!m_script_eng)
        return QVariant();

    QString exp = m_formula.arg(val);
    QScriptValue res = m_script_eng->evaluate(exp);

    if(!m_script_eng->hasUncaughtException())
    {
        if(m_error)
        {
            m_error = false;
            emit setError(false);
        }

        if(res.isValid())
            return res.toVariant();
    }
    else
    {
        m_error = true;
        emit setError(true, m_script_eng->uncaughtException().toString());
    }
    return QVariant();
}
