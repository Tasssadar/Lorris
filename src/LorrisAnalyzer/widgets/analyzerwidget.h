#ifndef ANALYZERWIDGET_H
#define ANALYZERWIDGET_H

#include <QMdiSubWindow>

class QHBoxLayout;

class AnalyzerWidget : public QMdiSubWindow
{
    Q_OBJECT
Q_SIGNALS:
    void connectLabel(AnalyzerWidget *widget, int id);

public:
    virtual ~AnalyzerWidget();

public slots:
    virtual void textChanged(QString text, int id);

protected:
    explicit AnalyzerWidget(QWidget *parent);

    void dragEnterEvent(QDragEnterEvent *event);

    QHBoxLayout *layout;
};

#endif // ANALYZERWIDGET_H
