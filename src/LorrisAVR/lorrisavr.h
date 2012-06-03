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

#ifndef LORRISAVR_H
#define LORRISAVR_H

#include "WorkTab/WorkTab.h"
#include "mcu.h"
#include "ui_lorrisavr.h"

#include <vector>

class LorrisAVR : public WorkTab, private Ui::LorrisAVR
{
    Q_OBJECT
public:
    explicit LorrisAVR();
    ~LorrisAVR();

private slots:
    void on_browseBtn_clicked();
    void on_startBtn_clicked();
    void on_pauseBtn_clicked();
    void on_freqBox_valueChanged(int i);

    void realFreq(quint32 freq);
    void hackToTerm(char c);

private:
    MCU m_mcu;
    Ui::LorrisAVR *ui;
    std::vector<mcu_prototype*> m_prototypes;
};

#endif // LORRISAVR_H
