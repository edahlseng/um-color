import sys 					# for running form the command line
import ColorCalculations 	# for determining the dominant color

def colorsForUrl(url):
	#vidcap = cv2.VideoCapture(url)

	colors = ColorCalculations.colorsForVideoAtLocation(url)
	return colors



if __name__=='__main__':
	if len(sys.argv)!=2:
		print 'Usage: python videoColors.py [pathToVideo]'
	else:
		print colorsForUrl(sys.argv[1])
