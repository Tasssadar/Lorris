/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QColor>
#include <QDialog>

#include "datawidget.h"
#include "ui_statusmanager.h"

class QSignalMapper;

class StatusWidget : public DataWidget
{
    Q_OBJECT

    struct status
    {
        QString text;
        QColor color;
        QColor textColor;
    };

    friend class StatusManager;
public:
    StatusWidget(QWidget *parent = 0);

    void setUp(Storage *storage);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    void addStatus(quint64 id, const QString& text, const QString& color, const QString& textColor);
    void removeStatus(quint64 id);
    void setValue(quint64 id);
    quint64 getValue() const { return m_status; }

    void setDataType(int type);

    void showStatusManager();

protected:
     void processData(analyzer_data *data);

private:
     QHash<quint64, status>& states() { return m_states; }
     void updateValue(quint64 id);
     void addStatus(quint64 id, const status& s);

     QLabel *m_label;
     QHash<quint64, status> m_states;
     quint64 m_status;

     QAction *m_typeAction[NUM_FLOAT];
     int m_dataType;
};

class StatusWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    StatusWidgetAddBtn(QWidget *parent = 0);
};

class StatusManager : public QDialog, private Ui::StatusManager
{
    Q_OBJECT
public:
    StatusManager(StatusWidget *widget);
    ~StatusManager();

private slots:
    void colorChanged(int row);
    void textColorChanged(int row);

    void on_table_itemChanged(QTableWidgetItem *item);
    void on_table_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *);
    void on_addBtn_clicked();
    void on_rmBtn_clicked();

private:
    inline StatusWidget *widget() const { return (StatusWidget*)parent(); }
    StatusWidget::status *getStatus(quint64 val);
    void updateItems();

    Ui::StatusManager *ui;
    QSignalMapper *m_clrMap;
    QSignalMapper *m_textClrMap;
    std::vector<quint64> m_rowVals;
};

#endif // STATUSWIDGET_H
