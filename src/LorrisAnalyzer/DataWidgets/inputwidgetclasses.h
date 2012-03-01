#ifndef INPUTWIDGETCLASSES_H
#define INPUTWIDGETCLASSES_H

#include <QPushButton>
#include <QLineEdit>

class Button : public QPushButton
{
    Q_OBJECT
public:
    Button( QWidget * parent = 0 ) : QPushButton(parent) { }
    ~Button() { }

    static QWidget *newInstance(QWidget *parent) { return new Button(parent); }

public slots:
    void setText(const QString &text) { QPushButton::setText(text); }
    QString text() const { return QPushButton::text(); }
};

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(QWidget *parent = 0) : QLineEdit(parent) { }
    ~LineEdit() { }

    static QWidget *newInstance(QWidget *parent) { return new LineEdit(parent); }

public slots:
    QString text() const { return QLineEdit::text(); }
};

#endif // INPUTWIDGETCLASSES_H
