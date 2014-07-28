#include <string> 			// for std::string
#include <cstring>			// for strcat
#include <unordered_map> 	// for unordered_map object
#include <math.h> 			// for pow
#include <vector>			// for vectors (arrays)
#include <sstream>			// for stringstream  // not needed??
#include <iostream>			// for std::hex // not needed??
#include <opencv2/highgui/highgui.hpp>		// for open cv stuff


//-------------------------------------
// helper functions
//-------------------------------------

// change these to modify the color analysis
int numberLUnits = 50.0;
int numberAUnits = 50.0;
int numberBUnits = 50.0;

// these are reference values for certain color coversions
float xreference = 95.047;
float yreference = 100.0;
float zreference = 108.883;

// these variables are defined before hand in order to speed up calculations that are done over and over
const float n1_24 = 1.0 / 2.4;
const float n6_29 = 6.0 / 29.0;
const float n29_6 = 29.0 / 6.0;
const float n4_29 = 4.0 / 29.0;
const float n1_3 = 1.0 / 3.0;
const float n6_29cubed = pow(n6_29, 3);
const float n29_6squared = pow(n29_6, 2);
const float n6_29squared = pow(n6_29, 2);

float pivot(float x)
{
	if (x > .04045) {
		return pow((x + .055) / 1.055, 2.4);
	}
	return x / 12.92;
}

float reversePivot(float x)
{
	if (x > 0.0031308) {
		return (1.055) * pow(x, n1_24) - 0.055;
	}
	return 12.92 * x;
}

float fOfX(float x)
{
	if (x > n6_29cubed) {
		return pow(x, n1_3);
	}
	return n1_3 * n29_6squared * x + n4_29;
}

float fInverseOfX(float x)
{
	if (x > n6_29) {
		return (x * x * x);
	}
	return 3.0 * n6_29squared * (x - n4_29);
}

//------------------------------------------
// Color Class
//------------------------------------------

class EMDcolor
{
	private:
		int _red;
		int _green;
		int _blue;
		float _l;
		float _a;
		float _b;
		char hexString[8];
		void calculateLabValues()
		{
			// first step in XYZ transformation
			float rlinear = pivot(_red / 255.0);
			float glinear = pivot(_green / 255.0);
			float blinear = pivot(_blue / 255.0);

			// calculate XYZ here
			// essentially a matrix calculation
			// need to confirm where to get these values
			float x = ((0.4124 * rlinear) + (0.3576 * glinear) + (0.1805 * blinear)) * 100;
			float y = ((0.2126 * rlinear) + (0.7152 * glinear) + (0.0722 * blinear)) * 100;
			float z = ((0.0193 * rlinear) + (0.1192 * glinear) + (0.9502 * blinear)) * 100;

			// calculate L*a*b* components
			_l = 116.0 * fOfX(y / yreference) - 16.0;
			_a = 500.0 * (fOfX(x / xreference) - fOfX(y / yreference));
			_b = 200.0 * (fOfX(y / yreference) - fOfX(z / zreference));
		}
	public:
	EMDcolor(int redValue, int greenValue, int blueValue)
	{
		_red = redValue;
		_green = greenValue;
		_blue = blueValue;

		_l = -1000;
		_a = -1000;
		_b = -1000;
	}
	float l() {
		if (_l == -1000)
		{
			calculateLabValues();
		}
		return _l;
	}
	float a() {
		if (_a == -1000) {
			calculateLabValues();
		}
		return _a;
	}
	float b() {
		if (_a == -1000) {
			calculateLabValues();
		}
		return _b;
	}

	int red() {
		return _red;
	}

	int green() {
		return _green;
	}

	int blue() {
		return _blue;
	}

	const char* toHexString(); // need to define this function separately because of SWIG requirements
};

const char* EMDcolor::toHexString() // consider returning std::string in the future instead
{
 	char redCString[3] = {'0', '0', '\0'}; // leave room for null character
	sprintf(redCString, "%02x", (int)fmin(_red, 255));
	std::string redHex = redCString;

	char greenCString[3] = {'0', '0', '\0'}; // leav room for null character
	sprintf(greenCString, "%02x", (int)fmin(_green, 255));
	std::string greenHex = greenCString;

	char blueCString[3]= {'0', '0', '\0'}; // leave room for null character
	sprintf(blueCString, "%02x", (int)fmin(_blue, 255));
	std::string blueHex = blueCString;

	std::strcpy(hexString, "#");
	std::strcat(hexString, redCString);
	std::strcat(hexString, greenCString);
	std::strcat(hexString, blueCString);

 	return hexString;
}

struct ColorCount {
	EMDcolor	*color;
	int 		count;
};

struct ColorGroup {					// change to ColorBins
	std::string 				groupName;
	std::vector<ColorCount> 	colors;
	int 						totalNumberOfColors;
};

EMDcolor* average(ColorGroup colorGroup)
{
	// calculate the average color using Lab values
	float averageL = 0;
	float averageA = 0;
	float averageB = 0;
	for (std::vector<ColorCount>::iterator iterator = colorGroup.colors.begin(); iterator != colorGroup.colors.end(); ++iterator)
	{
		averageL += (*iterator).color->l() * (*iterator).count;
		averageA += (*iterator).color->a() * (*iterator).count;
		averageB += (*iterator).color->b() * (*iterator).count;
	}

	averageL = averageL / colorGroup.totalNumberOfColors;
	averageA = averageA / colorGroup.totalNumberOfColors;
	averageB = averageB / colorGroup.totalNumberOfColors;

	// convert the lab values to XYZ
	float x = xreference * fInverseOfX((1.0 / 116.0) * (averageL + 16.0) + (1.0 / 500.0) * averageA);
	float y = yreference * fInverseOfX((1.0 / 116.0) * (averageL + 16.0));
	float z = zreference * fInverseOfX((1.0 / 116.0) * (averageL + 16.0) - (1.0 / 200.0) * averageB);

	x = x / 100.0;
	y = y / 100.0;
	z = z / 100.0;

	// convert XYZ to RGB linear??
	float r = (3.2406 * x) + (-1.5372 * y) + (-0.4986 * z);
	float g = (-0.9689 * x) + (1.8758 * y) + (.0415 * z);
	float b = (0.0557 * x) + (-0.2040 * y) + (1.0570 * z);

	// now convert to sRGB?
	int red = round(reversePivot(r) * 255.0);
	int green = round(reversePivot(g) * 255.0);
	int blue = round(reversePivot(b) * 255.0);

	return new EMDcolor(red, green, blue);
}

