/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QInputDialog>
#include <QDialogButtonBox>

#include "buttonwidget.h"
#include "../../ui/shortcutinputbox.h"

ButtonWidget::ButtonWidget(QWidget *parent) : DataWidget(parent)
{
    setTitle(tr("Button"));
    setIcon(":/dataWidgetIcons/button.png");

    m_widgetType = WIDGET_BUTTON;

    m_button = new QPushButton(tr("Button"), this);
    m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(m_button, 4);

    adjustSize();
}

void ButtonWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    QAction *btnText = contextMenu->addAction(tr("Set button text..."));
    QAction *btnShortcut = contextMenu->addAction(tr("Set shortcut..."));

    connect(btnText,     SIGNAL(triggered()), SLOT(setButtonName()));
    connect(btnShortcut, SIGNAL(triggered()), SLOT(setShortcut()));
    connect(m_button,    SIGNAL(clicked()),   SLOT(buttonClicked()));
}

void ButtonWidget::buttonClicked()
{
    emit scriptEvent(getTitle() + "_clicked");
}

void ButtonWidget::setButtonName()
{
    QString name = QInputDialog::getText(this, tr("Button text"), tr("Enter new button text"));
    if(name.isEmpty())
        return;
    setButtonName(name);
}

void ButtonWidget::setButtonName(const QString &name)
{
    m_button->setText(name);
}

void ButtonWidget::setShortcut()
{
    QDialog d(this);
    d.setWindowFlags(d.windowFlags() & ~(Qt::WindowMaximizeButtonHint | Qt::WindowContextHelpButtonHint));
    d.setWindowTitle(tr("Set button shortcut"));

    QVBoxLayout *l = new QVBoxLayout(&d);

    ShortcutInputBox *box = new ShortcutInputBox(m_button->shortcut(), &d);
    QDialogButtonBox *btn = new QDialogButtonBox((QDialogButtonBox::Ok |QDialogButtonBox::Cancel), Qt::Horizontal, &d);

    l->addWidget(box);
    l->addWidget(btn);

    connect(btn, SIGNAL(accepted()), &d, SLOT(accept()));
    connect(btn, SIGNAL(rejected()), &d, SLOT(reject()));

    if(d.exec() == QDialog::Accepted)
        m_button->setShortcut(box->getKeySequence());
}

void ButtonWidget::setShortcut(const QString &shortcut)
{
    m_button->setShortcut(QKeySequence(shortcut));
}

void ButtonWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("buttonWText");
    file->writeString(m_button->text());

    file->writeBlockIdentifier("buttonWShortcut");
    file->writeString(m_button->shortcut().toString());
}

void ButtonWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("buttonWText", BLOCK_WIDGET))
        m_button->setText(file->readString());

    if(file->seekToNextBlock("buttonWShortcut", BLOCK_WIDGET))
        m_button->setShortcut(QKeySequence(file->readString()));
}

ButtonWidgetAddBtn::ButtonWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Button"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/button.png"));

    m_widgetType = WIDGET_BUTTON;
}
