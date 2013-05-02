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
class DataFileParser;

class RenderWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit RenderWidget(QWidget *parent = 0);
    ~RenderWidget();
    
public slots:
    void rotateBy(float xAngle, float yAngle, float zAngle);
    void setRotationX(float ang);
    void setRotationY(float ang);
    void setRotationZ(float ang);

    void setModelFile(const QString& path);

    void save(DataFileParser *file);
    void load(DataFileParser *file);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void keyPressEvent(QKeyEvent *ev);

private slots:
    void repaint();

private:
    void mesaGluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);

    float m_modelRotX;
    float m_modelRotY;
    float m_modelRotZ;

    float m_cameraRotX;
    float m_cameraRotY;
    float m_cameraRotZ;

    std::vector<GLModel*> m_models;

    QPoint lastPos;
    double m_scale;
    double m_x;
    double m_z;
    double m_y;
    float m_camera_dist;
    QTimer *m_timer;
    QString m_modelFile;
    bool m_renderRequested;
};

#endif // RENDERWIDGET_H
