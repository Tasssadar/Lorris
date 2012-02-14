#include "scripteditor.h"
#include "ui_scripteditor.h"

ScriptEditor::ScriptEditor(const QString& source, const QString& widgetName) :
    QDialog(),
    ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);

    ui->sourceEdit->setPlainText(source);
    setWindowTitle(windowTitle() + widgetName);

    connect(ui->buttonBox, SIGNAL(accepted()), SIGNAL(okPressed()));
}

ScriptEditor::~ScriptEditor()
{
    delete ui;
}

QString ScriptEditor::getSource()
{
    return ui->sourceEdit->toPlainText();
}
