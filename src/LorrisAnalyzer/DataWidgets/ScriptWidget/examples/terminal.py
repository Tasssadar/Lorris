#  Make ScriptWidget work as terminal
history = [""]
cur_hist = 0
last_idx = -1

def onDataChanged(data, dev, cmd, index):
    if last_idx == index:
        return ""
    terminal.appendText(data)
    last_idx = index
    return ""

def onKeyPress(key):
    lorris.sendData(key)

# Input line - right click to script -> show input line
def interactiveInput():
    global history
    global cur_hist

    lorris.sendData(inputLine.text)

    if cur_hist != len(history)-1:
        history.pop(cur_hist)
    cur_hist = len(history)
    history[cur_hist-1] = inputLine.text;
    history.append("");

    inputLine.clear()

# code is Qt::Key (http://qt-project.org/doc/qt-4.8/qt.html#Key-enum)
def inputLineKeyPressed(code):
    global history
    global cur_hist
    if code == 0x01000013 and cur_hist > 0: # Qt::Key_Up
        cur_hist -= 1
        inputLine.setText(history[cur_hist])
    elif code == 0x01000015 and cur_hist < len(history)-1: # Qt::Key_Down
        cur_hist += 1
        inputLine.setText(history[cur_hist])

def inputLineKeyReleased(code):
    pass

inputLine.connect("returnPressed()", interactiveInput);
