/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef LORRISAVR_H
#define LORRISAVR_H

#include "../WorkTab/WorkTab.h"
#include "mcu.h"
#include "ui_lorrisavr.h"

#include <vector>

class LorrisAVR : public WorkTab, private Ui::LorrisAVR
{
    Q_OBJECT
public:
    explicit LorrisAVR();
    ~LorrisAVR();

    QString GetIdString();

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
