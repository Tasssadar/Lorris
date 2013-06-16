/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

// This file is named floatinginputdialog_impl.cpp because
// when it was just floatinginputdialog.cpp, MSVC2010 just ignored it.
// It didn't even run it through the preprocessor, just ignored it. wtf.

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QEventLoop>
#include <QPointer>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QGridLayout>

#include "../misc/utils.h"
#include "floatinginputdialog.h"

FloatingInputDialog::FloatingInputDialog(const QString& title, const QString& icon, QWidget *parent) :
    FloatingWidget(parent)
{
    QLabel *ic = new QLabel(this);
    ic->setPixmap(QIcon(icon).pixmap(16, 16));

    QLabel *l = new QLabel(title, this);
    l->setStyleSheet("color: white");

    QVBoxLayout *main = new QVBoxLayout(this);
    QHBoxLayout *top = new QHBoxLayout;
    top->addWidget(ic);
    top->addWidget(l, 1);
    top->setContentsMargins(0, 0, 0, 0);
    main->addLayout(top);
}

void FloatingInputDialog::initRangeLayout(QPointer<QLineEdit>& minEdit, QPointer<QLineEdit>& maxEdit)
{
    minEdit = new QLineEdit(this);
    maxEdit = new QLineEdit(this);

    minEdit->setStyleSheet("background-color: #1b1b1b; color: #ffffff; border: 1px solid #FF4444;");
    maxEdit->setStyleSheet("background-color: #1b1b1b; color: #ffffff; border: 1px solid #FF4444;");

    QPalette p;
    p.setColor(QPalette::Highlight, Qt::darkGray);
    minEdit->setPalette(p);
    maxEdit->setPalette(p);

    QLabel *labelMin = new QLabel(tr("Min:"), this);
    labelMin->setStyleSheet("color: white");
    QLabel *labelMax = new QLabel(tr("Max:"), this);
    labelMax->setStyleSheet("color: white");

    QGridLayout *ctx = new QGridLayout;
    ctx->addWidget(labelMin, 0, 0);
    ctx->addWidget(minEdit, 0, 1);
    ctx->addWidget(labelMax, 1, 0);
    ctx->addWidget(maxEdit, 1, 1);

    QVBoxLayout *l = (QVBoxLayout*)layout();
    l->addLayout(ctx);
}

QString FloatingInputDialog::getText(const QString& title, const QString & text,
                                     bool *ok, QLineEdit::EchoMode mode)
{
    FloatingInputDialog *w = new FloatingInputDialog(title, ":/icons/edit");

    QVBoxLayout *l = (QVBoxLayout*)w->layout();

    QPointer<QLineEdit> line = new QLineEdit(w);
    line->setFont(Utils::getMonospaceFont());
    line->setStyleSheet("background-color: #1b1b1b; color: #ffffff; border: 1px solid #FF4444;");
    line->setEchoMode(mode);
    line->setText(text);
    line->selectAll();

    QPalette p;
    p.setColor(QPalette::Highlight, Qt::darkGray);
    line->setPalette(p);

    l->addWidget(line);

    w->show();
    w->move(QCursor::pos()-line->geometry().center());
    w->ensureOnScreen();
    line->setFocus();

    QEventLoop loop;
    connect(line.data(), SIGNAL(returnPressed()), &loop, SLOT(quit()));
    connect(w,           SIGNAL(destroyed()),     &loop, SLOT(quit()));
    loop.exec();

    if(ok)
        *ok = !line.isNull();

    QString res;
    if(!line.isNull())
    {
        res = line->text();
        w->deleteLater();
    }
    return res;
}

int FloatingInputDialog::getInt(const QString &title, int value, int min, int max, int step, bool *ok)
{
    FloatingInputDialog *w = new FloatingInputDialog(title, ":/icons/edit");

    QPointer<QSpinBox> box = new QSpinBox(w);
    box->setFont(Utils::getMonospaceFont());
    box->setStyleSheet("background-color: #1b1b1b; color: #ffffff; border: 1px solid #FF4444;");
    box->setRange(min, max);
    box->setValue(value);
    box->setSingleStep(step);

    QPalette p;
    p.setColor(QPalette::Highlight, Qt::darkGray);
    box->setPalette(p);

    QVBoxLayout *l = (QVBoxLayout*)w->layout();
    l->addWidget(box);

    w->show();
    w->move(QCursor::pos()-box->geometry().center());
    w->ensureOnScreen();
    box->setFocus();

    QEventLoop loop;
    connect(box.data(), SIGNAL(editingFinished()), &loop, SLOT(quit()));
    connect(w,          SIGNAL(destroyed()),     &loop, SLOT(quit()));
    loop.exec();

    if(ok)
        *ok = !box.isNull();

    int res = value;
    if(!box.isNull() && box->hasFocus())
    {
        res = box->value();
        w->deleteLater();
    }
    return res;
}

