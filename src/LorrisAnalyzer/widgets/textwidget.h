#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include "analyzerwidget.h"

class TextWidget : public AnalyzerWidget
{
   Q_OBJECT
public:
    explicit TextWidget(QWidget *parent);
    virtual ~TextWidget();

public slots:
    void textChanged(QString text, int id);

protected:
    void dropEvent(QDropEvent *event);


};

#endif // TEXTWIDGET_H
