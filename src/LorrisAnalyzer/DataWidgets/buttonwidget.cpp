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
#include "../../ui/colordialog.h"

REGISTER_DATAWIDGET(WIDGET_BUTTON, Button, NULL)

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
    QAction *btnColor = contextMenu->addAction(tr("Set colors..."));

    const QPalette &p = m_button->palette();
    m_colors.push_back(p.color(QPalette::Button));
    m_colors.push_back(p.color(QPalette::ButtonText));

    connect(btnText,     SIGNAL(triggered()), SLOT(setButtonName()));
    connect(btnShortcut, SIGNAL(triggered()), SLOT(setShortcut()));
    connect(btnColor,    SIGNAL(triggered()),   SLOT(setColors()));
    connect(m_button,    SIGNAL(clicked()),   SLOT(buttonClicked()));
    connect(m_button,    SIGNAL(clicked()),   SIGNAL(clicked()));
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

void ButtonWidget::setColors()
{
    static const QStringList labels = (QStringList() << tr("Button") << tr("Button text"));
    if(!ColorDialog::getColors(m_colors, labels, this))
        return;

    QPalette p = m_button->palette();

#ifdef Q_OS_WIN
    m_button->setStyleSheet("background-color: " + m_colors[0].name());
#else
    p.setColor(QPalette::Button, m_colors[0]);
#endif

    p.setColor(QPalette::ButtonText, m_colors[1]);
    m_button->setPalette(p);
}

void ButtonWidget::setColor(const QString& color)
{
    QColor c(color);
    if(!c.isValid())
        return;

    m_colors[0] = c;

#ifdef Q_OS_WIN
    m_button->setStyleSheet("background-color: " + c.name());
#else
    QPalette p = m_button->palette();
    p.setColor(QPalette::Button, c);
    m_button->setPalette(p);
#endif
}

void ButtonWidget::setTextColor(const QString& color)
{
    QColor c(color);
    if(!c.isValid())
        return;

    m_colors[1] = c;

    QPalette p = m_button->palette();
    p.setColor(QPalette::ButtonText, c);
    m_button->setPalette(p);
}

void ButtonWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);

    file->writeBlockIdentifier("buttonWText");
    file->writeString(m_button->text());

    file->writeBlockIdentifier("buttonWShortcut");
    file->writeString(m_button->shortcut().toString());

    file->writeBlockIdentifier("buttonWColors");
    file->writeString(m_colors[0].name());
    file->writeString(m_colors[1].name());
}

void ButtonWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);

    if(file->seekToNextBlock("buttonWText", BLOCK_WIDGET))
        m_button->setText(file->readString());

    if(file->seekToNextBlock("buttonWShortcut", BLOCK_WIDGET))
        m_button->setShortcut(QKeySequence(file->readString()));

    if(file->seekToNextBlock("buttonWColors", BLOCK_WIDGET))
    {
        setColor(file->readString());
        setTextColor(file->readString());
    }
}

ButtonWidgetAddBtn::ButtonWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Button"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/button.png"));

    m_widgetType = WIDGET_BUTTON;
}
