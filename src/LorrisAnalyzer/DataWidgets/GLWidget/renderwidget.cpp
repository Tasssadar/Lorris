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
    zRot = 0;
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
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

#if !defined(QT_OPENGL_ES_2)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifndef QT_OPENGL_ES
    glOrtho(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
#else
    glOrthof(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
#endif
    glMatrixMode(GL_MODELVIEW);
#endif
    updateGL();
}

void RenderWidget::paintGL()
{
    qglClearColor(Qt::lightGray);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -10.0f);
    glRotatef(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(zRot / 16.0f, 0.0f, 0.0f, 1.0f);

    glVertexPointer(3, GL_FLOAT, 0, vertices.constData());
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords.constData());
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    for (int i = 0; i < 6; ++i) {
             glBindTexture(GL_TEXTURE_2D, textures[i]);
             glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
    }
}

void RenderWidget::rotateBy(int xAngle, int yAngle, int zAngle)
{
     xRot += xAngle;
     yRot += yAngle;
     zRot += zAngle;
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

    int dx = -(event->x() - lastPos.x());
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        rotateBy(8 * dy, 8 * dx, 0);
    } else if (event->buttons() & Qt::RightButton) {
        rotateBy(8 * dy, 0, 8 * dx);
    }
    lastPos = event->pos();
}
