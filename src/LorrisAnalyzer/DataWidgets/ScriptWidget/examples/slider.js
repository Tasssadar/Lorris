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
function Slider_valueChanged()
{
    appendTerm("value changed: " + Slider.getValue() + "\n");
}

// called whenever the minimum value is changed
function Slider_minimumChanged()
{
    appendTerm("minimum changed: " + Slider.getMin() + "\n");
}

// called whenever the maximum value is changed
function Slider_maximumChanged()
{
    appendTerm("maximum changed: " + Slider.getMax() + "\n");
}

// called whenever the data type is changed
function Slider_typeChanged()
{
    appendTerm("type changed to " + (Slider.isInteger() ? "Integer" : "Double") + "\n");
}

// called whenever the orientation is changed
function Slider_orientationChanged()
{
    appendTerm("orientation changed to " + Slider.getOrientation() + "\n");
}

// called whenever the visibility of min/max boxes is changed
function Slider_visibilityChanged()
{
    appendTerm("min/max visibility changed: " + (Slider.isMinMaxVisible() ? "Visible" : "Hidden") + "\n");
}
