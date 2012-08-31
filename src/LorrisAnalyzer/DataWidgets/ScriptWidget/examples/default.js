// You can use clearTerm() and appendTerm(string) to set term content
// You can use sendData(Array of ints) to send data to device. It expects array of uint8s

// This function gets called on data received
// it should return string, which is automatically appended to terminal
function onDataChanged(data, dev, cmd, index) {
    return "";
}

// This function is called on key press in terminal.
// Param is string
function onKeyPress(key) {

}

// This function is called when data arrives to serial port
// parameter is array with unparsed data
function onRawData(data) {

}

// Called when new widget is added.
// widget is widget's object, name is string
function onWidgetAdd(widget, name) {

}

// Called when new widget is removed.
// widget is widget's object, name is string
function onWidgetRemove(widget, name) {

}

// Called when this script instance is destroyed.
// useful for saving data.
function onScriptExit() {

}

// Called when this analyzer session is saved to data file.
// Useful for saving data.
function onSave() {

}
