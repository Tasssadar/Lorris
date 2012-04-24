/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef LORRISANALYZER_H
#define LORRISANALYZER_H

#include <QMutex>
#include <QTime>

#include "../WorkTab/WorkTab.h"
#include "packet.h"
#include "DataWidgets/datawidget.h"
#include "../ui/connectbutton.h"

class QVBoxLayout;
class QHBoxLayout;
class QMdiArea;
class AnalyzerDataStorage;
class QSlider;
class DeviceTabWidget;
class AnalyzerDataArea;
class QSpinBox;
class QScrollArea;
class PacketParser;
struct analyzer_packet;

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

        bool isAreaVisible(quint8 area);
        void setAreaVisibility(quint8 area, bool visible);
        analyzer_data *getLastData(quint32& idx);

        Connection *getCon() { return m_con; }

        quint32 getCurrentIndex();

        bool showTitleBars() const { return m_title_action->isChecked(); }

        void setConnection(PortConnection *con);

    public slots:
        void onTabShow();
        bool onTabClose();
        void updateData();
        void widgetMouseStatus(bool in, const data_widget_info& info, qint32 parent);
        void setDataChanged(bool changed = true) { m_data_changed = changed; }

    private slots:
        void doNewSource();

        void saveButton();
        void saveAsButton();
        void clearAllButton();
        void clearDataButton();
        void openFile();
        void editStruture();

        void collapseTopButton();
        void collapseRightButton();
        void collapseLeftButton();

        void connectionResult(Connection*,bool);
        void connectedStatus(bool connected);
        void indexChanged(int value);
        void showTitleTriggered(bool checked);

    private:
        void readData(const QByteArray& data);
        void load(QString *name, quint8 mask);

        bool highlightInfoNotNull;
        data_widget_info highlightInfo;
        Ui::LorrisAnalyzer *ui;
        AnalyzerDataStorage *m_storage;
        analyzer_packet *m_packet;
        PacketParser *m_parser;

        QAction *m_title_action;

        bool m_data_changed;
        qint32 m_curIndex;

        ConnectButton * m_connectButton;
};

#endif // LORRISANALYZER_H
