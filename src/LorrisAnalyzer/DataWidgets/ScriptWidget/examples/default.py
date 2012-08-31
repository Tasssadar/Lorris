# You can use terminal.clear() and terminal.appendText(string) to set term content
# You can use lorris.sendData(QByteArray) to send data to device.

# This function gets called on data received
# it should return string, which is automatically appended to terminal
def onDataChanged(data, dev, cmd, index):
    return ""

# This function is called on key press in terminal.
# Param is string
def onKeyPress(key):
    return

# This function is called when data arrives to serial port
# parameter is array with unparsed data
def onRawData(data):
    return

# Called when new widget is added.
# widget is widget's object, name is string
def onWidgetAdd(widget, name):
    return

# Called when new widget is removed.
# widget is widget's object, name is string
def onWidgetRemove(widget, name):
    return

# Called when this script instance is destroyed.
# useful for saving data.
def onScriptExit():
    return

# Called when this analyzer session is saved to data file.
# Useful for saving data.
def onSave():
    return
