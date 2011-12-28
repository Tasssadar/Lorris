#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>

#include "sourcedialog.h"
#include "WorkTab/WorkTab.h"
#include "ui_sourcedialog.h"

SourceDialog::SourceDialog(QWidget *parent) :
    QDialog(parent),ui(new Ui::SourceDialog)
{
    ui->setupUi(this);
   /* setFixedSize(600, 250);

    layout = new QVBoxLayout(this);

    QPushButton *b_w_data = new QPushButton(tr("New source (with real data)"), this);
    b_w_data->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QPushButton *b_wo_data = new QPushButton(tr("New source (without real data)"), this);
    b_wo_data->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QPushButton *b_file = new QPushButton(tr("Load source structure from file"), this);
    b_file->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    layout->addWidget(b_w_data, 1);
    layout->addWidget(b_wo_data, 1);
    layout->addWidget(b_file, 1);*/
}

SourceDialog::~SourceDialog()
{
    delete ui;
    //WorkTab::DeleteAllMembers(layout);
    //delete layout;
}

