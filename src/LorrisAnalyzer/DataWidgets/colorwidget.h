#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include "datawidget.h"

class ColorWidget : public DataWidget
{
    Q_OBJECT
public:
    ColorWidget(QWidget *parent = 0);
    ~ColorWidget();

    void setUp();
    void saveWidgetInfo(AnalyzerDataFile *file);
    void loadWidgetInfo(AnalyzerDataFile *file);

protected:
     void processData(analyzer_data *data);

private slots:

private:
     QWidget *m_widget;
};

class ColorWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    ColorWidgetAddBtn(QWidget *parent = 0);

protected:
    QPixmap getRender();
};
#endif // COLORWIDGET_H
