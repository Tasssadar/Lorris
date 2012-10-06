/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef FILTERTABWIDGET_H
#define FILTERTABWIDGET_H

#include <QTabWidget>
#include <QFrame>

#include "datafilter.h"
#include "lorrisanalyzer.h"
#include "ui_filterdialog.h"

class QScrollArea;
class QListWidget;
class QPushButton;
class QLineEdit;
class QComboBox;
class analyzer_data;
class EditorWidget;
class DataFileParser;
struct analyzer_header;
struct data_widget_info;

class FilterTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit FilterTabWidget(QWidget *parent = 0);
    ~FilterTabWidget();

    void reset(analyzer_header *header);
    void removeAll();

    void setHeader(analyzer_header *h);
    void Save(DataFileParser *file);
    void Load(DataFileParser *file, bool skip);
    void loadLegacy(DataFileParser *file);

    void addFilter(DataFilter *f);
    void removeFilter(DataFilter *f);
    quint32 generateId() { return m_filterIdCounter++; }

    quint32 getCurrFilterId() const;
    DataFilter *getCurrFilter() const;

    void setFilterName(DataFilter *f, const QString& name);

    DataFilter *getFilter(quint32 id) const;
    DataFilter *getFilterByOldInfo(const data_widget_infoV1& old_info) const;
    const std::vector<DataFilter*>& getFilters() const { return m_filters; }
    std::vector<DataFilter*>& getFilters() { return m_filters; }

    void sendLastData();

public slots:
    void handleData(analyzer_data *data, quint32 index);

private slots:
    void showSettings();
    void activateTab();

private:
    void addEmptyFilter();
    inline LorrisAnalyzer *analyzer() const { return (LorrisAnalyzer*)parent(); }

    analyzer_header *m_header;
    std::vector<DataFilter*> m_filters;
    quint32 m_filterIdCounter;
};

class FilterDialog : public QDialog, private Ui::FilterDialog
{
    Q_OBJECT
public:
    FilterDialog(QWidget *parent);
    ~FilterDialog();

private slots:
    void setCondVisibility(int cond);
    void scriptModified();

    void on_addBtn_clicked();
    void on_filterList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_condTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_addCondBtn_clicked();
    void on_typeBox_currentIndexChanged(int idx);
    void on_rmBtn_clicked();
    void on_rmCondBtn_clicked();
    void on_applyBtn_clicked();

    void on_nameEdit_textEdited(const QString& text);

    void on_devEdit_textEdited(const QString& text);
    void on_cmdEdit_textEdited(const QString& text);
    void on_byteValEdit_textEdited(const QString& text);
    void on_bytePosBox_valueChanged(int val);

private:
    FilterTabWidget *tabWidget() const { return (FilterTabWidget*)parent(); }
    QString getNewFilterName();
    void fillCondData(FilterCondition *c);
    FilterCondition *getCurrCondition();
    ConditionFilter *getCurrFilter();

    Ui::FilterDialog *ui;
    EditorWidget *m_editor;
};

#endif // FILTERTABWIDGET_H