double FloatingInputDialog::getDouble(const QString &title, double value, double min, double max, int decimals, bool *ok)
{
    FloatingInputDialog *w = new FloatingInputDialog(title, ":/icons/edit");

    QPointer<QDoubleSpinBox> box = new QDoubleSpinBox(w);
    box->setFont(Utils::getMonospaceFont());
    box->setStyleSheet("background-color: #1b1b1b; color: #ffffff; border: 1px solid #FF4444;");
    box->setRange(min, max);
    box->setValue(value);
    box->setDecimals(decimals);

    QPalette p;
    p.setColor(QPalette::Highlight, Qt::darkGray);
    box->setPalette(p);

    QVBoxLayout *l = (QVBoxLayout*)w->layout();
    l->addWidget(box);

    w->show();
    w->move(QCursor::pos()-box->geometry().center());
    w->ensureOnScreen();
    box->setFocus();

    QEventLoop loop;
    connect(box.data(), SIGNAL(editingFinished()), &loop, SLOT(quit()));
    connect(w,          SIGNAL(destroyed()),     &loop, SLOT(quit()));
    loop.exec();

    if(ok)
        *ok = !box.isNull();

    double res = value;
    if(!box.isNull() && box->hasFocus())
    {
        res = box->value();
        w->deleteLater();
    }
    return res;
}

bool FloatingInputDialog::getIntRange(const QString &title, int &minVal,
                                      int &maxVal, int min, int max)
{
    FloatingInputDialog *w = new FloatingInputDialog(title, ":/icons/edit");

    QPointer<QLineEdit> minEdit, maxEdit;
    w->initRangeLayout(minEdit, maxEdit);

    minEdit->setValidator(new QIntValidator(min, max, w));
    maxEdit->setValidator(new QIntValidator(min, max, w));
    minEdit->setText(QString::number(minVal));
    maxEdit->setText(QString::number(maxVal));

    w->show();
    w->move(QCursor::pos()-minEdit->geometry().center());
    w->ensureOnScreen();
    minEdit->setFocus();

    QEventLoop loop;
    connect(minEdit.data(), SIGNAL(returnPressed()), &loop, SLOT(quit()));
    connect(maxEdit.data(), SIGNAL(returnPressed()), &loop, SLOT(quit()));
    connect(w,          SIGNAL(destroyed()),     &loop, SLOT(quit()));
    loop.exec();


    if (!minEdit.isNull() && !maxEdit.isNull() &&
       (minEdit->hasFocus() || maxEdit->hasFocus()))
    {
        w->deleteLater();

        int min = minEdit->text().toInt();
        int max = maxEdit->text().toInt();
        if(min > max)
            return false;
        minVal = min;
        maxVal = max;
        return true;
    }

    return false;
}

bool FloatingInputDialog::getDoubleRange(const QString &title, double &minVal,
                                         double &maxVal, int decimals, double min, double max)
{
    FloatingInputDialog *w = new FloatingInputDialog(title, ":/icons/edit");

    QPointer<QLineEdit> minEdit, maxEdit;
    w->initRangeLayout(minEdit, maxEdit);

    minEdit->setValidator(new QDoubleValidator(min, max, decimals, w));
    maxEdit->setValidator(new QDoubleValidator(min, max, decimals, w));
    minEdit->setText(QString::number(minVal));
    maxEdit->setText(QString::number(maxVal));

    w->show();
    w->move(QCursor::pos()-minEdit->geometry().center());
    w->ensureOnScreen();
    minEdit->setFocus();

    QEventLoop loop;
    connect(minEdit.data(), SIGNAL(returnPressed()), &loop, SLOT(quit()));
    connect(maxEdit.data(), SIGNAL(returnPressed()), &loop, SLOT(quit()));
    connect(w,          SIGNAL(destroyed()),     &loop, SLOT(quit()));
    loop.exec();


    if (!minEdit.isNull() && !maxEdit.isNull() &&
       (minEdit->hasFocus() || maxEdit->hasFocus()))
    {
        w->deleteLater();

        double min = minEdit->text().toDouble();
        double max = maxEdit->text().toDouble();
        if(min > max)
            return false;
        minVal = min;
        maxVal = max;
        return true;
    }

    return false;
}
