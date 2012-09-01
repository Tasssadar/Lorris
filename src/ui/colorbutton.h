/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>

class ColorButton : public QPushButton
{
    Q_OBJECT
Q_SIGNALS:
    void colorChanged(const QColor& color);

public:
    ColorButton(QColor color, QWidget *parent = 0);
    ColorButton(QWidget *parent = 0);

    void setColor(const QColor& color);
    const QColor& getColor() const { return m_color; }

public slots:
    void choose();

private:
    QColor m_color;
};

#endif // COLORBUTTON_H
