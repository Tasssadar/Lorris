/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef GRAPHDIALOGS_H
#define GRAPHDIALOGS_H

#include <QDialog>

struct GraphCurveInfo;

namespace Ui {
    class GraphCurveAddDialog;
    class GraphCurveEditWidget;
}

class GraphCurveAddDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GraphCurveAddDialog(QWidget *parent, std::vector<GraphCurveInfo*> *curves, bool edit);
    ~GraphCurveAddDialog();

    QString getName();
    QString getColor();
    QString getCurrentCurve();
    quint8 getDataType();
    QString getEditName();

    bool forceEdit();
    bool edit();

private slots:
    void newOrEditCurve(bool newCurve);
    void colorChanged(int idx);
    void tryAccept();
    void curveChanged(int idx);

private:
    void showError(const QString& text);

    Ui::GraphCurveAddDialog *ui;
    Ui::GraphCurveEditWidget *edit_widget_ui;
    QWidget *m_edit_widget;
    std::vector<GraphCurveInfo*> *m_curves;
};

#endif // GRAPHDIALOGS_H
