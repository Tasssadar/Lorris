/**********************************************
**    This file is part of Lorris
**    http://tasssadar.github.com/Lorris/
**
**    See README and COPYING
***********************************************/
#include <QFileDialog>

#include "rotationwidget.h"

REGISTER_DATAWIDGET(WIDGET_ROTATION, Rotation, NULL)
W_TR(QT_TRANSLATE_NOOP("DataWidget", "Rotation"))

#ifdef HAVE_OPENGL
#include "renderwidget.h"

RotationWidget::RotationWidget(QWidget *parent) :
    DataWidget(parent)
{
    m_widget = new RenderWidget(this);
    layout->addWidget(m_widget, 1);

    resize(300, 300);
}

void RotationWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);

    contextMenu->addAction(tr("Load model..."), this, SLOT(loadModel()));
    contextMenu->addAction(tr("Reset camera"), this, SLOT(resetCamera()));
}

void RotationWidget::setRotationX(float ang)
{
    m_widget->setRotationX(ang);
}

void RotationWidget::setRotationY(float ang)
{
    m_widget->setRotationY(ang);
}

void RotationWidget::setRotationZ(float ang)
{
    m_widget->setRotationZ(ang);
}

void RotationWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);
    m_widget->save(file);
}

void RotationWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);
    m_widget->load(file);
}

void RotationWidget::loadModel(const QString &file)
{
    m_widget->setModelFile(file);
}

void RotationWidget::loadModel()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Load model"), QString(), tr("Wavefront models (*.obj)"));
    if(file.isEmpty())
        return;
    loadModel(file);
}

void RotationWidget::rotateCamera(float deltaX, float deltaY, float deltaZ)
{
    m_widget->rotateCamera(deltaX, deltaY, deltaZ);
}

void RotationWidget::resetCamera()
{
    m_widget->resetCamera();
}

#else //HAVE_OPENGL

RotationWidget::RotationWidget(QWidget *parent) :
    DataWidget(parent)
{
    QLabel *label = new QLabel(tr("This widget requires OpenGL support"), this);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label, 1);

    resize(300, 300);
}

void RotationWidget::setUp(Storage *storage)
{
    DataWidget::setUp(storage);
}

void RotationWidget::saveWidgetInfo(DataFileParser *file)
{
    DataWidget::saveWidgetInfo(file);
}

void RotationWidget::loadWidgetInfo(DataFileParser *file)
{
    DataWidget::loadWidgetInfo(file);
}

void RotationWidget::setRotationX(float ang) { }
void RotationWidget::setRotationY(float ang) { }
void RotationWidget::setRotationZ(float ang) { }
void RotationWidget::loadModel(const QString &file) { }
void RotationWidget::loadModel() { }
void RotationWidget::rotateCamera(float deltaX, float deltaY, float deltaZ) { }
void RotationWidget::resetCamera() { }

#endif // HAVE_OPENGL
