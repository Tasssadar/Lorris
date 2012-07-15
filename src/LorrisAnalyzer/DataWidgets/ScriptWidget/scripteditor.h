/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDialog>
#include <QTimer>
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
    ScriptEditor(const QString& source, const QString& filename, int type, const QString &widgetName = 0);
    ~ScriptEditor();

    QString getSource();
    int getEngine();

    QString getFilename() const
    {
        return m_filename;
    }

public slots:
    void addError(const QString& error);
    void clearErrors();
    void reject();

private slots:
    void sliderMoved(int val = -1);
    void rangeChanged(int, int);

    void on_buttonBox_clicked(QAbstractButton *btn);
    void on_sourceEdit_textChanged();
    void on_loadBtn_clicked();
    void on_langBox_currentIndexChanged(int idx);
    void on_errorBtn_toggled(bool checked);
    void on_exampleBox_activated(int index);
    void on_saveBtn_clicked();

    void contentsChange(int position, int charsRemoved, int charsAdded);
    void saveAs();
    void clearStatus() { ui->statusLabel->setText(QString()); }
    void checkChange();
    void focusChanged(QWidget *prev, QWidget *now);

private:
    bool save(const QString& file);
    void updateExampleList();
    void setStatus(const QString& status);
    void setFilename(const QString& filename);

    Ui::ScriptEditor *ui;
    LineNumber *m_line_num;
    bool m_changed;
    bool m_contentChanged;
    bool m_ignoreNextFocus;
    bool m_ignoreFocus;
    quint32 m_errors;

    QSyntaxHighlighter *m_highlighter;

    QString m_filename;
    QTimer m_status_timer;
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
