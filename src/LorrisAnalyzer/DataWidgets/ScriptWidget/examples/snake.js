// JavaScript implementation of Snake game for Lorris
// Controls: click to terminal and press:
//    r - start/stop
//    p - pause
//    wasd - control snake's movement

var MOVE_UP    = 0;
var MOVE_DOWN  = 1;
var MOVE_LEFT  = 2;
var MOVE_RIGHT = 3;

var STEP_LEN = script.height;

var timer = newTimer();
var direction = MOVE_RIGHT;
var run = false;
var tail;
var cookie = null;

var scoreW = newNumberWidget("Score", 150, 0);
moveWidget(scoreW, area.width - 150, 0);
var score = 0;

var moveQueue;

resizeWidget(script, 90, 90);
clearTerm();

function Pos(x, y)
{
    this.x = x;
    this.y = y;
}

function onKeyPress(key) {
    switch(key)
    {
        case "r":
            if(run)
                timer.stop();
            else
                start();
            run = !run;
            return;
        case "p":
        {
            if(!run)
                return;
            
            if(timer.active)
                timer.stop();
            else
                timer.start(400);
            return;
        }
    }

    if(!timer.active)
        return;

    var hasTail = tail != null && tail.lenght != 0;

    switch(key)
    {
        case "w":
            if(!hasTail || direction != MOVE_DOWN)
                moveQueue.push(MOVE_UP);
            break;
        case "a":
            if(!hasTail || direction != MOVE_RIGHT)
                moveQueue.push(MOVE_LEFT);
            break;
        case "s":
            if(!hasTail || direction != MOVE_UP)
                moveQueue.push(MOVE_DOWN);
            break;
        case "d":
            if(!hasTail || direction != MOVE_LEFT)
                moveQueue.push(MOVE_RIGHT);
            break;
    }
}

function start()
{
    moveWidget(script, 0, 0);

    direction = MOVE_RIGHT;
    score = 0;
    scoreW.setValue(0);
    
    for(var i = 0; tail != null && i < tail.length; ++i)
        tail[i].remove();
    tail = new Array();

    moveQueue = new Array();

    if(cookie != null)
        cookie.remove();

    spawnCookie();
    
    timer.start(300);
}

function moveAll()
{
    if(moveQueue.length == 0)
        moveQueue.push(direction);

    while(moveQueue.length != 0)
    {
        direction = moveQueue.shift();
        var prevPos = new Pos(script.x + 5, script.y + 5);
        move(script, direction);
    
        for(var i = 0; i < tail.length; ++i)
        {
            var pos = new Pos(tail[i].x, tail[i].y);
            moveWidget(tail[i], prevPos.x, prevPos.y);
            prevPos = pos;
        }
    }

    if(!checkCollision())
        return;

    if(rectOverlaps(script, cookie))
        consumeCookie();
}
timer.timeout.connect(moveAll);

function move(widget, dir)
{
    var x = widget.x;
    var y = widget.y;
    switch(dir)
    {
        case MOVE_UP:
            y -= STEP_LEN;
            break;
        case MOVE_DOWN:
            y += STEP_LEN;
            break;
        case MOVE_LEFT:
            x -= STEP_LEN;
            break;
        case MOVE_RIGHT:
            x += STEP_LEN;
            break;
    }

    if(y < 0)                y = area.height + y;
    else if(y > area.height) y = y - area.height;

    if(x < 0)               x = area.width + x;
    else if(x > area.width)    x = x - area.width;

    moveWidget(widget, x, y);
}

var cookieColor;
function spawnCookie()
{
    var x = rand(area.width-110);
    var y = rand(area.height-110);

    cookie = newColorWidget("Cookie", 110, 110);
    cookieColor = new Array(rand(255), rand(255), rand(255));
    cookie.setValue(cookieColor[0], cookieColor[1], cookieColor[2]);

    moveWidget(cookie, x, y);
}

function rectOverlaps(first, second)
{
    var f_xw = first.x + first.width;
    var f_yh = first.y + first.height;
    var s_xw = second.x + second.width;
    var s_yh = second.y + second.height;
    
    var xOverlap = valueInRange(first.x, second.x, s_xw) || valueInRange(second.x, first.x, f_xw);
    var yOverlap = valueInRange(first.y, second.y, s_yh) || valueInRange(second.y, first.y, f_yh);

    return (xOverlap && yOverlap);
}

function valueInRange(val, min, max)
{
    return (val >= min) && (val <= max);
}

function consumeCookie()
{
    var tailpart = newColorWidget("T", script.width-10, script.height-10);
    tailpart.setValue(cookieColor[0], cookieColor[1], cookieColor[2]);

    cookie.remove();
    spawnCookie();

    var p = new Pos(0, 0);
    if(tail.length != 0)
    {
        p.x = tail[tail.length - 1].x;
        p.y = tail[tail.length - 1].y;
    }
    else
    {
        p.x = script.x + 5;
        p.y = script.y + 5;
    }

    moveWidget(tailpart, p.x, p.y);
    tail.push(tailpart);

    score += 10;

    scoreW.setValue(score);
}

function checkCollision()
{
    for(var i = 0; i < tail.length; ++i)
    {
        if(rectOverlaps(script, tail[i]))
        {
            run = false;
            timer.stop();
            throwException("Game Over!");
            return false;
        }
    }
    return true;
}

function rand(max)
{
    return Math.floor(Math.random() * max);
}
