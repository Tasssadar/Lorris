#ifndef LORRISANALYZER_H
#define LORRISANALYZER_H

#include "WorkTab/WorkTab.h"

class QVBoxLayout;
class QMdiArea;

enum states_
{
    STATE_DISCONNECTED    = 0x01,
    STATE_DIALOG          = 0x02,
};

class NewSourceDialog;

class LorrisAnalyzer : public WorkTab
{
    Q_OBJECT
    public:
        explicit LorrisAnalyzer();
        virtual ~LorrisAnalyzer();

    private slots:
        void connectButton();
        void newSourceButton();

        void connectionResult(Connection*,bool);
        void connectedStatus(bool connected);

    private:
        void readData(QByteArray data);

        QVBoxLayout *layout;
        QMdiArea *m_area;
        quint16 m_state;
        NewSourceDialog *dialog;
};

#endif // LORRISANALYZER_H
