#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QScrollBar>

#include "lorristerminal.h"

LorrisTerminal::LorrisTerminal() : WorkTab()
{
    mainWidget = new QWidget();
    layout = new QVBoxLayout(mainWidget);
    QHBoxLayout *layout_buttons = new QHBoxLayout(mainWidget);
    text = new QTextEdit(mainWidget);
    text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    text->setShown(true);
    text->setReadOnly(true);

    QColor color_black(0, 0, 0);\
    QColor color_white(255, 255, 255);\
    QPalette palette;
    palette.setColor(QPalette::Base, color_black);
    palette.setColor(QPalette::Text, color_white);
    text->setPalette(palette);

    hexLine = new QLineEdit(mainWidget);
    QPushButton *browse = new QPushButton("Browse...", mainWidget);
    connect(browse, SIGNAL(clicked()), this, SLOT(browseForHex()));

    layout_buttons->addWidget(hexLine);
    layout_buttons->addWidget(browse);
    layout->addLayout(layout_buttons);
    layout->addWidget(text);

    mainWidget->setLayout(layout);
}

LorrisTerminal::~LorrisTerminal()
{
    text = NULL;
    if(mainWidget)
    {
        WorkTab::DeleteAllMembers(layout);
        delete layout;
        delete mainWidget;
    }
}

QWidget *LorrisTerminal::GetTab(QWidget *parent)
{
    mainWidget->setParent(parent);
    return mainWidget;
}

void LorrisTerminal::browseForHex()
{
    hexLine->setText(QFileDialog::getOpenFileName(mainWidget, tr("Open File"), "", tr("Intel hex file (*.hex)")));\
}

void LorrisTerminal::readData(QByteArray data)
{
    if(!text)
        return;

    text->moveCursor(QTextCursor::End);
    text->insertPlainText(QString(data));
    QScrollBar *sb = text->verticalScrollBar();
    sb->setValue(sb->maximum());
}
