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
class AnalyzerMdi;
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

    public:
        explicit LorrisAnalyzer();
        virtual ~LorrisAnalyzer();

    public slots:
        void onTabShow();

    private slots:
        void connectButton();
        void loadDataButton();

        void connectionResult(Connection*,bool);
        void connectedStatus(bool connected);
        void timeSliderMoved(int value);

    private:
        void readData(QByteArray data);

        quint16 m_state;
        Ui::LorrisAnalyzer *ui;
        AnalyzerDataStorage *m_storage;
        QSlider *timeSlider;
        analyzer_packet *m_packet;
        DeviceTabWidget *m_dev_tabs;
        AnalyzerMdi *m_mdi;
};

#endif // LORRISANALYZER_H
