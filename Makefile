PYTHON_LIBS_MAC = -L/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config -ldl -framework CoreFoundation -lpython2.7
PYTHON_FLAGS_MAC = $(shell python-config --cflags)

# need to clean this up!!!

default:
	swig -c++ -python ColorCalculations.i
	g++ -c $(PYTHON_FLAGS_MAC) colorCalculations.cpp ColorCalculations_wrap.cxx
	g++ -shared $(PYTHON_LIBS_MAC) -lopencv_highgui -lopencv_core colorCalculations.o ColorCalculations_wrap.o -o _ColorCalculations.so

clean:
	rm *.so *.o *_wrap.cxx ColorCalculations.pyc ColorCalculations.py