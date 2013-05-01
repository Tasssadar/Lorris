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
Q_SIGNALS:
    void apply();

public:
    explicit GraphCurveAddDialog(QWidget *parent, std::vector<GraphCurveInfo*> *curves, bool edit);
    ~GraphCurveAddDialog();

    QString getName();
    QColor getColor();
    quint8 getDataType();
    QString getEditName();
    QString getFormula();

    bool forceEdit();
    bool edit();

private slots:
    void newOrEditCurve(bool newCurve);
    void tryAccept();
    void curveChanged(int idx);
    void selectColor();
    void buttonBoxClicked(QAbstractButton *btn);

private:
    void showError(const QString& text);
    void setButtonColor(const QColor& clr);

    Ui::GraphCurveAddDialog *ui;
    Ui::GraphCurveEditWidget *edit_widget_ui;
    QWidget *m_edit_widget;
    std::vector<GraphCurveInfo*> *m_curves;
    QColor m_color;
};

#endif // GRAPHDIALOGS_H
