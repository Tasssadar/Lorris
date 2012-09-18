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
#include <vector>
#include <QLinkedList>

#include "datawidget.h"
#include "ui_statusmanager.h"

class QSignalMapper;

class StatusWidget : public DataWidget
{
    Q_OBJECT

    struct status
    {
        quint64 id;
        bool mask;

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
    /// \deprecated Use addStatus(quint64 id, bool mask, const QString& text, const QString& color, const QString& textColor)
    void addStatus(quint64 id, const QString& text, const QString& color, const QString& textColor)
    {
        addStatus(id, false, text, color, textColor);
    }

    void addStatus(quint64 id, bool mask, const QString& text, const QString& color, const QString& textColor);
    void removeStatus(quint64 id, bool mask = false);
    void setValue(quint64 id)
    {
        setValue(id, false);
    }
    quint64 getValue() const { return m_lastVal; }

    void setUnknownText(const QString& text);
    void setUnknownColor(const QString& color);
    void setUnknownTextColor(const QString& color);

    void setDataType(int type);

    void showStatusManager();

protected:
     void processData(analyzer_data *data);

private:
     QLinkedList<status>& states() { return m_states; }
     status& unknown() { return m_unknown; }
     void updateActive();
     void updateUnknown();
     void updateLabels();
     void addStatus(const status& s);
     void setFromStatus(status *st, QLabel *label);
     void setValue(quint64 id, bool force);

     std::vector<QLabel*> m_labels;
     QLinkedList<status> m_states;
     QLinkedList<status*> m_active;
     QLabel *m_emptyLabel;
     status m_unknown;
     quint64 m_lastVal;

     QAction *m_typeAction[NUM_FLOAT];
     int m_dataType;

     bool m_curUnknown;
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
    void on_unkText_editingFinished();
    void on_unkColor_colorChanged(const QColor& color);
    void on_unkTextColor_colorChanged(const QColor& color);

private:
    inline StatusWidget *widget() const { return (StatusWidget*)parent(); }
    StatusWidget::status *getStatus(quint64 val, bool mask);
    void updateItems();

    Ui::StatusManager *ui;
    QSignalMapper *m_clrMap;
    QSignalMapper *m_textClrMap;
    std::vector<std::pair<quint64, bool> > m_rowVals;
};

#endif // STATUSWIDGET_H
