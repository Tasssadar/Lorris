// Graph widget example.
// you can comment out following line and create widget graph with title "graph"
var graph = newGraphWidget("Graph", 700, 300, script.width + 20, 0);

// set graph scale
graph.setAxisScale(false, -1.2, 1.2); // for y axis
graph.setAxisScale(true, 0, 200); // for x axis

var sin = null;
var cos = null;
var tg = null;

var names = new Array("sin", "cos", "tan");

// This function is called on key press in terminal.
// Param is string
function onKeyPress(key) {
	switch(key)
	{
		case "s":
			if(sin == null)
				sin = addCurve(0);
			break;
		case "c":
			if(cos == null)
				cos = addCurve(1);
			break;
		case "t":
			if(tg == null)
				tg = addCurve(2);
			break;
		case "r": // Remove curves - they are indetified by name
			for(var i = 0; i < names.length; ++i)
				graph.removeCurve(names[i]);
			sin = null;
			cos = null;
			tg = null;
			break;
		case "m": // scroll graph so that it is at its maximal value of x axis
			graph.updateVisibleArea();
			break;
	}	
}

function addCurve(type)
{
	var colors = new Array("red", "blue", "green");
    
	// Add curve - first is name, second is color (as in html - name or hex #FFFFFF)
	var c = graph.addCurve(names[type], colors[type]);

	var x = 0;
	for(var i = 0; i < 500; ++i)
	{
		// Add point to graph. first is index, second is value
		// index IS NOT value on x axis, but data will be ordered by this index
		// (point with idx 10 will be always before point with idx 50, even is it was inserted after)
        // if you use same index twice, value if that index will be changed
		switch(type)
		{
			case 0: c.addPoint(i, Math.sin(x)); break; 
			case 1: c.addPoint(i, Math.cos(x)); break;
			case 2: c.addPoint(i, Math.tan(x)); break;
		}
		x += 0.1;
	}
	return c;
}