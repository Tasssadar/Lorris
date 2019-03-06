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
        #Slider.setRange(min, max, step) <-- also possible

        # like in menu - 0 - hor, l -> r; 1 - ver, b -> t; 2 - hor, r -> l; 3 - ver, t -> b
        Slider.setOrientation(1);

        Slider.setValue(50.5);
        Slider.hideMinMax(True);

# called whenever the value is changed
def Slider_valueChanged(val):
    terminal.appendText("value changed: " + str(val) + "\n");

# called whenever the minimum value is changed
def Slider_minimumChanged(min):
    terminal.appendText("minimum changed: " + str(min) + "\n");

# called whenever the maximum value is changed
def Slider_maximumChanged(max):
    terminal.appendText("maximum changed: " + str(max) + "\n");

# called whenever the data type is changed
def Slider_typeChanged(isDouble):
    terminal.appendText("type changed to " + ("Integer" if not isDouble else "Double") + "\n");

# called whenever the orientation is changed
def Slider_orientationChanged(orientation):
    terminal.appendText("orientation changed to " + str(orientation) + "\n");

# called whenever the visibility of min/max boxes is changed
def Slider_visibilityChanged(visible):
    terminal.appendText("min/max visibility changed: " + ("Visible" if visible else "Hidden") + "\n");
