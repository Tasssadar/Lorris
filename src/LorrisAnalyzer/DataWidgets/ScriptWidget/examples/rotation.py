# Rotation widget example - how to set object's rotation

# create widgets
def createWidgets():
    x = lorris.getWidth() + 20
    y = 0

    lorris.newWidget(WIDGET_ROTATION, "rot_test", 400, 400, x, y)
    x += rot_test.width + 20

    lorris.newWidget(WIDGET_SLIDER, "rot_x", 300, 120, x, y)
    y += rot_x.height+10
    lorris.newWidget(WIDGET_SLIDER, "rot_y", 300, 120, x, y)
    y += rot_x.height+10
    lorris.newWidget(WIDGET_SLIDER, "rot_z", 300, 120, x, y)

    rot_x.setRange(-180, 180)
    rot_y.setRange(-180, 180)
    rot_z.setRange(-180, 180)
createWidgets()

# Set rotation of the object by slider values
def rot_x_valueChanged():
    rot_test.setRotationX(rot_x.getValue())

def rot_y_valueChanged():
    rot_test.setRotationY(rot_y.getValue());

def rot_z_valueChanged():
    rot_test.setRotationZ(rot_z.getValue())
