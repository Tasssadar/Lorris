/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include "datawidget.h"

class QSlider;
class ColorDisplay;

class ColorWidget : public DataWidget
{
    Q_OBJECT
public:
    /// \brief To be used with \c setColorType()
    enum colorType
    {
        COLOR_RGB_8,
        COLOR_RGB_10,
        COLOR_RGB_10_UINT,
        COLOR_GRAY_8,
        COLOR_GRAY_10,
        COLOR_RGBA_8,
        COLOR_ARGB_8,

        COLOR_MAX
    };
    static void addEnum();

    ColorWidget(QWidget *parent = 0);
    ~ColorWidget();

    void setUp(Storage *);
    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);

public slots:
    /// Format of `color` depends on the color type
    /// \li <tt>COLOR_RGB_8:</tt> ((R << 16) | (G << 8) | B), channel values 0 to 255
    /// \li <tt>COLOR_RGBA_8:</tt> ((R << 24) | (G << 16) | (B << 8) | A), channel values 0 to 255, alpha is ignored
    /// \li <tt>COLOR_ARGB_8:</tt> ((A << 24) | (R << 16) | (G << 8) | B), channel values 0 to 255, alpha is ignored
    /// \li <tt>COLOR_RGB_10 and COLOR_RGB_10_UINT:</tt> ((R << 20) | (G << 10) | B), channel values 0 to 1023
    /// \li <tt>COLOR_GRAY_8:</tt> value from 0 to 255
    /// \li <tt>COLOR_GRAY_10:</tt> value from 0 to 1023
    /// \brief See detailed description
    void setValue(quint32 color);

    void setValue(int r, int g, int b);
    void setValue(QString hex);
    void setValueAr(QList<int> val);
    void showValues(bool show);
    void setColorType(int type);

    bool is10bit() const
    {
        return m_color_type == COLOR_RGB_10 ||
               m_color_type == COLOR_RGB_10_UINT ||
               m_color_type == COLOR_GRAY_10;
    }

protected:
     void processData(analyzer_data *data);

private slots:
     void brightTriggered();
     void colorTriggered();
     void brightChanged(int value);
     void colorChangedR(int value);
     void colorChangedG(int value);
     void colorChangedB(int value);

private:
     void updateColor();

     ColorDisplay *m_widget;
     QHBoxLayout *m_brightness_layout;
     QHBoxLayout *m_color_layout[3];
     qint16 m_brightness;
     qint16 m_color_cor[3];
     int m_color_type;

     QAction *brightAct;
     QAction *colorAct;
     QAction *textAct;
     QAction *colorType[COLOR_MAX];
};

class ColorDisplay : public QWidget
{
    Q_OBJECT
public:
    ColorDisplay(QWidget *parent);

    quint16 *color() { return m_color; }
    void setAll(quint16 color)
    {
        m_color[0] = m_color[1] = m_color[2] = color;
    }

    void setDrawNums(bool draw)
    {
        if(draw) m_drawNums |= 0x01;
        else     m_drawNums &= ~(0x01);
    }

    void setGrey(bool gray)
    {
        if(gray) m_drawNums |= 0x02;
        else     m_drawNums &= ~(0x02);
    }

protected:
    void paintEvent(QPaintEvent * ev);

private:
    quint16 m_color[3];
    quint8 m_drawNums;
};

#endif // COLORWIDGET_H
