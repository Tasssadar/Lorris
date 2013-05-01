# Input widget example.
# Create script widget with this script and then add
# new input widget to workspace

inputW = None;
clearBtn = None;
inputLine = None;
createBtn = None;

def createWidgets():
    global clearBtn
    global inputW
    global inputLine

    # create widgets. You can create any Qt Widget
    clearBtn = inputW.newWidget("QPushButton")
    clearBtn.text = "Clear"
    clearBtn.connect("clicked()", clearBtn_clicked)

    inputLine = inputW.newWidget("QLineEdit")
    inputLine.connect("returnPressed()", inputLine_returnPressed)

    # you can use idString to find widgets later
    inputW.newWidget("QLabel", "label")
    inputW.get("label").text = "Test label"

    inputW.setHorizontal(True) # Changes orientation of layout in widget

def onWidgetAdd(widget, name):
    global inputW

    if inputW != None or widget.widgetType != WIDGET_INPUT:
        return

    inputW = widget
    lorris.resizeWidget(widget, 200, 80)
    createWidgets()

def clearBtn_clicked():
    global inputW
    global createBtn

    inputW.clear() # remove all widgets

    createBtn = inputW.newWidget("QPushButton")
    createBtn.text = "Create again"
    createBtn.connect("clicked()", createBtn_clicked)

def createBtn_clicked():
    global createBtn
    global inputW

    inputW.removeWidget(createBtn) # remove by pointer
    inputW.removeWidget("label") # remove by idString
    createWidgets()

def inputLine_returnPressed():
    global inputLine

    terminal.appendText(inputLine.text + "\n")
    inputLine.clear()
