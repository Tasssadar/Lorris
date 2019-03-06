// Slider example
// You have to create SliderWidget by yourself and name it "Slider"
//
// I am using events here, syntax is *widget_title*_*event_name*,
// eg. Slider's valueChanged event is Slider_valueChanged()

clearTerm();

function onKeyPress(key)
{
    if(key == "c")
        clearTerm();

    if(key == "r")
    {
        // SEE THIS: Setting slider values from script
        Slider.setDouble();
        Slider.setMin(-55.5);
        Slider.setMax(1000.433);
        Slider.setValue(50.5);
        //Slider.setRange(min, max, step) <-- also possible

        // like in menu - 0 - hor, l -> r; 1 - ver, b -> t; 2 - hor, r -> l; 3 - ver, t -> b
        Slider.setOrientation(1);

        Slider.setValue(50.5);
        Slider.hideMinMax(true);
    }
}

// called whenever the value is changed
function Slider_valueChanged(val)
{
    appendTerm("value changed: " + val + "\n");
}

// called whenever the minimum value is changed
function Slider_minimumChanged(min)
{
    appendTerm("minimum changed: " + min + "\n");
}

// called whenever the maximum value is changed
function Slider_maximumChanged(max)
{
    appendTerm("maximum changed: " + max + "\n");
}

// called whenever the data type is changed
function Slider_typeChanged(isDouble)
{
    appendTerm("type changed to " + (isDouble ? "Double" : "Integer") + "\n");
}

// called whenever the orientation is changed
function Slider_orientationChanged(orientation)
{
    appendTerm("orientation changed to " + orientation + "\n");
}

// called whenever the visibility of min/max boxes is changed
function Slider_visibilityChanged(isVisible)
{
    appendTerm("min/max visibility changed: " + (isVisible ? "Visible" : "Hidden") + "\n");
}
