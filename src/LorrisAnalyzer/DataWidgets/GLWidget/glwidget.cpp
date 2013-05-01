/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/
#include <QFileDialog>

#include "glwidget.h"
#include "renderwidget.h"

REGISTER_DATAWIDGET(WIDGET_OPENGL, GL, NULL)

GLWidget::GLWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_widgetType = WIDGET_OPENGL;

    setTitle(tr("Rotation"));
    setIcon(":/dataWidgetIcons/opengl.png");

    m_widget = new RenderWidget(this);
    layout->addWidget(m_widget, 1);

    resize(300, 300);
}

void GLWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    contextMenu->addAction(tr("Load model..."), this, SLOT(loadModel()));
}

void GLWidget::setRotationX(float ang)
{
    m_widget->setRotationX(ang);
}

void GLWidget::setRotationY(float ang)
{
    m_widget->setRotationY(ang);
}

void GLWidget::setRotationZ(float ang)
{
    m_widget->setRotationZ(ang);
}

void GLWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);
    m_widget->save(file);
}

void GLWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);
    m_widget->load(file);
}

void GLWidget::loadModel(const QString &file)
{
    m_widget->setModelFile(file);
}

void GLWidget::loadModel()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Load model"), QString(), tr("Wavefront models (*.obj)"));
    if(file.isEmpty())
        return;
    loadModel(file);
}

GLWidgetAddBtn::GLWidgetAddBtn(QWidget *parent) : DataWidgetAddBtn(parent)
{
    setText(tr("Rotation"));
    setIconSize(QSize(17, 17));
    setIcon(QIcon(":/dataWidgetIcons/opengl.png"));

    m_widgetType = WIDGET_OPENGL;
}
