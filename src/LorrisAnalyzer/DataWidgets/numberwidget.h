#ifndef NUMBERWIDGET_H
#define NUMBERWIDGET_H

#include "datawidget.h"

class QMenu;

class NumberWidget : public DataWidget
{
    Q_OBJECT
public:
    NumberWidget(QWidget *parent = 0);
    ~NumberWidget();

    void setUp();

protected:
     void processData(analyzer_data *data);
     void contextMenuEvent ( QContextMenuEvent * event );

private slots:
     void fmtSelected(int i);
     void bitsSelected(int i);
     void levelSelected();

private:
     QLabel *num;
     bool sign;
     quint8 bytes;
     bool hex;
     bool level;
     QMenu *contextMenu;
     QMenu *bitsMenu;
     QMenu *formatMenu;
     QAction *bitsAction[8];
     QAction *fmtAction[2];
     QAction *levelAction;
};

class NumberWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    NumberWidgetAddBtn(QWidget *parent = 0);
    ~NumberWidgetAddBtn();

protected:
    QPixmap getRender();
};

#endif // NUMBERWIDGET_H
