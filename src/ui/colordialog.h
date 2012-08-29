/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QDialog>

class ColorDialog : public QDialog
{
    Q_OBJECT
public:
    static bool getColors(QList<QColor>& colors, QStringList labels, QWidget *parent = 0);

private slots:
    void btnClicked(int idx);

private:
    ColorDialog(QList<QColor> colors, QStringList labels, QWidget *parent = 0);
    void setButtonColor(int idx, const QColor& clr);

    QList<QColor> m_colors;
    std::vector<QPushButton*> m_buttons;
};

#endif // COLORDIALOG_H
