#pragma once
#ifndef lineDetector__H__
#define lineDetector__H__

#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/trace.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <atomic>
#include <vector>
#include <iostream>
#include <mutex>


class LineDetector {

public:

	// Constructor / Destructor
	LineDetector();
	~LineDetector();

	struct EdgeConfig {
		
		// Multithreading variables
		int id = 0;
		bool continueProcessing = true;
		bool newImgAvailable = false;
		bool linesDrawn = true;
		bool detectionComplete = true;

		// Window Properties
		const std::string blurThreshWindowName = "1. Blur & Thresholding";
		const std::string roiBoxWindowName = "1. Set Roi-Box";
		const std::string edgeWindowName = "2. Canny Edge Detection";
		const std::string morphWindowName = "3. Morphological Operations 1";
		const std::string morphWindowName2 = "4. Morphological Operations 2";
		const std::string houghWindowName = "5. Hough-Line Operations";
		const std::string houghCWindowName = "6. Hough-Circle Operations";
		const std::string roiBoxWindowName_car = "7. Set Roi-Box";
		const int screenHeight = 1080;
		const int screenWidth = 1920;
		float newCols;
		float newRows;

		// Lane Roi Properties
		int x1_roi = 400;  
		int y1_roi = 854;
		int recWidth = 850;
		int recHeight = 96;
		cv::Rect roi_Bbox = cv::Rect(x1_roi, y1_roi, recWidth, recHeight);
		cv::Mat roiImg;

		// Car Roi Properties
		int x1_roi_car = 0;
		int y1_roi_car = 629;
		int recWidth_car = 1489;
		int recHeight_car = 300;
		cv::Rect roi_Box_car = cv::Rect(x1_roi_car, y1_roi_car, recWidth_car, recHeight_car);
		cv::Mat roiImg_car;

		// Edge Detection Operations
		int lowThresh = 9;
		int highThresh = 23;
		int cannyKernel = 0;
		cv::Mat origImg;
		cv::Mat currImg;
		cv::Mat cannyImg;
		cv::Mat blurThreshImg;

		// Blur Parameters
		int gauss_ksize = 10;

		// Binary Thresholding
		int thresBin = 0;

		// Morphological Operations
		int morph_elem_shape = 0;
		int morphTransformType = 1;
		int	kernel_morph_size = 5;
		
		int morphTransformType_ = 1;
		int	kernel_morph_size_ = 1;
		cv::Mat morphImg;
		cv::Mat morphImg2;

		// Hough transforms
		cv::Mat houghImg;

		// --Lines
		int houghThreshold = 300;
		int apertureSize = 1;
		int minVotes = 62;
		int minVotesLim = 100;
		int minLineLengthLimit = 100;
		int minLineLength = 56;
		int maxLineGap = 6;
		int maxLineGapLimit = 20;
		int lineThickness = 4;
		std::vector<cv::Vec4i> edgeLines;
		cv::Point line1_pt1, line1_pt2, line2_pt1, line2_pt2;

		// -- Circles
		int centreThresh = 100;
		int circleThickness = 1;
		bool clrChange = true;
		int minRadius = 10;
		int maxRadius = 50;
		int minDistBtwCenters = 10;
		int dp = 2; // Inverse ratio of the accumulator resolution to the image resolution

		// Tracking & detection variables:
		std::vector<cv::Rect> trackedObjects;
		cv::Ptr<cv::Tracker> tracker;
		cv::dnn::Net net;

		bool trackerExists = false;
		bool imgProcessed = true;


	};

	struct ConfigParams {
		EdgeConfig edgeParams;
	};

	// member variables
	ConfigParams configParams;

	/* -------------------- Preprocessing --------------------*/
	
	//  -- ROI Setting
	void setLane_ROIBox();
	static void roiLaneCallback(int, void *userdata);
	void setCarDetectionROIBox();
	static void roiCarCallback(int, void *userdata);

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
	void houghCParameters();
	static void houghC_Callback(int, void *userdata);
	void houghLParametersP();
	static void houghLPCallback(int, void *userdata);

	/* -------------------- Processing --------------------*/
	void Pipeline();
	void processImg();
	void detectLanes();
	static void detectCars(LineDetector& ld, cv::dnn::Net& net);
	static void trackCars(LineDetector& ld, cv::dnn::Net& net);
	
	static void detectLanes(LineDetector& ld, bool draw = false);

	/* -------------------- Helper functions --------------------*/
	static void displayImg(cv::Mat Img, const std::string title, int screenWidth, int screenHeight, int imgNum);
	static void drawLines(EdgeConfig* edgeParams, cv::Mat& Img, std::vector<cv::Vec4i> lines, bool laneDetection = false);
	static void drawCircles(EdgeConfig * edgeParams, cv::Mat & img, const std::vector<cv::Vec3f>& circles);
	static void initialiseTracker(cv::Ptr<cv::Tracker>& tracker,std::string& trackerType); 
	//static int initialiseModelAndVideo(VideoCapture& vCap, cv::dnn::Net& net, string CLASSES[]);
	static void PrintFullPath(char * partialPath);
	static void func();



	static void processImg_thread(LineDetector& ld, std::atomic<bool>& stopThreads, cv::dnn::Net& net);
	static void processDetectTrack_thread(LineDetector& ld, std::atomic<bool>& stopThreads, std::vector<cv::Rect2d>& trackBoxVec, std::vector<int> &trackingStatus, std::vector<std::string> &statusLabels, std::vector<std::vector<cv::Vec4i>>& lines, std::mutex& mt_trackbox, std::mutex& lines_reserve);
	



};
#endif 