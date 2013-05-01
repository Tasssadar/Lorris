// Input widget example.
// Create script widget with this script and then add
// new input widget to workspace

var inputW = null;
var clearBtn = null;
var inputLine = null;
var createBtn = null;

function createWidgets()
{
    // create widgets. You can create any Qt Widget
    clearBtn = inputW.newWidget("QPushButton");
    clearBtn.text = "Clear";
    clearBtn.clicked.connect(clearBtn_clicked);

    inputLine = inputW.newWidget("QLineEdit");
    inputLine.returnPressed.connect(inputLine_returnPressed);

    // you can use idString to find widgets later
    inputW.newWidget("QLabel", "label");
    inputW.get("label").text = "Test label";

    inputW.setHorizontal(true); // Changes orientation of layout in widget
}

function onWidgetAdd(widget, name)
{
    if(inputW != null || widget.widgetType != WIDGET_INPUT)
        return;

    inputW = widget;
    resizeWidget(widget, 200, 80);
    createWidgets();
}

function clearBtn_clicked()
{
    inputW.clear(); // remove all widgets

    createBtn = inputW.newWidget("QPushButton");
    createBtn.text = "Create again";
    createBtn.clicked.connect(createBtn_clicked);
}

function createBtn_clicked()
{
    inputW.removeWidget(createBtn); // remove by pointer
    inputW.removeWidget("label") // remove by idString
    createWidgets();
}

function inputLine_returnPressed()
{
    terminal.appendText(inputLine.text + "\n");
    inputLine.clear();
}
