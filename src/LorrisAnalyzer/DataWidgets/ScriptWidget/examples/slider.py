# Slider example
# You have to create SliderWidget by yourself and name it "Slider"
#
# I am using events here, syntax is *widget_title*_*event_name*,
# eg. Slider's valueChanged event is Slider_valueChanged()

terminal.clear();

def onKeyPress(key):
    if key == "c":
        terminal.clear();

    if key == "r":
        # SEE THIS: Setting slider values from script
        Slider.setDouble();
        Slider.setMin(-55.5);
        Slider.setMax(1000.433);
        Slider.setValue(50.5);

# called whenever the value is changed
def Slider_valueChanged():
    terminal.appendText("value changed: " + str(Slider.getValue()) + "\n");

# called whenever the minimum value is changed
def Slider_minimumChanged():
    terminal.appendText("minimum changed: " + str(Slider.getMin()) + "\n");

# called whenever the maximum value is changed
def Slider_maximumChanged():
    terminal.appendText("maximum changed: " + str(Slider.getMax()) + "\n");

# called whenever the data type is changed
def Slider_typeChanged():
    terminal.appendText("type changed to " + ("Integer" if Slider.isInteger() else "Double") + "\n");
