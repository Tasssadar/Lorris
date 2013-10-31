// Joystick example

// Create joystick select widgets
var joySelect = null;
var joystick = null;

(function init()
{
    clearTerm();

    // create input widget and joystick selection
    var inputW = newInputWidget("Joystick", 200, 80, script.width+20, 0);
    var label = inputW.newWidget("QLabel", 1);
    label.setText("Select joystick:");

    joySelect = inputW.newWidget("QComboBox");
    joySelect["currentIndexChanged(int)"].connect(joystick_selected);

    // Add joystick names
    addComboBoxItems(joySelect, getJoystickNames());
})();

function joystick_selected(idx)
{
    if(idx == -1)
        return;

    appendTerm("Joystick \"" + joySelect.currentText + "\" selected\n");

    if(joystick != null)
        closeJoystick(joystick);

    var id_list = getJoystickIds();
    joystick = getJoystick(id_list[idx]);

    // If you don't wanna let user select the joystick and just
    // grab the first one, you can use getFirstJoystick() function.
    //joystick = getFirstJoystick();

    if(joystick)
    {
        // connect to signals
        joystick["axesChanged(QList<int>)"].connect(axesChanged);
        joystick["buttonChanged(int, quint8)"].connect(buttonChanged);

        // SEE THIS: signal timer - maximal frequency of signals, default is 50 ms
        joystick.setSignalTimer(50);

        dumpJoyInfo();
    }
}

function dumpJoyInfo()
{
    var str = "Joystick Id: " + joystick.getId() + "\n";
    str += "Number of axes: " + joystick.getNumAxes() + "\n";
    str += "Number of buttons: " + joystick.getNumButtons() + "\n";
    // To get axis value, use joystick.getAxisVal(id);
    // To get button state, use joystick.getButtonVal(id);
    appendTerm(str);
}

// Called on axes change. "axes" is Array of ints with ids of axes which were changed
function axesChanged(axes)
{
    appendTerm("Axes changed: " + axes + "\n");
    var str = "";
    for(var i = 0; i < joystick.getNumAxes(); ++i)
        str += i + ": " + joystick.getAxisVal(i) + ", "; // SEE THIS: Get actual axis value. From -32768 to 32767
    appendTerm(str + "\n\n");
}

// Called when button status is changed, for each button separatedly
// Pressed: state == 1
function buttonChanged(id, state)
{
    appendTerm("Button " + id + ", state " + state + "\n");
}
