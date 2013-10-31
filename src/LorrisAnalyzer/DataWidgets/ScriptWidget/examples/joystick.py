# Joystick example

joySelect = None
joystick = None

# Called on axes change. "axes" is Array of ints with ids of axes which were changed
def axesChanged(axes):
    terminal.appendText("Axes changed: " + str(axes) + "\n");
    res = "";
    for i in range(0, joystick.getNumAxes()):
        res += str(i) + ": " + str(joystick.getAxisVal(i)) + ", "; # SEE THIS: Get actual axis value. From -32768 to 32767
    terminal.appendText(res + "\n\n");

# Called when button status is changed, for each button separatedly
# Pressed: state == 1
def buttonChanged(id, state):
    terminal.appendText("Button " + str(id) + ", state " + str(state) + "\n");

def dumpJoyInfo():
    res = "Joystick Id: " + str(joystick.getId()) + "\n";
    res += "Number of axes: " + str(joystick.getNumAxes()) + "\n";
    res += "Number of buttons: " + str(joystick.getNumButtons()) + "\n";
    # To get axis value, use joystick.getAxisVal(id);
    # To get button state, use joystick.getButtonVal(id);
    terminal.appendText(res);

def joystick_selected(idx):
    global joystick;

    if idx == -1:
        return;

    terminal.appendText("Joystick \"" + joySelect.currentText + "\" selected\n");

    if joystick != None:
        lorris.closeJoystick(joystick);

    id_list = lorris.getJoystickIds();
    joystick = lorris.getJoystick(id_list[idx]);

    # If you don't wanna let user select the joystick and just
    # grab the first one, you can use getFirstJoystick() function.
    #joystick = lorris.getFirstJoystick()

    if joystick != None:
        # connect to signals
        joystick.connect("axesChanged(QList<int>)", axesChanged);
        joystick.connect("buttonChanged(int, quint8)", buttonChanged);

        # SEE THIS: signal timer - maximal frequency of signals, default is 50 ms
        joystick.setSignalTimer(50);

        dumpJoyInfo();

def init():
    global joySelect

    terminal.clear();

    # create input widget and joystick selection
    inputW = lorris.newWidget(WIDGET_INPUT, "Joystick", 200, 80, script.width+20, 0);
    label = inputW.newWidget("QLabel", 1);
    label.setText("Select joystick:");

    joySelect = inputW.newWidget("QComboBox");
    joySelect.connect("currentIndexChanged(int)", joystick_selected);

    # Add joystick names
    lorris.AddComboBoxItems(joySelect, lorris.getJoystickNames());
init()
