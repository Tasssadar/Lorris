/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPixmap>
#include <QMouseEvent>
#include <QTimer>
#include <QtOpenGL>

#include "glutils.h"
#include "objfileloader.h"
#include "renderwidget.h"
#include "../../../misc/utils.h"
#include "../../../misc/datafileparser.h"

#define RENDER_TIMER 33 // peasantry
#define DEFAULT_MODEL ":/models/default.obj"

RenderWidget::RenderWidget(QWidget *parent) :
    QOpenGLWidget(parent)
{
    m_modelRotX = m_modelRotY = m_modelRotZ = 0;
    m_renderRequested = false;
    m_modelFile = DEFAULT_MODEL;

    resetCamera();

    setFocusPolicy(Qt::StrongFocus);

    m_timer = new QTimer(this);
    m_timer->setInterval(RENDER_TIMER);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(requestRender()));

    ObjFileLoader::load(m_modelFile, m_models);
}

RenderWidget::~RenderWidget()
{
    delete_vect(m_models);
}

void RenderWidget::resetCamera()
{
    m_cameraRotX = 45;
    m_cameraRotY = 45;
    m_cameraRotZ = 0;
    m_scale = 2.0;
    m_x = 0;
    m_z = -10;
    m_y = 0;
    m_camera_dist = 10.f;

    m_renderRequested = true;
}

void RenderWidget::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_BLEND);
    glEnable (GL_LINE_SMOOTH);
    glCullFace(GL_NONE);

    {
        glEnable(GL_LIGHT0);
        float specular[] = {0.5, 0.5, 0.5, 1.0};
        float diffuse[] = {0.8, 0.8, 0.8, 1.0};
        float position[] = { 1.0, 1.0, 1.0, 0.0 };
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, position);
    }

    {
        glEnable(GL_LIGHT1);
        float specular[] = {0.2, 0.2, 0.2, 1.0};
        float diffuse[] = {0.5, 0.5, 0.6, 1.0};
        float position[] = { 0.0, 1.0, 1.0, 0.0 };
        glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT1, GL_POSITION, position);
    }

    {
        glEnable(GL_LIGHT2);
        float specular[] = {0.5, 0.5, 0.5, 1.0};
        float diffuse[] = {0.8, 0.8, 0.8, 1.0};
        float position[] = { -1.0, 1.0, -1.0, 0.0 };
        glLightfv(GL_LIGHT2, GL_SPECULAR, specular);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT2, GL_POSITION, position);
    }
}

void RenderWidget::mesaGluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    GLdouble xmin, xmax, ymin, ymax;

    ymax = zNear * tan(fovy * M_PI / 360.0);
    ymin = -ymax;

    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void RenderWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /*GLdouble top, bottom, left, right;
    top = 4 * tan((M_PI/180)*90/2);
    bottom = -top;
    right = (float(width)/height)*top;
    left = -right;
    glFrustum(left, right, bottom, top, m_near, m_far);
    //glOrtho(left, right, bottom, top, 4, 80);
    */

    mesaGluPerspective(90, (double(width)/height), 1, 100);

    glMatrixMode(GL_MODELVIEW);

    update();
}

void RenderWidget::paintGL()
{
    glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    float camerax = m_camera_dist * cos((90 + 270.0f) * M_PI / 180) + m_x;
    float cameraz = m_camera_dist * sin((90 - 270.0f) * M_PI / 180) + m_z;

    glTranslatef(0, 0, -m_camera_dist);
    glScalef(m_scale, m_scale, m_scale);

    glRotatef(m_cameraRotX, 1.0f, 0.0f, 0.0f);
    glRotatef(m_cameraRotY, 0.0f, 1.0f, 0.0f);
    glRotatef(m_cameraRotZ, 0.0f, 0.0f, 1.0f);

    glTranslatef(-camerax, -m_y, -cameraz);
    glTranslatef(10, 0, -10);

    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glBegin(GL_LINES);

        glColor3f (1,1,1);
        for(int i = -10; i <= 10; ++i) {
            if(i == 0)
                continue;

            glVertex3f(i, 0,-10);
            glVertex3f(i, 0, 10);
            glVertex3f(-10, 0, i);
            glVertex3f(10, 0, i);
        };

        glLineWidth (10.0);
        glColor3f (1,0,0);
        glVertex3f(-50, 0, 0);
        glVertex3f(50, 0, 0);

        glColor3f (0,1,0);
        glVertex3f(0, -50, 0);
        glVertex3f(0, 50, 0);

        glColor3f (0,0,1);
        glVertex3f(0, 0, -50);
        glVertex3f(0, 0, 50);
        glEnd();


        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }

    glRotatef(180, 1.0f, 0.0f, 0.0f);
    glRotatef(180, 0.0f, 0.0f, 1.0f);

    glRotatef(m_modelRotY, 0.0f, 1.0f, 0.0f);
    glRotatef(m_modelRotX, 1.0f, 0.0f, 0.0f);
    glRotatef(m_modelRotZ, 0.0f, 0.0f, 1.0f);

    for(quint32 i = 0; i < m_models.size(); ++i)
        m_models.at(i)->draw();

    m_lastRender.restart();
}

