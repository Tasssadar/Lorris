#ifndef ANALYZERWIDGET_H
#define ANALYZERWIDGET_H

#include <QMdiSubWindow>

class AnalyzerWidget : public QMdiSubWindow
{
public:
    AnalyzerWidget(QWidget *parent);
    ~AnalyzerWidget();
protected:
    void dragEnterEvent(QDragEnterEvent *event);
};

#endif // ANALYZERWIDGET_H
