#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include <QtGui/QWidget>

namespace Ui {
    class TraceWidget;
}

class TraceWidget : public QWidget
{
    Q_OBJECT

signals:
    void sendSerialData(const QByteArray &data);

public:
    explicit TraceWidget(QWidget *parent = 0);
    ~TraceWidget();

    void setTitle(const QString &name);

public slots:
    void printTrace(const QByteArray &data, bool directionRx);

protected:
    void changeEvent(QEvent *e);

private slots:
    void procSendButtonClick();
    void procClearButtonClick();

private:
    Ui::TraceWidget *ui;
};

#endif // TRACEWIDGET_H
