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
class DataFilter;
class SearchWidget;

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
    void SendData(const QByteArray& data);
    void rawData(const QByteArray& data);
    void tinyWidgetBtn(bool tiny);

public:
    explicit LorrisAnalyzer();
    virtual ~LorrisAnalyzer();

    QString GetIdString();

    bool isAreaVisible(quint8 area);
    void setAreaVisibility(quint8 area, bool visible);
    analyzer_data *getLastData(quint32& idx);
    QByteArray *getDataAt(quint32 idx);
    analyzer_packet *getPacket() const { return m_packet; }
    void setEnableSearchWidget(bool enable);

    quint32 getCurrentIndex();

    void setPortConnection(ConnectionPointer<PortConnection> const & con);
    void openFile(const QString& filename);

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    DataFilter *getFilter(quint32 id);
    DataFilter *getFilterByOldInfo(const data_widget_infoV1& old_info) const;

public slots:
    void onTabShow(const QString& filename);
    bool onTabClose();
    void updateData();
    void widgetMouseStatus(bool in, const data_widget_info &, qint32 parent);
    void setDataChanged(bool changed = true) { m_data_changed = changed; }

    void addChildTab(ChildTab *tab, const QString& name);
    void removeChildTab(ChildTab *tab);

    void clearData();
    void editStructure();

protected:
    void keyPressEvent(QKeyEvent *ev);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void doNewSource();

    void saveButton();
    void saveAsButton();
    void exportBin();
    void importBinAct();
    void clearAllButton();
    void openFile();
    void setPacketLimit();

    void collapseTopButton();
    void collapseRightButton();
    void collapseLeftButton();

    void connectedStatus(bool connected);
    void indexChanged(int value);
    void onPacketLimitChanged(int limit);

    void updateForWidget();

private:
    void readData(const QByteArray& data);
    bool load(QString& name, quint8 mask);
    void importBinary(const QString& filename, bool reset = true);
    void resetDevAndStorage(analyzer_packet *packet = NULL);
    void setPacket(analyzer_packet *packet);
    bool askToSave();

    Ui::LorrisAnalyzer *ui;
    Storage m_storage;
    analyzer_packet *m_packet;
    PacketParser m_parser;

    bool m_data_changed;
    qint32 m_curIndex;

    ConnectButton * m_connectButton;
    analyzer_data m_curData;
    bool m_rightVisible;

    bool m_enableSearchWidget;
    SearchWidget *m_searchWidget;
};

#endif // LORRISANALYZER_H
