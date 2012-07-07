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
