// Make ScriptWidget work as terminal

function onDataChanged(data, dev, cmd, index) {
    appendTerm(data);
}

function onKeyPress(key) {
    sendData(new Array(key.charCodeAt(0)));
}

// Input line - right click to script -> show input line
function interactiveInput()
{
    sendData(inputLine.text);
    inputLine.clear();
}
inputLine.returnPressed.connect(interactiveInput);
