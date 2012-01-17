#include "graphdialogs.h"
#include "ui_graphcurveadddialog.h"
#include "ui_graphcurveeditwidget.h"

GraphCurveAddDialog::GraphCurveAddDialog(QWidget *parent) :
    QDialog(parent),ui(new Ui::GraphCurveAddDialog),
    edit_widget_ui(new Ui::GraphCurveEditWidget)
{
    ui->setupUi(this);

    m_edit_widget = new QWidget(this);
    edit_widget_ui->setupUi(m_edit_widget);

    ui->mainLayout->insertWidget(1, m_edit_widget);
}

GraphCurveAddDialog::~GraphCurveAddDialog()
{
    delete edit_widget_ui;
    delete m_edit_widget;
    delete ui;
}
