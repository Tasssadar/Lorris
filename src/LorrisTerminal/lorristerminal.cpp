#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include "lorristerminal.h"


LorrisTerminal::LorrisTerminal()
{
    mainWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(mainWidget);
    QTextEdit *text = new QTextEdit(mainWidget);
    text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    text->setShown(true);
    text->setEnabled(false);
    QPushButton *button = new QPushButton("Hi", mainWidget);

    layout->addWidget(button);
    layout->addWidget(text);
    layout->setAlignment(button, Qt::AlignTop);
    mainWidget->setLayout(layout);
}

LorrisTerminal::~LorrisTerminal()
{
    if(mainWidget)
        delete mainWidget;
}

QWidget *LorrisTerminal::GetTab(QWidget *parent)
{
    mainWidget->setParent(parent);
    return mainWidget;
}
