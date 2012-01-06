#ifndef ANALYZERDATAAREA_H
#define ANALYZERDATAAREA_H

#include <QFrame>
#include <map>
#include "DataWidgets/datawidget.h"

class AnalyzerDataArea : public QFrame
{
    Q_OBJECT

Q_SIGNALS:
    void updateData();

public:
    typedef std::map<quint32, DataWidget*> w_map;

    explicit AnalyzerDataArea(QWidget *parent = 0);
    ~AnalyzerDataArea();

    void removeWidget(quint32 id);

    void SaveWidgets(QFile *file);
    void LoadWidgets(QFile *file, bool skip);

    DataWidget *isMouseInWidget();
    
protected:
    void dropEvent ( QDropEvent * event );
    void dragEnterEvent( QDragEnterEvent *event );

private:
    DataWidget *newWidget(quint8 type);
    DataWidget *addWidget(QPoint pos, quint8 type, bool show = true);

    quint32 getNewId() { return m_widgetIdCounter++; }

    void fixWidgetPos(QPoint& pos, QWidget *w);

    w_map m_widgets;
    quint32 m_widgetIdCounter;
};

#endif // ANALYZERDATAAREA_H
