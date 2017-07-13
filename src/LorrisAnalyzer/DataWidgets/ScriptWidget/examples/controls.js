// Controls & data sending
// Needs these widgets on the widgetArea:
//  * ButtonWidget named "Button"
//  * SliderWidget named "Slider"

// Simulates sending data packets with following header to the device:
// 0x80 [COMMAND] [LENGTH] [DATA]

// Send Button status
function Button_clicked() {
    sendData([ 0x80, 0x1, 0x1, 0x1]);
}

function Button_pressed() {
    sendData([ 0x80, 0x2, 0x1, 0x1]);
}

function Button_released() {
    sendData([ 0x80, 0x2, 0x1, 0x0]);
}

// Send slider values as 16-bit unsigned int as big endian
function Slider_valueChanged(val) {
    val = val | 0; // make sure it is an integer, not double
    sendData([ 0x80, 0x3, 0x2, ((val >> 8) & 0xFF), (val & 0xFF)]);
}
