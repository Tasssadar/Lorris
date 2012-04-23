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

#include <QTimer>

#include "../common.h"
#include "playback.h"
#include "ui_playback.h"

Playback::Playback(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Playback)
{
    ui->setupUi(this);

    m_playing = false;
    m_timer = new QTimer(this);
    m_index = 0;
    m_pause = false;

    ui->delayBox->setValue(sConfig.get(CFG_QUINT32_ANALYZER_PLAY_DEL));

    connect(ui->startBtn,   SIGNAL(clicked()),         SLOT(startBtn()));
    connect(ui->stopButton, SIGNAL(clicked()),         SLOT(stopPlayback()));
    connect(ui->delayBox,   SIGNAL(valueChanged(int)), SLOT(delayChanged(int)));
    connect(m_timer,        SIGNAL(timeout()),         SLOT(timeout()));
}

Playback::~Playback()
{
    sConfig.set(CFG_QUINT32_ANALYZER_PLAY_DEL, ui->delayBox->value());

    delete m_timer;
    delete ui;
}

void Playback::valChanged(int val)
{
    if(ui->startBox->isEnabled())
        ui->startBox->setValue(val);
}

void Playback::rangeChanged(int /*min*/, int max)
{
    ui->startBox->setMaximum(max);
}

void Playback::startBtn()
{
    if(!m_playing)
    {
        if(ui->startBox->value() == ui->startBox->maximum())
            return;

        ui->startBtn->setText(tr("Pause"));
        m_playing = true;

        m_index = ui->startBox->value();

        emit enablePosSet(false);

        m_timer->start(ui->delayBox->value());

        enableUi(false);
    }
    else
    {
        m_pause = !m_pause;

        if(m_pause)
        {
            ui->startBtn->setText(tr("Start"));
            m_timer->stop();
        }
        else
        {
            ui->startBtn->setText(tr("Pause"));
            m_timer->start();
        }
    }
}

void Playback::stopPlayback()
{
    m_playing = false;
    m_timer->stop();

    emit enablePosSet(true);
    ui->startBtn->setText(tr("Start"));

    enableUi(true);
}

void Playback::enableUi(bool enable)
{
    ui->startBox->setEnabled(enable);
    ui->cntBox->setEnabled(enable);
    ui->repeatBox->setEnabled(enable);
    ui->reverseBox->setEnabled(enable);
    ui->stopButton->setEnabled(!enable);
}

void Playback::timeout()
{
    bool exit = updateIndex();

    emit setPos(m_index);

    if(exit && ui->repeatBox->isChecked())
    {
        exit = false;
        m_index = ui->startBox->value();
    }

    if(exit)
        stopPlayback();
    else
        m_timer->start();
}

bool Playback::updateIndex()
{
    if(!ui->reverseBox->isChecked())
    {
        ++m_index;

        if(m_index == ui->startBox->maximum())
            return true;

        if(ui->cntBox->value() && m_index - ui->startBox->value() >= ui->cntBox->value())
            return true;
    }
    else
    {
        --m_index;

        if(m_index < 0)
            return true;

        if(ui->cntBox->value() && ui->startBox->value() - m_index >= ui->cntBox->value())
            return true;
    }
    return false;
}

void Playback::delayChanged(int val)
{
    if(m_playing && !m_pause)
        m_timer->start(val);
    else
        m_timer->setInterval(val);
}
