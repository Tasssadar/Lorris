#ifndef LORRISANALYZER_H
#define LORRISANALYZER_H

#include "WorkTab/WorkTab.h"
#include "packet.h"

class QVBoxLayout;
class QHBoxLayout;
class QMdiArea;
class AnalyzerDataStorage;
class QSlider;
class DeviceTabWidget;
class AnalyzerDataArea;
class QSpinBox;
struct analyzer_packet;

enum states_
{
    STATE_DISCONNECTED    = 0x01,
    STATE_DIALOG          = 0x02
};

namespace Ui {
  class LorrisAnalyzer;
}

class LorrisAnalyzer : public WorkTab
{
    Q_OBJECT

    Q_SIGNALS:
        void newData(analyzer_data *data);

    public:
        explicit LorrisAnalyzer();
        virtual ~LorrisAnalyzer();

    public slots:
        void onTabShow();
        void updateData();

    protected:
        void mouseMoveEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);
        void paintEvent(QPaintEvent *event);

    private slots:
        void connectButton();
        void loadDataButton();
        void saveDataButton();

        void connectionResult(Connection*,bool);
        void connectedStatus(bool connected);
        void timeSliderMoved(int value);
        void timeBoxChanged(int value);

    private:
        void readData(const QByteArray& data);
        void load(QString *name, quint8 mask);

        quint16 m_state;
        bool shiftPressed;
        bool lineShowed;
        Ui::LorrisAnalyzer *ui;
        AnalyzerDataStorage *m_storage;
        QSlider *timeSlider;
        QSpinBox *timeBox;
        analyzer_packet *m_packet;
        DeviceTabWidget *m_dev_tabs;
        AnalyzerDataArea *m_data_area;
        QLine connectLine;
};

#endif // LORRISANALYZER_H
