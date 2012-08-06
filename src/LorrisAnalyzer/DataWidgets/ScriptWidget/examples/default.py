# You can use terminal.clear() and terminal.appendText(string) to set term content
# You can use lorris.sendData(QByteArray) to send data to device.

# This function gets called on data received
# it should return string, which is automatically appended to terminal
def onDataChanged(data, dev, cmd, index):
        return "";

# This function is called on key press in terminal.
# Param is string
def onKeyPress(key):
        return;
