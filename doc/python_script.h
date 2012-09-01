/** Python native methods
 * Default script:
 *  \include default.py
 * 
 * More examples can be found int "Examples" section
 */
#ifndef PYTHON_SCRIPT
#define PYTHON_SCRIPT
class lorris
{
public:
    enum WidgetTypes
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
        WIDGET_STATUS
    };
    
    enum NumberTypes
    {
        NUM_UINT8,
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
    
public slots:
    void sendData(const QByteArray& data);
    void sendData(const QString& str);
    int getWidth();
    int getHeight();
    void throwException(const QString& text);
    Joystick *getJoystick(int id);
    void closeJoystick(Joystick *joy);
    QStringList getJoystickNames();
    QList<quint32> getJoystickIds();
    QTimer *newTimer();
    void AddComboBoxItems(QComboBox *box, QStringList items);
    void moveWidget(QWidget *w, int x, int y);
    void resizeWidget(QWidget *w, int width, int height);
    DataWidget *newWidget(int type, QString title, int width, int height, int x, int y);
    DataWidget *newWidget(int type, QString title, int width, int height)
    {
        return newWidget(type, title, width, height, -1, -1);
    }
    DataWidget *newWidget(int type, QString title)
    {
        return newWidget(type, title, -1, -1, -1, -1);
    }  
};

/*!
 * \enum lorris::WidgetTypes
 * \brief This enum is to be used with newWidget. Example: `lorris.newWidget(WIDGET_BUTTON, "button", 100, 80, 0, 0);`
 */

/*!
 * \enum lorris::NumberTypes
 * \brief This enum is to be used with setDataType() methods. Example: `numerWidget.setDataType(NUM_UINT16);`
 */

/*!
 * \example default.py
 * Default python script
 * 
 * \example canvas.py
 * Python canvas example
 * 
 * \example graph.py
 * Python graph example
 * 
 * \example input.py
 * Python input widget example
 * 
 * \example joystick.py
 * Python joystick example
 * 
 * \example slider.py
 * Python sliderwidget example
 * 
 * \example snake.py
 * Python snake game implementation
 * 
 * \example terminal.py
 * Python terminal script
 */
#endif