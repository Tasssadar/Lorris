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

#include "WorkTab/WorkTab.h"
#include "packet.h"
#include "DataWidgets/datawidget.h"

class QVBoxLayout;
class QHBoxLayout;
class QMdiArea;
class AnalyzerDataStorage;
class QSlider;
class DeviceTabWidget;
class AnalyzerDataArea;
class QSpinBox;
class QScrollArea;
struct analyzer_packet;

enum states_
{
    STATE_DISCONNECTED    = 0x01,
    STATE_DIALOG          = 0x02
};

enum hideable_areas
{
    AREA_TOP    = 0x01,
    AREA_RIGHT  = 0x02,
    AREA_LEFT   = 0x04
};

namespace Ui {
  class LorrisAnalyzer;
}

class LorrisAnalyzer : public WorkTab
{
    Q_OBJECT

    Q_SIGNALS:
        void newData(analyzer_data *data, quint32 index);

    public:
        explicit LorrisAnalyzer();
        virtual ~LorrisAnalyzer();

        bool isAreaVisible(quint8 area);
        void setAreaVisibility(quint8 area, bool visible);
        analyzer_data *getLastData(quint32& idx);

        Connection *getCon() { return m_con; }

        quint32 getCurrentIndex();

    public slots:
        void onTabShow();
        void updateData(bool ignoreTime = false);
        void widgetMouseStatus(bool in, const data_widget_info& info, qint32 parent);

    private slots:
        void connectButton();
        void saveDataButton();
        void clearAllButton();
        void clearDataButton();
        void openFile();
        void editStruture();

        void collapseTopButton();
        void collapseRightButton();
        void collapseLeftButton();

        void connectionResult(Connection*,bool);
        void connectedStatus(bool connected);
        void timeSliderMoved(int value);
        void timeBoxChanged(int value);

        void updateTimeChanged(int value);

    private:
        void readData(const QByteArray& data);
        void load(QString *name, quint8 mask);

        inline bool canUpdateUi(bool ignore = false)
        {
            return ignore || updateTime.elapsed() > minUpdateDelay;
        }

        quint16 m_state;
        bool highlightInfoNotNull;
        data_widget_info highlightInfo;
        Ui::LorrisAnalyzer *ui;
        AnalyzerDataStorage *m_storage;
        analyzer_packet *m_packet;
        analyzer_data *m_curData;

        QTime updateTime;
        int minUpdateDelay;
};

#endif // LORRISANALYZER_H
