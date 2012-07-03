# Python implementation of Snake game for Lorris
# Controls: click to terminal and press:
#    r - start/stop
#    p - pause
#    wasd - control snake's movement

from random import *

MOVE_UP    = 0
MOVE_DOWN  = 1
MOVE_LEFT  = 2
MOVE_RIGHT = 3

STEP_LEN = script.height

terminal.clear()
lorris.resizeWidget(script, 90, 90);

timer = lorris.newTimer()
direction = MOVE_RIGHT
run = False
cookie = None
cookieColor = []
tail = []

scoreW = lorris.newWidget(WIDGET_NUMBER, 'Score', 150, 0)
lorris.moveWidget(scoreW, area.width - 150, 0)
score = 0

moveQueue = []

def startStop():
    global run
    if run:
        timer.stop();
    else:
        start();
    run = not run

def pause():
    if not run:
        return
    if timer.active:
        timer.stop()
    else:
        timer.start(400)

def onKeyPress(key):
    if key == 'r': startStop()
    elif key == 'p': pause()

    if not timer.active:
        return

    hasTail = (tail != None and len(tail) != 0)

    if key == 'w' and (not hasTail or direction != MOVE_DOWN):
        moveQueue.append(MOVE_UP)
    elif key == 'a' and (not hasTail or direction != MOVE_RIGHT):
        moveQueue.append(MOVE_LEFT)
    elif key == 's' and (not hasTail or direction != MOVE_UP):
        moveQueue.append(MOVE_DOWN)
    elif key == 'd' and (not hasTail or direction != MOVE_LEFT):
        moveQueue.append(MOVE_RIGHT)

def start():
    global direction
    global scoreW
    global tail
    global moveQueue
    global cookie
    global timer

    lorris.moveWidget(script, 0, 0);
    
    direction = MOVE_RIGHT
    score = 0
    scoreW.setValue(0)

    while len(tail) != 0:
        tail.pop().remove()

    del moveQueue[:]

    if cookie != None:
        cookie.remove();

    spawnCookie();

    timer.start(400);

def moveAll():
    global moveQueue
    global direction
    if len(moveQueue) == 0:
        moveQueue.append(direction);
    
    while len(moveQueue) != 0:
        direction = moveQueue.pop(0)
        prevPos = [script.x + 5, script.y + 5 ]
        move(script, direction)

        for i in tail:
            pos = [i.x, i.y]
            lorris.moveWidget(i, prevPos[0], prevPos[1])
            prevPos = pos

    if not checkCollision():
        return

    if rectOverlaps(script, cookie):
        consumeCookie()

timer.connect('timeout()', moveAll);

def move(widget, dir):
    x = widget.x
    y = widget.y

    if dir == MOVE_UP: y -= STEP_LEN
    elif dir == MOVE_DOWN: y += STEP_LEN
    elif dir == MOVE_LEFT: x -= STEP_LEN
    elif dir == MOVE_RIGHT: x += STEP_LEN

    if y < 0: y = area.height + y
    elif y > area.height: y = y - area.height

    if x < 0: x = area.width + x
    elif x > area.width: x = x - area.width

    lorris.moveWidget(widget, x, y)

def spawnCookie():
    global cookieColor
    global cookie

    x = randint(0, area.width-110)
    y = randint(0, area.height-110)

    cookie = lorris.newWidget(WIDGET_COLOR, "Cookie", 110, 110)
    cookieColor = [ randint(0, 255), randint(0, 255), randint(0, 255) ]
    cookie.setValue(cookieColor[0], cookieColor[1], cookieColor[2])

    lorris.moveWidget(cookie, x, y)

def rectOverlaps(first, second):
    f_xw = first.x + first.width
    f_yh = first.y + first.height
    s_xw = second.x + second.width;
    s_yh = second.y + second.height;

    xOverlap = valueInRange(first.x, second.x, s_xw) or valueInRange(second.x, first.x, f_xw)
    yOverlap = valueInRange(first.y, second.y, s_yh) or valueInRange(second.y, first.y, f_yh)
    return xOverlap and yOverlap

def valueInRange(val, min, max):
    return val >= min and val <= max

def consumeCookie():
    global tail
    global score
    global tail
    global scoreW

    tailpart = lorris.newWidget(WIDGET_COLOR, "T", script.width-10, script.height-10)
    tailpart.setValue(cookieColor[0], cookieColor[1], cookieColor[2])
    
    cookie.remove()
    spawnCookie()

    p = [0, 0]
    if len(tail) != 0:
        p[0] = tail[len(tail)-1].x
        p[1] = tail[len(tail)-1].y
    else:
        p[0] = script.x + 5
        p[1] = script.y + 5

    lorris.moveWidget(tailpart, p[0], p[1])
    tail.append(tailpart)

    score += 10
    scoreW.setValue(score)

def checkCollision():
    global run
    global timer
    for i in tail:
        if rectOverlaps(script, i):
            run = False
            timer.stop()
            lorris.throwException("Game Over!")
            return False
    return True
