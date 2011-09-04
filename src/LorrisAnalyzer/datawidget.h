#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <QWidget>
#include <QLabel>



class QVBoxLayout;
class QHBoxLayout;
class AnalyzerWidget;

class DataWidget : public QWidget
{
    Q_OBJECT
public:
    DataWidget(QWidget *parent);
    ~DataWidget();

    void setSize(quint16 size);
    quint16 getSize() { return m_size; }

    void newData(QByteArray data);

public slots:
    void connectLabel(AnalyzerWidget *widget, int id);

private:
    QVBoxLayout *layout;
    QHBoxLayout *first_line;
    QHBoxLayout *second_line;

    quint16 m_size;
};

class DataLabel : public QLabel
{
    Q_OBJECT

Q_SIGNALS:
    void textChanged(QString value, int id);

public:
    DataLabel(const QString & text, QWidget *parent);
    DataLabel(QWidget *parent)
    {
        DataLabel("", parent);
    }
    ~DataLabel();

public slots:
    void setText ( const QString & );

protected:
    void mousePressEvent ( QMouseEvent * event );

};

#endif // DATAWIDGET_H
