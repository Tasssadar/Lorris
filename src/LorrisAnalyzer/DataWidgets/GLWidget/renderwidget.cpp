/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/

#include <QPixmap>
#include <QMouseEvent>
#include "renderwidget.h"

RenderWidget::RenderWidget(QWidget *parent) :
    QGLWidget(parent)
{
    xRot = 0;
    yRot = 0;
    m_scale = 2.0;
    zRot = 0;
    m_x = 0;
    m_z = -10;
    m_y = 0.5;
    setFocusPolicy(Qt::StrongFocus);
}

void RenderWidget::initializeGL()
{
    static const int coords[6][4][3] = {
        { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
        { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
        { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
        { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
        { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
        { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };

    QPixmap map(256, 256);
    map.fill(Qt::white);

    for (int j=0; j < 6; ++j) {
             textures[j] = bindTexture
                  (QPixmap(":/dataWidgetIcons/canvas.png"), GL_TEXTURE_2D);
         }

    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 4; ++j) {
            texCoords.append
                    (QVector2D(j == 0 || j == 3, j == 0 || j == 1));
            vertices.append
                    (QVector3D(0.2 * coords[i][j][0], 0.2 * coords[i][j][1],
                               0.2 * coords[i][j][2]));
        }
    }

    glClearColor(0,0,0,0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
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
    qglClearColor(Qt::white);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float camerax = 10 * cos((90 + 270.0f) * M_PI / 180) + m_x;
    float cameraz = 10 * sin((90 - 270.0f) * M_PI / 180) + m_z;

    // draw cube
    glLoadIdentity();
    glTranslatef(0.0f, 0, -10.0f);
    glRotatef(xRot / 16.0f, 1.0f, 0.0f, 0.0f);

    glScalef(m_scale, m_scale, m_scale);

    glVertexPointer(3, GL_FLOAT, 0, vertices.constData());
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords.constData());
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    for (int i = 0; i < 6; ++i) {
             glBindTexture(GL_TEXTURE_2D, textures[i]);
             glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
    }


    // draw grid

    glRotatef(yRot / 16.0f, 0.0f, 1.0f, 0.0f);

    glTranslatef(-camerax, -m_y, -cameraz);
    glRotatef(180, 1.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    glVertex3f( 0,-0.001, 0);
    glVertex3f( 0,-0.001,10);
    glVertex3f(10,-0.001,10);
    glVertex3f(10,-0.001, 0);
    glEnd();

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
