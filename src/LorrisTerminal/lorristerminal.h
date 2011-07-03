#ifndef LORRISTERMINAL_H
#define LORRISTERMINAL_H

#include <QObject>
#include <QLineEdit>
#include <QPlainTextEdit>

#include "WorkTab.h"

class LorrisTerminal : public WorkTab
{
    Q_OBJECT
public:
    explicit LorrisTerminal();
    virtual ~LorrisTerminal();

    QWidget *GetTab(QWidget *parent);

private slots:
    void browseForHex();
    void readData(QByteArray data);

private:


    QWidget *mainWidget;
    QLineEdit *hexLine;
    QPlainTextEdit *text;
};

#endif // LORRISTERMINAL_H
