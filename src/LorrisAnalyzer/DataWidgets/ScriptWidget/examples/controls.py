# Controls & data sending
# Needs these widgets in the area:
#  * ButtonWidget named "Button"
#  * SliderWidget named "Slider"

# Simulates sending data packets with following header to the device:
# 0x80 [COMMAND] [LENGTH] [DATA]

# Send Button status
def Button_clicked():
    lorris.sendData([ 0x80, 0x1, 0x1, 0x1]);

def Button_pressed():
    lorris.sendData([ 0x80, 0x2, 0x1, 0x1]);

def Button_released():
    lorris.sendData([ 0x80, 0x2, 0x1, 0x0]);

# Send slider values as 16-bit unsigned int as big endian
def Slider_valueChanged(val):
    val = int(val); # make sure it is an integer, not double
    lorris.sendData([ 0x80, 0x3, 0x2,
        ((val >> 8) & 0xFF), (val & 0xFF) ]);
