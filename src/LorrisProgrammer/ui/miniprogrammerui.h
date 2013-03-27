/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef MINIPROGRAMMERUI_H
#define MINIPROGRAMMERUI_H

#include "programmerui.h"

#include "ui_miniprogrammerui.h"

class MiniProgrammerUI : public ProgrammerUI
{
    Q_OBJECT
public:
    explicit MiniProgrammerUI(QObject *parent = 0);
    ~MiniProgrammerUI();

    void setupUi(LorrisProgrammer *widget);
    void setChipId(const QString &text);
    void setFileAndTime(const QString &file, const QDateTime &time);

    void saveData(DataFileParser *file);
    void loadData(DataFileParser *file);

    void setHexData(quint32 memid, const QByteArray& data);
    QByteArray getHexData(quint32 memid) const;

    void writeSelectedMem();
    void warnSecondFlash();

    void enableButtons(bool enable) override;

public slots:
    void setVertical(bool vertical);

protected:
    QToolButton *startStopBtn() const { return ui->startStopBtn; }
    QBoxLayout *vddLayout() const { return ui->vddLayout; }
    QLabel *engineLabel() const { return ui->engineLabel; }
    QLabel *vccLabel() const { return ui->vccLabel; }

    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void enableWrite(bool enable);

private:
    Ui::MiniProgrammerUI *ui;

    QByteArray m_hexData[MEM_FUSES];
    QByteArray m_svfData;
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
    bool m_isVertical;
};

#endif // MINIPROGRAMMERUI_H
