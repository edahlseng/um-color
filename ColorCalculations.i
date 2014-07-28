/* ColorCalculations.i */
%module ColorCalculations
%include typemaps.i
%include std_vector.i 	// for passing in python lists
%{
	#include <vector>

	class EMDcolor
	{
		public:
			const char* toHexString();
	};

	struct ColorCount {
		EMDcolor	*color;
		int 		count;
	};

	std::vector<ColorCount> getColorsForVideoAtLocation(std::string fileLocation);
%}

namespace std {
	%template(Array)  vector < int >;
}  

%inline
%{
	PyObject* colorsForVideoAtLocation(char *location)
	{
		std::vector<struct ColorCount> colors = getColorsForVideoAtLocation(std::string(location));
		
		PyObject *listOfColors = PyList_New(colors.size());

		if(colors.size() > 0)
		{	
            int i;
			for (i = 0; i < colors.size(); i++)
			{
				PyList_SetItem(listOfColors, i, Py_BuildValue("{s:s,s:i}","color",colors[i].color->toHexString(),"count",colors[i].count));
			}	
		}

		return listOfColors;
	}
%}
