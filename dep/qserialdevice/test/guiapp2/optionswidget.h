#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#include <QtGui/QWidget>

namespace Ui {
    class OptionsWidget;
}

class OptionsWidget : public QWidget
{
    Q_OBJECT
signals:
    void applyOptions(const QStringList &list);
public:
    class OptionsData {
    public:
        QString baud;
        QString data;
        QString parity;
        QString stop;
        QString flow;
        QString name;
    };
    explicit OptionsWidget(const QStringList baudList,
                           const QStringList dataList,
                           const QStringList parityList,
                           const QStringList stopList,
                           const QStringList flowList,
                           QWidget *parent = 0);
    ~OptionsWidget();

    void updateOptions(const OptionsData &data);

public slots:
    void procAppliedOptions(bool applied, const QStringList &notAppliedList);

protected:
    void showEvent(QShowEvent *e);

private slots:
    void procBoxChanged();
    void procApplyButtonClick();

private:
    Ui::OptionsWidget *ui;

    void fillingBoxes(const QStringList baudList,
                      const QStringList dataList,
                      const QStringList parityList,
                      const QStringList stopList,
                      const QStringList flowList);
    void initButtonConnections();
    void initBoxConnections();
};

#endif // OPTIONSWIDGET_H
