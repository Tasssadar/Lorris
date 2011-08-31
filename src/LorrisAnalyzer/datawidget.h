#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <QWidget>
#include <QLabel>

class QVBoxLayout;

class DataWidget : public QWidget
{
    Q_OBJECT
public:
    DataWidget(QWidget *parent);
    ~DataWidget();

private:
    QVBoxLayout *layout;
};

class DataLabel : public QLabel
{
    Q_OBJECT
public:
    DataLabel(const QString & text, QWidget *parent);
    DataLabel(QWidget *parent)
    {
        DataLabel("", parent);
    }
    ~DataLabel();

protected:
    void mousePressEvent ( QMouseEvent * event );

};

#endif // DATAWIDGET_H
