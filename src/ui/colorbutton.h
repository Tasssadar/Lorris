/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>
#include <QMetaType>

class ColorButton : public QPushButton
{
    Q_OBJECT
Q_SIGNALS:
    void colorChanged(const QColor& color);

public:
    ColorButton(QColor color, QWidget *parent = 0);
    ColorButton(QWidget *parent = 0);
    ColorButton(const ColorButton& btn);

public slots:
    void setColor(const QColor& color);
    void setColor(const QString& color) { setColor(QColor(color)); }
    const QColor& getColor() const { return m_color; }
    QString getColorName() const { return m_color.name(); }
    int r() const { return m_color.red(); }
    int g() const { return m_color.green(); }
    int b() const { return m_color.blue(); }

    void choose();

private:
    QColor m_color;
};

Q_DECLARE_METATYPE(ColorButton)

#endif // COLORBUTTON_H
