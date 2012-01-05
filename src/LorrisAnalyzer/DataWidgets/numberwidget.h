#ifndef NUMBERWIDGET_H
#define NUMBERWIDGET_H

#include "datawidget.h"

class QMenu;

enum NumberFormats
{
    FMT_DECIMAL,
    FMT_EXPONENT,
    FMT_HEX,
    FMT_COUNT
};

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
     void resizeEvent ( QResizeEvent * event );

private slots:
     void fmtSelected(int i);
     void bitsSelected(int i);
     void levelSelected();

private:
     QLabel *num;
     quint8 numberType;
     quint8 format;
     bool level;
     QMenu *contextMenu;
     QAction *bitsAction[NUM_COUNT];
     QAction *fmtAction[FMT_COUNT];
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