bool sortRules(ColorGroup a, ColorGroup b)
{
	return a.totalNumberOfColors > b.totalNumberOfColors;
}

// consider using something other than Mat so that it doesn't depend on opencv??
std::vector<ColorCount> getDominantColors(unsigned char *imageArray, int minimumX, int maximumX, int minimumY, int maximumY)
{
	std::unordered_map<std::string, ColorCount> colorDictionary;

	// loop throught pixels
	// assume 3 for R, G, B
	// the order of X and Y might be incorrect
	for (int j = minimumY; j < maximumY; j++)
	{
		for (int i = minimumX; i < maximumX; i++)
		{
			// open cv uses BGR order instead of RGB order
			EMDcolor *color = new EMDcolor(imageArray[j * maximumX * 3 + i * 3 + 2], imageArray[j * maximumX * 3 + i * 3 + 1], imageArray[j * maximumX * 3 + i * 3]);
			
			std::string key = std::to_string(color->red()) + '_' + std::to_string(color->green()) + '_' + std::to_string(color->blue());


			// add to map
			if (colorDictionary.find(key) != colorDictionary.end()) {
				colorDictionary.at(key).count++;
			}
			else {
				ColorCount colorCount;
				colorCount.color = color;
				colorCount.count = 1;
				colorDictionary[key] = colorCount;
			}
		}

	}


	// put the colors into bins
	// ------------------------

	std::unordered_map<std::string, std::vector<ColorCount> > colorBins;

	for (std::unordered_map<std::string, ColorCount>::iterator iterator = colorDictionary.begin(); iterator != colorDictionary.end(); ++iterator)
	{
		ColorCount colorCount = iterator->second;

		float l = floor(colorCount.color->l() / (100.0 / numberLUnits));
		float a = floor((colorCount.color->a() + 128.0) / (255.0 / numberLUnits));
		float b = floor((colorCount.color->b() + 128.0) / (255.0 / numberBUnits));

		std::string key = std::to_string(l) + '_' + std::to_string(a) + '_' + std::to_string(b);

		if (colorBins.find(key) != colorBins.end()) {
			colorBins.at(key).push_back(colorCount);
		}
		else {
			std::vector<ColorCount> colorVector;
			colorVector.push_back(colorCount);
			colorBins[key] = colorVector;
		}
	}



	// sort the bins (some of this can be combined with the above step)
	// ----------------------------------------------------------------

	std::vector<ColorGroup> colorsUsed;

	for (std::unordered_map<std::string, std::vector<ColorCount> >::iterator iterator = colorBins.begin(); iterator != colorBins.end(); ++iterator)
    {
    	ColorGroup group;
    	group.groupName = iterator->first; // key
    	group.colors = iterator->second; // value
    	group.totalNumberOfColors = 0;
    	for (std::vector<ColorCount>::iterator iterator2 = group.colors.begin(); iterator2 != group.colors.end(); ++iterator2)
    	{
    		group.totalNumberOfColors += (*iterator2).count;
    	}

    	

    	colorsUsed.push_back(group);
	}



	// sort based on count
	std::sort(colorsUsed.begin(), colorsUsed.end(), sortRules);


	std::vector<ColorCount> dominantColors;


	// return the top 5 dominant colors
	for (int i = 0; i < fmin(5, colorsUsed.size()); i ++)
	{
		ColorCount colorCount;
		colorCount.count = colorsUsed[i].totalNumberOfColors;
		colorCount.color = average(colorsUsed[i]);
		dominantColors.push_back(colorCount);
	}

	return dominantColors;
}

std::vector<ColorCount> getColorsForVideoAtLocation(std::string fileLocation)
{
	std::vector<ColorCount> dominantColors;

	cv::VideoCapture video(fileLocation); // open the video file for reading

	if (!video.isOpened())  // if not success, exit program
    {
        std::cout << "Cannot open the video file" << std::endl;
        return dominantColors;
    }

    // calculate the number of frames here
    int count = 0;
    while (true) // change this to a for loop
    {
    	printf("Finding dominant color of frame %i\n", count);
    	cv::Mat frame;

    	bool success = video.read(frame); // read a new frame from the video

    	if (!success) // check if there was an error
    	{
    		std::cout << "Cannot read the frame from video file" << std::endl;
            break;
    	}

    	// if you only want to calculate the dominant colors along one edge or in the center, for example, change these arguments
    	int minimumX = 0;
		int maximumX = frame.cols;
		int minimumY = 0;
		int maximumY = frame.rows;
		
		std::vector<ColorCount> colors = getDominantColors(frame.data, minimumX, maximumX, minimumY, maximumY);

		// print colors

		// here we have the options to select certain colors over others if we so choose
		// for example, we could choose colors that weren't black or white if that's what we were going for
		dominantColors.push_back(colors[0]);

    	count += 1;
    }

    return dominantColors;
}

