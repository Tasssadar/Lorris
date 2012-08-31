/*! \addtogroup qtscript
 * QtString native methods
 * 
 * Default script:
 *  \include default.js
 * 
 * More examples can be found int "Examples" section
 * @{
 */
#ifndef QT_SCRIPT
#define QT_SCRIPT

void clearTerm();
void appendTerm(String str);
void appendTerm(Array<int> data);
void sendData(String str);
void sendData(Array<int> data);
int getWidth();
int getHeight();
void throwException(String text);
Joystick *getJoystick(int id);
void closeJoystick(Joystick *joy);
Array<String> getJoystickNames();
Array<int> getJoystickIds();
/// \brief returns <a href="http://qt-project.org/doc/qt-4.8/QTimer.html">QTimer</a>
QTimer *newTimer();
void addComboBoxItems(QComboBox *box, Array<String> items);
void moveWidget(DataWidget *widget, int x, int y);
void resizeWidget(DataWidget *widget, int width, int height);
/// \brief x and y are relative to parent ScriptWidget
DataWidget *newWidget(int type, String name, int w = 0, int h = 0, int x = 0, int y = 0);

/*!
 * \brief This enum is to be used with newWidget. Example: `newWidget(WIDGET_BUTTON, "button", 100, 80, 0, 0);`
 */
enum
{
    WIDGET_NUMBERS,
    WIDGET_BAR,
    WIDGET_COLOR,
    WIDGET_GRAPH,
    WIDGET_SCRIPT,
    WIDGET_INPUT,
    WIDGET_TERMINAL,
    WIDGET_BUTTON,
    WIDGET_CIRCLE,
    WIDGET_SLIDER,
    WIDGET_CANVAS,
};

/*!
 * \brief This enum is to be used with setDataType() methods. Example: `numerWidget.setDataType(NUM_UINT16);`
 */
enum
{
    NUM_UINT8 ,
    NUM_UINT16,
    NUM_UINT32,
    NUM_UINT64,

    NUM_INT8,
    NUM_INT16,
    NUM_INT32,
    NUM_INT64,

    NUM_FLOAT,
    NUM_DOUBLE,
};

/*!
 * \example default.js
 * Default QtScript
 * 
 * \example canvas.js
 * QtScript canvas example
 * 
 * \example graph.js
 * QtScript graph example
 * 
 * \example input.js
 * QtScript input widget example
 * 
 * \example joystick.js
 * QtScript joystick example
 * 
 * \example slider.js
 * QtScript sliderwidget example
 * 
 * \example snake.js
 * QtScript snake game implementation
 * 
 * \example terminal.js
 * QtScript terminal script
 */
/*! @} */

#endif