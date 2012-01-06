#include <QFileDialog>
#include <QMessageBox>

#include "sourceselectdialog.h"
#include "ui_sourceselectdialog.h"
#include "analyzerdatastorage.h"

SourceSelectDialog::SourceSelectDialog(QWidget *parent) :
    QDialog(parent),ui(new Ui::SourceSelectDialog)
{
    ui->setupUi(this);

    QPushButton *cont = findChild<QPushButton*>("contButton");
    connect(cont, SIGNAL(clicked()), this, SLOT(contButton()));

    QPushButton *browseButton = findChild<QPushButton*>("browseButton");
    connect(browseButton, SIGNAL(clicked()), this, SLOT(browse()));

    QRadioButton *loadRadio = findChild<QRadioButton*>("loadRadio");
    connect(loadRadio, SIGNAL(toggled(bool)), this, SLOT(loadRadioToggled(bool)));
}

SourceSelectDialog::~SourceSelectDialog()
{
    delete ui;
}

void SourceSelectDialog::DisableNew()
{
    QRadioButton *newRadio = findChild<QRadioButton*>("newRadio");
    newRadio->setEnabled(false);

    QRadioButton *loadRadio = findChild<QRadioButton*>("loadRadio");
    loadRadio->setChecked(true);
}

void SourceSelectDialog::contButton()
{
    QRadioButton *newRadio = findChild<QRadioButton*>("newRadio");
    if(newRadio->isChecked())
    {
        accept();
        return;
    }
    QCheckBox *structBox = findChild<QCheckBox*>("structBox");
    QCheckBox *dataBox = findChild<QCheckBox*>("dataBox");
    QCheckBox *widgetBox = findChild<QCheckBox*>("widgetBox");

    if(!structBox->isChecked() && !dataBox->isChecked() && !widgetBox->isChecked())
    {
        QMessageBox *box = new QMessageBox();
        box->setWindowTitle(QObject::tr("Error!"));
        box->setText(QObject::tr("You have to select at least one thing to load."));
        box->setIcon(QMessageBox::Critical);
        box->exec();
        delete box;
        return;
    }

    accept();
}

qint8 SourceSelectDialog::get()
{
    if(exec() == QDialog::Rejected)
        return -1;

    QRadioButton *newRadio = findChild<QRadioButton*>("newRadio");
    return newRadio->isChecked();
}

quint8 SourceSelectDialog::getDataMask()
{
    quint8 res = 0;

    QCheckBox *structBox = findChild<QCheckBox*>("structBox");
    if(structBox->isChecked())
        res |= STORAGE_STRUCTURE;

    QCheckBox *dataBox = findChild<QCheckBox*>("dataBox");
    if(dataBox->isChecked())
        res |= STORAGE_DATA;

    QCheckBox *widgetBox = findChild<QCheckBox*>("widgetBox");
    if(widgetBox->isChecked())
        res |= STORAGE_WIDGETS;
    return res;
}

QString SourceSelectDialog::getFileName()
{
    QLineEdit *file = findChild<QLineEdit*>("fileEdit");
    return file->text();
}

void SourceSelectDialog::browse()
{
    QString filters = QObject::tr("Lorris data file (*.ldta)");
    QString filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Import Data"), "", filters);

    QPushButton *cont = findChild<QPushButton*>("contButton");
    cont->setEnabled(filename.length() != 0);

    if(filename.length() == 0)
        return;

    QLineEdit *file = findChild<QLineEdit*>("fileEdit");
    file->setText(filename);

}

void SourceSelectDialog::loadRadioToggled(bool toggle)
{
    bool enableCont = !toggle;
    if(toggle)
    {
        QLineEdit *file = findChild<QLineEdit*>("fileEdit");
        if(file->text().length() != 0)
            enableCont = true;
    }

    QPushButton *cont = findChild<QPushButton*>("contButton");
    cont->setEnabled(enableCont);
}
