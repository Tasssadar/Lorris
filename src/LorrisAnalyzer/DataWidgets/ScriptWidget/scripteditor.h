/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDialog>
#include "ui_scripteditor.h"

class QAbstractButton;
class LineNumber;
class QSyntaxHighlighter;

class ScriptEditor : public QDialog, private Ui::ScriptEditor
{
    Q_OBJECT

Q_SIGNALS:
    void applySource(bool close);
    
public:
    ScriptEditor(const QString& source, int type, const QString &widgetName = 0);
    ~ScriptEditor();

    QString getSource();
    int getEngine();

public slots:
    void addError(const QString& error);
    void clearErrors();

private slots:
    void sliderMoved(int val = -1);
    void rangeChanged(int, int);

    void on_buttonBox_clicked(QAbstractButton *btn);
    void on_sourceEdit_textChanged();
    void on_loadBtn_clicked();
    void on_langBox_currentIndexChanged(int idx);
    void on_errorBtn_toggled(bool checked);

    void contentsChange(int position, int charsRemoved, int charsAdded);

private:
    Ui::ScriptEditor *ui;
    LineNumber *m_line_num;
    bool m_changed;
    quint32 m_errors;

    QSyntaxHighlighter *m_highlighter;
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
