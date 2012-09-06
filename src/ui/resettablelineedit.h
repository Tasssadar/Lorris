/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef RESETTABLELINEEDIT_H
#define RESETTABLELINEEDIT_H

#include <QLineEdit>
#include <QToolButton>

class ResettableLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit ResettableLineEdit(QWidget *parent = 0);
    ResettableLineEdit(const QString& defValue, QWidget *parent = 0);
    
    void setDefaultValue(const QString& defValue)
    {
        m_def_value = defValue;
    }

protected slots:
    virtual void reset();

protected:
    void resizeEvent(QResizeEvent *);

    QString m_def_value;

private slots:
    void updateClearBtn(const QString& text);

private:
    void init(QString defValue = QString());

    QToolButton *m_clear_btn;
};

#endif // RESETTABLELINEEDIT_H
