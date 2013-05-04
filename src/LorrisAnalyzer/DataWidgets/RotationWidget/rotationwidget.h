/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef ROTATIONWIDGET_H
#define ROTATIONWIDGET_H

#include "../datawidget.h"

class RenderWidget;

class RotationWidget : public DataWidget
{
    Q_OBJECT
public:
    explicit RotationWidget(QWidget *parent = 0);

    void setUp(Storage *storage);

    void saveWidgetInfo(DataFileParser *file);
    void loadWidgetInfo(DataFileParser *file);
    
public slots:
    void setRotationX(float ang);
    void setRotationY(float ang);
    void setRotationZ(float ang);
    void loadModel(const QString& file);
    void loadModel();
    void resetCamera();
    void rotateCamera(float deltaX, float deltaY, float deltaZ);

private:
    RenderWidget *m_widget;
};

class RotationWidgetAddBtn : public DataWidgetAddBtn
{
    Q_OBJECT
public:
    RotationWidgetAddBtn(QWidget *parent = 0);
};

#endif // ROTATIONWIDGET_H
