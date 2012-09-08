/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef MINISHUPITOUI_H
#define MINISHUPITOUI_H

#include "shupitoui.h"

#include "ui_minishupitoui.h"

class MiniShupitoUI : public ShupitoUI
{
    Q_OBJECT
public:
    explicit MiniShupitoUI(QObject *parent = 0);
    ~MiniShupitoUI();

    void setupUi(LorrisShupito *widget);
    void setChipId(const QString &text);
    void setFileAndTime(const QString &file, const QDateTime &time);

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    void setHexData(quint32 memid, const QByteArray& data)
    {
        m_hexData[memid] = data;
    }

    QByteArray getHexData(quint32 memid) const
    {
        return m_hexData[memid];
    }

protected:
    QToolButton *startStopBtn() const { return ui->startStopBtn; }
    QBoxLayout *vddLayout() const { return ui->vddLayout; }
    QLabel *engineLabel() const { return ui->engineLabel; }
    QLabel *vccLabel() const { return ui->vccLabel; }

private slots:
    void enableWrite(bool enable);

private:
    Ui::MiniShupitoUI *ui;

    QByteArray m_hexData[MEM_FUSES];
    bool m_fileSet;

    QString m_termSett;
    int m_termFmt;
    QByteArray m_termData;
    int m_curTabIdx;
    bool m_showLog;
    bool m_showFuses;
    bool m_showSettings;
    bool m_warnBox;
    bool m_tunnelEnabled;
    quint32 m_tunnelSpeed;
    bool m_over_enable;
    bool m_over_turnoff;
    double m_over_val;
};

#endif // MINISHUPITOUI_H
