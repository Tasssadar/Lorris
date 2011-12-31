#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <QMdiSubWindow>


class DataWidget : public QMdiSubWindow
{
    Q_OBJECT
public:
    explicit DataWidget(QWidget *parent = 0);
    ~DataWidget();

signals:

public slots:

};

#endif // DATAWIDGET_H
