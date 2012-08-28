// Slider example
// You have to create CanvasWidget by yourself and name it "Canvas"

Canvas.setBackground("white");

function onKeyPress(key)
{
    Canvas.clear(); // clears all lines
	if(key == "c")
    	return;


	Canvas.setBackground("black");
	
	// aplies only for lines drawn AFTER this call
	Canvas.setLineColor("white");
	Canvas.setLineSize(5);

	Canvas.drawLine(50, 50, 100, 50); // x1, y1, x2, y2

	Canvas.setLineColor("blue");
	Canvas.setLineSize(3);

	Canvas.drawLine(100, 100); // x2, y2 - x1 and y1 are x2 and y2 from last line

    // dimensions of the actual drawing space
    terminal.appendText(Canvas.cWidth + "x" + Canvas.cHeight + "\n");
}
