// Make ScriptWidget work as terminal
var history = new Array("");
var cur_hist = 0;
var last_idx = -1;

function onDataChanged(data, dev, cmd, index) {
    if(index != last_idx)
    {
        appendTerm(data);
        last_idx = index;
    }
}

function onKeyPress(key) {
    sendData(key);
}

// Input line - right click to script -> show input line
function interactiveInput()
{
    sendData(inputLine.text);


    if(cur_hist != history.length-1)
        history.splice(cur_hist, 1);
    cur_hist = history.length;
    history[cur_hist-1] = inputLine.text;
    history.push("");

    inputLine.clear();
}

// code is Qt::Key (http://qt-project.org/doc/qt-4.8/qt.html#Key-enum)
function inputLineKeyPressed(code) {
    switch(code) {
    case 0x01000013: // Qt::Key_U
        if(cur_hist > 0) {
            --cur_hist;
            inputLine.setText(history[cur_hist]);
        }
        break;
    case 0x01000015: // Qt::Key_Down
        if(cur_hist < history.length-1) {
            ++cur_hist;
            inputLine.setText(history[cur_hist]);
        }
        break;
    }
}

function inputLineKeyReleased(code) {
    return;
}

inputLine.returnPressed.connect(interactiveInput);
