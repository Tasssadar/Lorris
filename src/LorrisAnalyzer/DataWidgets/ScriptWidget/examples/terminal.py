#  Make ScriptWidget work as terminal

def onDataChanged(data, dev, cmd, index):
    terminal.appendText(data)
    return ""

def onKeyPress(key):
    lorris.sendData(key)

# Input line - right click to script -> show input line
def interactiveInput():
    lorris.sendData(inputLine.text)
    inputLine.clear()
inputLine.connect("returnPressed()", interactiveInput);
