/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QGLWidget>
#include <QVector>
#include <QVector3D>
#include <QVector2D>

class GLModel;
class RenderWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit RenderWidget(QWidget *parent = 0);
    ~RenderWidget();
    
    void rotateBy(int xAngle, int yAngle, int zAngle);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void keyPressEvent(QKeyEvent *ev);

private:
    float xRot;
    float yRot;
    float zRot;

    std::vector<GLModel*> m_models;

    QPoint lastPos;
    double m_scale;
    double m_x;
    double m_z;
    double m_y;
    float m_camera_dist;
};

#endif // RENDERWIDGET_H
