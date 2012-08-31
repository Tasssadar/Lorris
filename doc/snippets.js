//! [slot example]
terminal.appendText("Hello world!\n");
terminal.clear();
//! [slot example]
//! [signal example]
var button = newWidget(WIDGET_BUTTON, "button", 100, 50);
function clicked() {
    terminal.appendText("clicked\n");
}
button['clicked()'].connect(clicked); // connecting slot to signal
//! [signal example]
//! [property example]
var button = newWidget(WIDGET_BUTTON, "button", 100, 50);
terminal.appendText("widget type: " + button.widgetType + "\n");
//! [property example]
