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

#include <QMessageBox>

#include "../datawidget.h"
#include "graphdialogs.h"
#include "graphwidget.h"
#include "graphcurve.h"
#include "ui_graphcurveadddialog.h"
#include "ui_graphcurveeditwidget.h"

GraphCurveAddDialog::GraphCurveAddDialog(QWidget *parent, std::vector<GraphCurveInfo*> *curves, bool edit) :
    QDialog(parent),ui(new Ui::GraphCurveAddDialog),
    edit_widget_ui(new Ui::GraphCurveEditWidget)
{
    ui->setupUi(this);

    m_edit_widget = new QWidget(this);
    edit_widget_ui->setupUi(m_edit_widget);

    ui->mainLayout->insertWidget(1, m_edit_widget);
    newOrEditCurve(true);

    connect(ui->newRadio,             SIGNAL(toggled(bool)),                SLOT(newOrEditCurve(bool)));
    connect(edit_widget_ui->colorBox, SIGNAL(currentIndexChanged(int)),     SLOT(colorChanged(int)));
    connect(ui->buttonBox,            SIGNAL(accepted()),                   SLOT(tryAccept()));
    connect(edit_widget_ui->curveBox, SIGNAL(currentIndexChanged(int)),     SLOT(curveChanged(int)));

    static const QString dataTypes[] =
    {
        tr("unsigned 8bit"),
        tr("unsigned 16bit"),
        tr("unsigned 32bit"),
        tr("unsigned 64bit"),

        tr("signed 8bit"),
        tr("signed 16bit"),
        tr("signed 32bit"),
        tr("signed 64bit"),

        tr("float (4 bytes)"),
        tr("double (8 bytes)")
    };

    for(quint8 i = 0; i < NUM_COUNT; ++i)
        edit_widget_ui->dataTypeBox->addItem(dataTypes[i]);

    m_curves = curves;

    ui->setRadio->setEnabled(!m_curves->empty());

    if(edit)
    {
        ui->setRadio->setChecked(true);
        ui->setRadio->setVisible(false);
        ui->newRadio->setVisible(false);
    }
}

GraphCurveAddDialog::~GraphCurveAddDialog()
{
    delete edit_widget_ui;
    delete m_edit_widget;
    delete ui;
}

void GraphCurveAddDialog::newOrEditCurve(bool newCurve)
{
    if(!newCurve && edit_widget_ui->curveBox->count() == 0)
        for(std::vector<GraphCurveInfo*>::iterator i = m_curves->begin(); i != m_curves->end(); ++i)
            edit_widget_ui->curveBox->addItem((*i)->curve->title().text());

    edit_widget_ui->curveLabel->setVisible(!newCurve);
    edit_widget_ui->curveBox->setVisible(!newCurve);
}

QString GraphCurveAddDialog::getName()
{
    return edit_widget_ui->nameEdit->text();
}

QString GraphCurveAddDialog::getEditName()
{
    return edit_widget_ui->curveBox->currentText();
}

QString GraphCurveAddDialog::getColor()
{
    if(edit_widget_ui->colorBox->currentIndex() == 0)
        return edit_widget_ui->ColorEdit->text();
    else
        return edit_widget_ui->colorBox->currentText();
}

QString GraphCurveAddDialog::getCurrentCurve()
{
    return "";
}

quint8 GraphCurveAddDialog::getDataType()
{
    return edit_widget_ui->dataTypeBox->currentIndex();
}

void GraphCurveAddDialog::colorChanged(int idx)
{
    edit_widget_ui->ColorEdit->setEnabled(idx == 0);
}

void GraphCurveAddDialog::showError(const QString& text)
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Critical);
    box.setWindowTitle(tr("Error!"));
    box.setText(text);
    box.exec();
}

void GraphCurveAddDialog::tryAccept()
{
    if(edit_widget_ui->nameEdit->text().isEmpty())
    {
        showError(tr("You have to set name!"));
        return;
    }

    if(edit_widget_ui->colorBox->currentIndex() == 0 &&
       edit_widget_ui->ColorEdit->text().isEmpty())
    {
        showError(tr("You have to set color!"));
        return;
    }

    for(std::vector<GraphCurveInfo*>::iterator i = m_curves->begin(); i != m_curves->end(); ++i)
    {
        QString title = (*i)->curve->title().text();
        QString newName = edit_widget_ui->nameEdit->text();

        if((!edit() || title != edit_widget_ui->curveBox->currentText()) &&
           title.compare(newName, Qt::CaseInsensitive) == 0)
        {
            showError(tr("This name is already taken"));
            return;
        }
    }

    accept();
}

bool GraphCurveAddDialog::forceEdit()
{
    return ui->newRadio->isHidden();
}

bool GraphCurveAddDialog::edit()
{
    return ui->setRadio->isChecked();
}

void GraphCurveAddDialog::curveChanged(int idx)
{
    if((quint32)idx >= m_curves->size())
        return;

    GraphCurve* curve = m_curves->at(idx)->curve;
    edit_widget_ui->nameEdit->setText(curve->title().text());
    edit_widget_ui->dataTypeBox->setCurrentIndex(curve->getDataType());
}
