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

#define RENDER_TIMER 50
#define DEFAULT_MODEL ":/models/default.obj"

RenderWidget::RenderWidget(QWidget *parent) :
    QGLWidget(parent)
{
    m_modelRotX = m_modelRotY = m_modelRotZ = 0;
    m_cameraRotX = 45;
    m_cameraRotY = 45;
    m_cameraRotZ = 0;
    m_scale = 2.0;
    m_x = 0;
    m_z = -10;
    m_y = 0;
    m_camera_dist = 10.f;

    m_modelFile = DEFAULT_MODEL;

    setFocusPolicy(Qt::StrongFocus);

    m_timer = new QTimer(this);
    m_timer->start(RENDER_TIMER);
    connect(m_timer, SIGNAL(timeout()), SLOT(repaint()));
}

RenderWidget::~RenderWidget()
{
    delete_vect(m_models);
}

void RenderWidget::initializeGL()
{
    ObjFileLoader::load(m_modelFile, m_models);

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
    glFrustum(left, right, bottom, top, 5, 80);
    glMatrixMode(GL_MODELVIEW);

    updateGL();
}

void RenderWidget::paintGL()
{
    m_timer->start(RENDER_TIMER);

    qglClearColor(Qt::lightGray);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -m_camera_dist);

    glRotatef(m_cameraRotX, 1.0f, 0.0f, 0.0f);
    glRotatef(m_cameraRotY, 0.0f, 1.0f, 0.0f);
    glRotatef(m_cameraRotZ, 0.0f, 0.0f, 1.0f);

    glRotatef(m_modelRotY, 0.0f, 1.0f, 0.0f);
    glRotatef(m_modelRotX, 1.0f, 0.0f, 0.0f);
    glRotatef(m_modelRotZ, 0.0f, 0.0f, 1.0f);

    glScalef(m_scale, m_scale, m_scale);


    for(quint32 i = 0; i < m_models.size(); ++i)
        m_models.at(i)->draw();

    glLoadIdentity();

    float camerax = m_camera_dist * cos((90 + 270.0f) * M_PI / 180) + m_x;
    float cameraz = m_camera_dist * sin((90 - 270.0f) * M_PI / 180) + m_z;

    glTranslatef(0.0f, 0, -m_camera_dist);
    glScalef(m_scale, m_scale, m_scale);

    glRotatef(m_cameraRotX, 1.0f, 0.0f, 0.0f);
    glRotatef(m_cameraRotY, 0.0f, 1.0f, 0.0f);
    glTranslatef(-camerax, -m_y, -cameraz);
    glRotatef(180, 1.0f, 0.0f, 0.0f);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glBegin(GL_LINES);
    for(int i=0;i<=20;i++) {
        if(i == 10)
            continue;

        glVertex3f(i,0,0);
        glVertex3f(i,0,20);
        glVertex3f(0,0,i);
        glVertex3f(20,0,i);
    };

    glLineWidth (10.0);
    glColor3f (1,0,0);
    glVertex3f(-50, 0, 10);
    glVertex3f(50, 0, 10);

    glColor3f (0,1,0);
    glVertex3f(10, -50, 10);
    glVertex3f(10, 50, 10);

    glColor3f (0,0,1);
    glVertex3f(10, 0, -50);
    glVertex3f(10, 0, 50);
    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void RenderWidget::rotateBy(float xAngle, float yAngle, float zAngle)
{
     m_cameraRotY += yAngle;

     m_cameraRotX += xAngle;
     m_cameraRotX = (std::max)(0.f, m_cameraRotX);
     m_cameraRotX = (std::min)(90.f, m_cameraRotX);

     m_cameraRotZ += zAngle;

     updateGL();
}

void RenderWidget::setRotationX(float ang)
{
    m_modelRotX = ang;
}

void RenderWidget::setRotationY(float ang)
{
    m_modelRotY = ang;
}

void RenderWidget::setRotationZ(float ang)
{
    m_modelRotZ = ang;
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
        rotateBy(dy, dx, 0);
    } else if (event->buttons() & Qt::MidButton) {
        rotateBy(dy, 0, dx);
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
    return QGLWidget::keyPressEvent(ev);

    /*
    float ang = ((m_cameraRotY)+180);

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
    */
}

void RenderWidget::repaint()
{
    updateGL();
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
