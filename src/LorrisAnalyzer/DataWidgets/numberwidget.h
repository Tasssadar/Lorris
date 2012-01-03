#ifndef NUMBERWIDGET_H
#define NUMBERWIDGET_H

#include "datawidget.h"

class QMenu;

enum NumberTypes
{
    NUM_UINT8,
    NUM_UINT16,
    NUM_UINT32,
    NUM_UINT64,

    NUM_INT8,
    NUM_INT16,
    NUM_INT32,
    NUM_INT64,

    NUM_FLOAT,
    NUM_DOUBLE,

    NUM_COUNT
};

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
     QMenu *bitsMenu;
     QMenu *formatMenu;
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
