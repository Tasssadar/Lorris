#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include "analyzerwidget.h"

class TextWidget : public AnalyzerWidget
{
public:
    TextWidget(QWidget *parent);

protected:
    void dropEvent(QDropEvent *event);
};

#endif // TEXTWIDGET_H
