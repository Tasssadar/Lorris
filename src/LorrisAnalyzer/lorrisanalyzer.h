/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISANALYZER_H
#define LORRISANALYZER_H

#include <QMutex>
#include <QTime>

#include "../WorkTab/WorkTab.h"
#include "packet.h"
#include "DataWidgets/datawidget.h"
#include "../ui/connectbutton.h"
#include "storage.h"
#include "packetparser.h"

class QVBoxLayout;
class QHBoxLayout;
class QMdiArea;
class Storage;
class QSlider;
class DeviceTabWidget;
class WidgetArea;
class QSpinBox;
class QScrollArea;

enum hideable_areas
{
    AREA_TOP    = 0x01,
    AREA_RIGHT  = 0x02,
    AREA_LEFT   = 0x04
};

namespace Ui {
  class LorrisAnalyzer;
}

class LorrisAnalyzer : public PortConnWorkTab
{
    Q_OBJECT

    Q_SIGNALS:
        void newData(analyzer_data *data, quint32 index);
        void setTitleVisibility(bool visible);
        void SendData(const QByteArray& data);

    public:
        explicit LorrisAnalyzer();
        virtual ~LorrisAnalyzer();

        QString GetIdString();

        bool isAreaVisible(quint8 area);
        void setAreaVisibility(quint8 area, bool visible);
        analyzer_data *getLastData(quint32& idx);

        quint32 getCurrentIndex();

        bool showTitleBars() const { return m_title_action->isChecked(); }

        void setPortConnection(ConnectionPointer<PortConnection> const & con);
        void openFile(const QString& filename);

        void saveData(DataFileParser *file);
        void loadData(DataFileParser *file);

    public slots:
        void onTabShow(const QString& filename);
        bool onTabClose();
        void updateData();
        void widgetMouseStatus(bool in, const data_widget_info& info, qint32 parent);
        void setDataChanged(bool changed = true) { m_data_changed = changed; }

        void addChildTab(ChildTab *tab, const QString& name);
        void removeChildTab(ChildTab *tab);

    private slots:
        void doNewSource();

        void saveButton();
        void saveAsButton();
        void exportBin();
        void importBinAct();
        void clearAllButton();
        void clearDataButton();
        void openFile();
        void editStruture();

        void collapseTopButton();
        void collapseRightButton();
        void collapseLeftButton();

        void connectedStatus(bool connected);
        void indexChanged(int value);
        void showTitleTriggered(bool checked);

        void updateForWidget();

    private:
        void readData(const QByteArray& data);
        bool load(QString& name, quint8 mask);
        void importBinary(const QString& filename, bool reset = true);
        void resetDevAndStorage(analyzer_packet *packet = NULL);
        void setPacket(analyzer_packet *packet);
        bool askToSave();

        bool highlightInfoNotNull;
        data_widget_info highlightInfo;
        Ui::LorrisAnalyzer *ui;
        Storage m_storage;
        analyzer_packet *m_packet;
        PacketParser m_parser;

        QAction *m_title_action;

        bool m_data_changed;
        qint32 m_curIndex;

        ConnectButton * m_connectButton;
        analyzer_data m_curData;
};

#endif // LORRISANALYZER_H
