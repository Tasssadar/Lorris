#ifndef LORRISANALYZER_H
#define LORRISANALYZER_H

#include "WorkTab/WorkTab.h"

class QVBoxLayout;
class QHBoxLayout;
class QMdiArea;
class Parser;

enum states_
{
    STATE_DISCONNECTED    = 0x01,
    STATE_DIALOG          = 0x02,
};

class NewSourceDialog;

struct analyzer_packet
{
    QByteArray start;
    QByteArray stop;
    quint8 lenght;
};

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

        void dataStructure(analyzer_packet pkt, QByteArray curData);

    private:
        void readData(QByteArray data);

        QVBoxLayout *layout;
        QHBoxLayout *layout_area;
        QMdiArea *m_area;
        quint16 m_state;
        NewSourceDialog *dialog;
        Parser *m_parser;
};

#endif // LORRISANALYZER_H
