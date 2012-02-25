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

#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <QFrame>

class QTimer;

namespace Ui {
    class Playback;
}

class Playback : public QFrame

{
    Q_OBJECT
    
Q_SIGNALS:
    void enablePosSet(bool enable);
    void setPos(int pos);

public:
    explicit Playback(QWidget *parent = 0);
    ~Playback();
    
public slots:
    void rangeChanged(int min, int max);
    void valChanged(int val);

private slots:
    void startBtn();
    void timeout();
    void delayChanged(int val);

    void stopPlayback();

private:
    void enableUi(bool enable);
    bool updateIndex();

    bool m_playing;
    bool m_pause;
    int m_index;

    QTimer *m_timer;
    Ui::Playback *ui;
};

#endif // PLAYBACK_H
