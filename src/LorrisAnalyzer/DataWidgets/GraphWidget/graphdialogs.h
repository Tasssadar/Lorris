#ifndef GRAPHDIALOGS_H
#define GRAPHDIALOGS_H

#include <QDialog>

namespace Ui {
    class GraphCurveAddDialog;
    class GraphCurveEditWidget;
}

class GraphCurveAddDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GraphCurveAddDialog(QWidget *parent = 0);
    ~GraphCurveAddDialog();

private:
    Ui::GraphCurveAddDialog *ui;
    Ui::GraphCurveEditWidget *edit_widget_ui;
    QWidget *m_edit_widget;
};

#endif // GRAPHDIALOGS_H
