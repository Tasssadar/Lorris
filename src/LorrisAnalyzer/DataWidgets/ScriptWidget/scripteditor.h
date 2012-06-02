/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDialog>

namespace Ui {
    class ScriptEditor;
}

class QAbstractButton;
class LineNumber;

class ScriptEditor : public QDialog
{
    Q_OBJECT

Q_SIGNALS:
    void applySource(bool close);
    
public:
    explicit ScriptEditor(const QString& source, const QString &widgetName = 0);
    ~ScriptEditor();

    QString getSource();
    
private slots:
    void buttonPressed(QAbstractButton *btn);
    void textChanged();
    void sliderMoved(int val = -1);
    void rangeChanged(int, int);
    void loadFile();

private:
    Ui::ScriptEditor *ui;
    LineNumber *m_line_num;
};

class LineNumber : public QWidget
{
public:
    LineNumber(QWidget *parent = 0);

    void setLineNum(int lineNum);
    void setScroll(int line);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int m_char_h;
    int m_line_num;
    int m_scroll;
    quint8 m_last_w;
};

#endif // SCRIPTEDITOR_H
