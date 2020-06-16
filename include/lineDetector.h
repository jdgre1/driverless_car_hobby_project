#ifndef lineDetector__H__
#define lineDetector__H__

#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video.hpp>
#include <vector>
#include <iostream>
//#include <string.h>

class LineDetector {

public:

	// Constructor / Destructor
	LineDetector();
	~LineDetector();

	struct EdgeConfig {

		// Window Properties
		const std::string blurThreshWindowName = "1. Blur & Thresholding";
		const std::string edgeWindowName = "1. Canny Edge Detection";
		const std::string morphWindowName = "2. Morphological Operations 1";
		const std::string morphWindowName2 = "3. Morphological Operations 2";
		const std::string houghWindowName = "4. Hough Operations";
		const int screenHeight = 1080;
		const int screenWidth = 1650;
		float newCols;
		float newRows;

		// Edge Detection Operations
		int lowThresh = 42;
		int highThresh = 106;
		int cannyKernel = 0;
		cv::Mat origImg;
		cv::Mat cannyImg;
		cv::Mat blurThreshImg;

		// Blur Parameters
		int gauss_ksize = 10;

		// Binary Thresholding
		int thresBin = 0;

		// Morphological Operations
		int morph_elem_shape = 0;
		int morphTransformType = 1;
		int	kernel_morph_size = 2;
		
		int morphTransformType_ = 0;
		int	kernel_morph_size_ = 1;
		cv::Mat morphImg;
		cv::Mat morphImg2;

		// Hough transforms
		// --Lines
		int houghThreshold = 300;
		int apertureSize = 1;
		int minVotes = 62;
		int minVotesLim = 100;
		int minLineLengthLimit = 100;
		int minLineLength = 56;
		int maxLineGap = 6;
		int maxLineGapLimit = 20;
		int lineThickness = 2;
		std::vector<cv::Vec4i> edgeLines;
		cv::Mat houghImg;
		// -- Circles
		int centreThresh = 100;
		int circleThickness = 1;
		bool clrChange = true;
		int minRadius = 2;
		int maxRadius = 50;
		int minDistBtwCenters = 10;
		int dp = 2; // Inverse ratio of the accumulator resolution to the image resolution

	};

	struct ConfigParams {
		EdgeConfig edgeParams;
	};

	// member variables
	ConfigParams configParams;

	/* -------------------- Preprocessing --------------------*/
	
	//  -- Edge detection
	void edgeParametersP();
	static void edgeDetectCallback(int, void *userdata);
	void blurThreshParametersP();
	static void blurThreshCallback(int, void *userdata);

	// -- Morphological parameters
	void morphParametersP();
	static void morphCallback(int, void *userdata);
	void morphParametersP2();
	static void morphCallback2(int, void *userdata);

	// -- Hough transforms
	void houghParametersP();
	static void houghPCallback(int, void *userdata);

	// -- Helper functions
	static void displayImg(cv::Mat Img, const std::string title, int screenWidth, int screenHeight, int imgNum);
	static void drawLines(EdgeConfig* edgeParams, cv::Mat& Img, std::vector<cv::Vec4i> lines);
	static void drawCircles(EdgeConfig * edgeParams, cv::Mat & img, const std::vector<cv::Vec3f>& circles);
	static void PrintFullPath(char * partialPath);
	static void func();
};
#endif 