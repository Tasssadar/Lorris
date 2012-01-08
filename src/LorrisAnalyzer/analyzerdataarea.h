#ifndef ANALYZERDATAAREA_H
#define ANALYZERDATAAREA_H

#include <QFrame>
#include <map>
#include "DataWidgets/datawidget.h"

class AnalyzerDataFile;

class AnalyzerDataArea : public QFrame
{
    Q_OBJECT

Q_SIGNALS:
    void updateData();
    void mouseStatus(bool in, const data_widget_info& info);

public:
    typedef std::map<quint32, DataWidget*> w_map;

    explicit AnalyzerDataArea(QWidget *parent = 0);
    ~AnalyzerDataArea();

    void removeWidget(quint32 id);

    void SaveWidgets(AnalyzerDataFile *file);
    void LoadWidgets(AnalyzerDataFile *file, bool skip);
    static DataWidget *newWidget(quint8 type, QWidget *parent);
    
protected:
    void dropEvent ( QDropEvent * event );
    void dragEnterEvent( QDragEnterEvent *event );

private:
    DataWidget *addWidget(QPoint pos, quint8 type, bool show = true);

    quint32 getNewId() { return m_widgetIdCounter++; }

    void fixWidgetPos(QPoint& pos, QWidget *w);

    w_map m_widgets;
    quint32 m_widgetIdCounter;
};

#endif // ANALYZERDATAAREA_H
