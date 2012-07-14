# Graph widget example.
import math

# you can comment out following line and create widget graph with title "graph"
graph = lorris.newWidget(WIDGET_GRAPH, "Graph", 700, 300, script.width + 20, 0);

# set graph scale
graph.setAxisScale(False, -1.2, 1.2); # for y axis
graph.setAxisScale(True, 0, 200); # for x axis

sin = None;
cos = None;
tg = None;

names = [ "sin", "cos", "tan" ] 

def onKeyPress(key):
	global names
	global cos
	global sin
	global tg

	if key == "s":
		if sin == None:
			sin = addCurve(0)
	elif key == "c":
		if cos == None:
			cos = addCurve(1)
	elif key == "t":
		if tg == None:
			tg = addCurve(2);
	elif key == "r": # Remove curves - they are indetified by name
		for name in names:
			graph.removeCurve(name);
		sin = cos = tg = None
	elif key == "m": # scroll graph so that it is at its maximal value of x axis
		graph.updateVisibleArea();

def addCurve(type):
	global names
	colors = [ "red", "blue", "green" ]
    
	# Add curve - first is name, second is color (as in html - name or hex #FFFFFF)
	c = graph.addCurve(names[type], colors[type]);

	x = 0;
	for i in range(500):
		# Add point to graph. first is index, second is value
		# index IS NOT value on x axis, but data will be ordered by this index
		# (point with idx 10 will be always before point with idx 50, even is it was inserted after)
        # if you use same index twice, value if that index will be changed
		if type == 0:
			c.addPoint(i, math.sin(x))
		elif type == 1:
			c.addPoint(i, math.cos(x))
		elif type == 2:
			c.addPoint(i, math.tan(x))
		x += 0.1;
	return c;
