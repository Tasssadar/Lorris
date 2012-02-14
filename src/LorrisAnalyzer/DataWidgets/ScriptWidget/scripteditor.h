#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDialog>

namespace Ui {
class ScriptEditor;
}

class ScriptEditor : public QDialog
{
    Q_OBJECT

Q_SIGNALS:
    void okPressed();
    
public:
    explicit ScriptEditor(const QString& source, const QString &widgetName = 0);
    ~ScriptEditor();

    QString getSource();
    
private:
    Ui::ScriptEditor *ui;
};

#endif // SCRIPTEDITOR_H
