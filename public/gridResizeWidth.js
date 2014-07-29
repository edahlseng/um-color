window.addEventListener("resize", resizeHandler);
window.addEventListener("load", function () {
	document.body.addEventListener("overflowchanged", resizeHandler); // this will fire when a scrollbar appears
	resizeHandler();
});

// set the margin size as the margin-left in your stylesheet
// make sure the elements have the proper class (matches the one you specify here)
// make sure the elements have display: inline-block;
var desiredResizeClass = "test";
var desiredNumberPerLine = 4;
var minimumWidth = 300;
var heightPerUnitWidth = .5;

function resizeHandler()
{
	var containerWidth = document.body.clientWidth;

	// get an array of all of the video elements
	var nodeList = document.querySelectorAll('.' + desiredResizeClass);
	var marginLeft = parseInt(getComputedStyle(nodeList[0]).marginLeft);
	var borderWidth = parseInt(getComputedStyle(nodeList[0]).borderLeftWidth) + parseInt(getComputedStyle(nodeList[0]).borderLeftWidth);

	// attempt to set the proper width
	var width;
	for (var i = desiredNumberPerLine; i > 0; i--)
	{
		var marginSpace = (i + 1) * marginLeft;
		var width = (containerWidth - marginSpace) / i;
		if (width > minimumWidth) {
			break;
		}
	}

	// if we have declared a desired aspect ratio, use it
	var height;
	if (heightPerUnitWidth) {
		height = width * heightPerUnitWidth;
	}

	// account for the border width if we must
	if (borderWidth) {
		width -= borderWidth;
	}


	// get the styleSheet that has the rule for the video elements, and edit it
	for (var i = document.styleSheets.length - 1; i >= 0; i--)
	{
		var styleSheet = document.styleSheets[i];

		// get the rule for the video elements
		var rules = styleSheet.cssRules;		

		// determine which rule affects the video elements
		for (var i = rules.length - 1; i >= 0; i--)
		{
			if (rules[i].selectorText == '.' + desiredResizeClass)
			{
				// set the style
				rules[i].style.width = width + "px";
				if (height) { rules[i].style.height = height + "px"; }
			}
		}
	}
}