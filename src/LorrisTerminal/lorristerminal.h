#ifndef LORRISTERMINAL_H
#define LORRISTERMINAL_H

#include <QObject>
#include <QLineEdit>

#include "WorkTab.h"

class QVBoxLayout;
class QTextEdit;

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
    QVBoxLayout *layout;
    QLineEdit *hexLine;
    QTextEdit *text;
};

#endif // LORRISTERMINAL_H
