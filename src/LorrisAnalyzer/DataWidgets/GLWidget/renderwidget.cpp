/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPixmap>
#include <QMouseEvent>

#include "glutils.h"
#include "objfileloader.h"
#include "renderwidget.h"
#include "../../../misc/utils.h"

RenderWidget::RenderWidget(QWidget *parent) :
    QGLWidget(parent)
{
    xRot = 0;
    yRot = 0;
    m_scale = 2.0;
    zRot = 0;
    m_x = 0;
    m_z = -10;
    m_y = 0;
    m_camera_dist = 10.f;

    setFocusPolicy(Qt::StrongFocus);
}

RenderWidget::~RenderWidget()
{
    delete_vect(m_models);
}

void RenderWidget::initializeGL()
{
    ObjFileLoader::load("/home/tassadar/kostka_test/opice6.obj", m_models);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_BLEND);
    glEnable (GL_LINE_SMOOTH);
    glCullFace(GL_NONE);
    glEnable (GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    {
        float specular[] = {1.0, 1.0, 1.0, 1.0};
        float diffuse[] = {1, 1, 1, 1.0};
        float ambient[] = {0, 0, 0, 1};
        float position[] = { 1, 1, 0, 0.0f };
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glEnable(GL_LIGHT0);
    }

    {
        float specular[] = {1.0, 1.0, 1.0, 1.0};
        float diffuse[] = {1, 1, 1, 1.0};
        float ambient[] = {0, 0, 0, 1};
        float position[] = { -1, -1, 0, 0.0f };
        glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT1, GL_POSITION, position);
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
        glEnable(GL_LIGHT1);
    }
}

void RenderWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    GLdouble top, bottom, left, right;
    top = 4 * tan((M_PI/180)*80/2);
    bottom = -top;
    right = (float(width)/height)*top;
    left = -right;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(left, right, bottom, top, 4, 80);
    glMatrixMode(GL_MODELVIEW);

    updateGL();
}

void RenderWidget::paintGL()
{
    qglClearColor(Qt::lightGray);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);

    // draw cube
    glLoadIdentity();
    glTranslatef(0.0f, 0.f, -m_camera_dist);
    glRotatef(xRot / 16.0f, 1.0f, 0.0f, 0.0f);

    glScalef(m_scale, m_scale, m_scale);


    for(quint32 i = 0; i < m_models.size(); ++i)
        m_models.at(i)->draw();

    // draw grid
    float camerax = m_camera_dist * cos((90 + 270.0f) * M_PI / 180) + m_x;
    float cameraz = m_camera_dist * sin((90 - 270.0f) * M_PI / 180) + m_z;

    glLoadIdentity();
    glTranslatef(0.0f, 0, -10.0f);
    glRotatef(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    glScalef(m_scale, m_scale, m_scale);

    glRotatef(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(-camerax, -m_y, -cameraz);
    glRotatef(180, 1.0f, 0.0f, 0.0f);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    /*glBegin(GL_QUADS);
    glVertex3f( 0,-0.001, 0);
    glVertex3f( 0,-0.001,10);
    glVertex3f(10,-0.001,10);
    glVertex3f(10,-0.001, 0);
    glEnd();*/

    glBegin(GL_LINES);
    for(int i=0;i<=10;i++) {
        if (i==0) { glColor3f(.6,.3,.3); } else { glColor3f(.25,.25,.25); };
        glVertex3f(i,0,0);
        glVertex3f(i,0,10);
        if (i==0) { glColor3f(.3,.3,.6); } else { glColor3f(.25,.25,.25); };
        glVertex3f(0,0,i);
        glVertex3f(10,0,i);
    };
    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void RenderWidget::rotateBy(int xAngle, int yAngle, int zAngle)
{
     yRot += yAngle;

     xRot += xAngle;
     xRot = std::max(0.f, xRot);
     xRot = std::min(90.f*16, xRot);

     zRot = 0;
     updateGL();
}

void RenderWidget::mousePressEvent(QMouseEvent *ev)
{
    if(!(ev->buttons() & (Qt::LeftButton | Qt::RightButton)))
        return QGLWidget::mousePressEvent(ev);
    lastPos = ev->pos();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & (Qt::LeftButton | Qt::RightButton)))
        return QGLWidget::mouseMoveEvent(event);

    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        rotateBy(8 * dy, 8 * dx, 0);
    } else if (event->buttons() & Qt::RightButton) {
        rotateBy(8 * dy, 0, 8 * dx);
    }
    lastPos = event->pos();
}

void RenderWidget::wheelEvent(QWheelEvent *ev)
{
    if(ev->delta() == 0)
        return;

    int delta = (ev->delta());

    m_scale += 0.05*(delta/120);
    if(m_scale <= 0)
        m_scale = 0.05;
    updateGL();
}

void RenderWidget::keyPressEvent(QKeyEvent *ev)
{
    float ang = ((yRot/16)+180);

    while(ang >= 360) ang -= 360;
    while(ang < 0) ang = 360 - ang;

    ang = 360 - ang;

    while(ang >= 360) ang -= 360;
    while(ang < 0) ang = 360 - ang;

    if(ev->key() == Qt::Key_W)
        ang += 0.0f;
    else if(ev->key() == Qt::Key_S)
        ang += 180.0f;
    else if(ev->key() == Qt::Key_A)
        ang += 90.f;
    else if(ev->key() == Qt::Key_D)
        ang -= 90.f;
    else if(ev->key() == Qt::Key_Shift)
    {
        m_y += 0.25;
        return updateGL();
    }
    else if(ev->key() == Qt::Key_Control)
    {
        m_y -= 0.25;
        return updateGL();
    }

    ang *= (M_PI / 180);

    m_z += cos(ang);
    m_x += sin(ang);

    updateGL();
}