void RenderWidget::rotateCamera(float xAngle, float yAngle, float zAngle)
{
     m_cameraRotY += yAngle;

     m_cameraRotX += xAngle;
     m_cameraRotX = (std::max)(-90.f, m_cameraRotX);
     m_cameraRotX = (std::min)(90.f, m_cameraRotX);

     m_cameraRotZ += zAngle;

     requestRender();
}

void RenderWidget::setRotationX(float ang)
{
    m_modelRotX = ang;
    requestRender();
}

void RenderWidget::setRotationY(float ang)
{
    m_modelRotY = ang;
    requestRender();
}

void RenderWidget::setRotationZ(float ang)
{
    m_modelRotZ = ang;
    requestRender();
}

void RenderWidget::mousePressEvent(QMouseEvent *ev)
{
    if(!(ev->buttons() & (Qt::LeftButton | Qt::MidButton)))
        return QOpenGLWidget::mousePressEvent(ev);
    lastPos = ev->pos();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & (Qt::LeftButton | Qt::MidButton)))
        return QOpenGLWidget::mouseMoveEvent(event);

    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton)
        rotateCamera(dy, dx, 0);
    else if (event->buttons() & Qt::MidButton)
        rotateCamera(dy, 0, dx);

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
    update();
}

void RenderWidget::keyPressEvent(QKeyEvent *ev)
{
    float ang = ((m_cameraRotY)+180);

    while(ang >= 360) ang -= 360;
    while(ang < 0) ang = 360 - ang;

    ang = 360 - ang;

    while(ang >= 360) ang -= 360;
    while(ang < 0) ang = 360 - ang;

    switch(ev->key())
    {
        case Qt::Key_W:
            //ang += 0.f;
            break;
        case Qt::Key_S:
            ang += 180.f;
            break;
        case Qt::Key_A:
            ang += 90.f;
            break;
        case Qt::Key_D:
            ang -= 90.f;
            break;
        case Qt::Key_Shift:
        case Qt::Key_Q:
            m_y += 0.25;
            return update();
        case Qt::Key_Control:
        case Qt::Key_E:
            m_y -= 0.25;
            return update();
        default:
            return QOpenGLWidget::keyPressEvent(ev);
    }

    ang *= (M_PI / 180);

    m_z += cos(ang)*1;
    m_x += sin(ang)*1;

    requestRender();
}

void RenderWidget::requestRender()
{
    if(m_lastRender.elapsed() > RENDER_TIMER)
        update();
    else
        m_timer->start();
}

void RenderWidget::setModelFile(const QString &path)
{
    delete_vect(m_models);
    if(!ObjFileLoader::load(path, m_models))
    {
        delete_vect(m_models);
        ObjFileLoader::load(m_modelFile, m_models);
        return;
    }
    m_modelFile = path;
}

void RenderWidget::save(DataFileParser *file)
{
    file->writeBlockIdentifier("glwidgetCamera");
    file->writeVal(m_scale);
    file->writeVal(m_cameraRotX);
    file->writeVal(m_cameraRotY);
    file->writeVal(m_cameraRotZ);

    file->writeBlockIdentifier("glwidgetModel");
    file->writeString(m_modelFile);
    file->writeVal(m_modelRotX);
    file->writeVal(m_modelRotY);
    file->writeVal(m_modelRotZ);
}

void RenderWidget::load(DataFileParser *file)
{
    if(file->seekToNextBlock("glwidgetCamera", BLOCK_WIDGET))
    {
        m_scale = file->readVal<double>();
        m_cameraRotX = file->readVal<float>();
        m_cameraRotY = file->readVal<float>();
        m_cameraRotZ = file->readVal<float>();
    }

    if(file->seekToNextBlock("glwidgetModel", BLOCK_WIDGET))
    {
        setModelFile(file->readString());
        m_modelRotX = file->readVal<float>();
        m_modelRotY = file->readVal<float>();
        m_modelRotZ = file->readVal<float>();
    }
}
