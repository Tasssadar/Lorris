//! [signal example]
button = lorris.newWidget(WIDGET_BUTTON, "button", 100, 50)
def clicked():
    terminal.appendText("clicked\n")

button.connect("clicked()", clicked); # connecting slot to signal
//! [signal example]
//! [property example]
button = lorris.newWidget(WIDGET_BUTTON, "button", 100, 50)
terminal.appendText("widget type: " + str(button.widgetType) + "\n")
//! [property example]
