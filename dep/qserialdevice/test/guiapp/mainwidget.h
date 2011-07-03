#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtGui/QWidget>
#include <QtCore/QDateTime>

namespace Ui {
    class MainWidget;
}

class SerialDeviceEnumerator;
class AbstractSerial;
class InfoWidget;
class OptionsWidget;
class TraceWidget;
class MainWidget : public QWidget
{
    Q_OBJECT
signals:
    void optionsApplied(bool applied, const QStringList &notAppliedList);

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

protected:
    void changeEvent(QEvent *e);

private slots:
    //
    void procEnumerate(const QStringList &l);
    void procSerialMessages(const QString &msg, QDateTime dt);
    void procSerialDataReceive();
    void procSerialDataTransfer(const QByteArray &data);
    void procCtsChanged(bool val);
    void procDsrChanged(bool val);
    void procRingChanged(bool val);
    void procApplyOptions(const QStringList &list);
    // Proc buttons click
    void procControlButtonClick();
    void procOptionsButtonClick();
    void procInfoButtonClick();
    void procIOButtonClick();

    void procRtsButtonClick();
    void procDtrButtonClick();
    // Proc box item change
    void procBoxChange(const QString &item);

private:
    Ui::MainWidget *ui;
    InfoWidget *infoWidget;
    OptionsWidget *optionsWidget;
    TraceWidget *traceWidget;

    SerialDeviceEnumerator *enumerator;
    AbstractSerial *serial;

    void initMainWidgetCloseState();
    void initMainWidgetOpenState();
    bool initInfoWidget();
    bool initOptionsWidget();
    bool initTraceWidget();
    void initEnumerator();
    void deinitEnumerator();
    void initSerial();
    void deinitSerial();
    void initButtonConnections();
    void initBoxConnections();
    void deinitWidgets();
    void setRtsDtrButtonsCaption(bool opened, bool rts, bool dtr);
    void detectSerialLineStates();
    void updateInfoData(const QString &name);
    void updateOptionsData();
};

#endif // MAINWIDGET_H
