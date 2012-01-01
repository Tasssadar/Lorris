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
    
protected:
    void dropEvent ( QDropEvent * event );
    void dragEnterEvent( QDragEnterEvent *event );

private:
    quint32 getNewId() { return m_widgetIdCounter++; }
    DataWidget *newWidget(quint8 type);

    w_map m_widgets;
    quint32 m_widgetIdCounter;
};

#endif // ANALYZERDATAAREA_H
