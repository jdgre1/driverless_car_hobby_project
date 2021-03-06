#include <dashboardTracker.h>
using namespace cv;
using namespace std;

DashboardTracker::DashboardTracker(const int& iD, const LineDetector& lD, const TrafficDetector& tD, const CarTracker& cT): m_id(iD), m_ld(lD), m_td(tD), m_ct(cT){
	m_ld.setId(iD);
	m_td.setId(iD);
	m_ct.setId(iD);
}

DashboardTracker::DashboardTracker()
{
}

DashboardTracker::~DashboardTracker()
{
}



int DashboardTracker::getId() const
{
	return m_id;
}

void DashboardTracker::setLd(const LineDetector& lD)
{
	m_ld = lD;
}

void DashboardTracker::setCt(const CarTracker& cT)
{
	m_ct = cT;
}

void DashboardTracker::setTd(const TrafficDetector& tD)
{
	m_td = tD;
}

void DashboardTracker::setCurrImg(const cv::Mat& curr_Img)
{
	m_currImg = curr_Img;// .clone();
	m_ld.setCurrImg(curr_Img);
	m_td.setCurrImg(curr_Img);
	m_ct.setCurrImg(curr_Img);
}

cv::Mat DashboardTracker::getCurrImg() const
{
	return m_currImg;
}

LineDetector DashboardTracker::getLd() const
{
	return m_ld;
}

TrafficDetector DashboardTracker::getTd() const
{
	return m_td;
}

CarTracker DashboardTracker::getCt() const
{
	return m_ct;
}

void DashboardTracker::setImgProcessed(const bool& status)
{
	m_imgProcessed = status;
}

bool DashboardTracker::getImgProcessed() const
{
	return m_imgProcessed;
}

std::thread DashboardTracker::dashboardThread(std::vector<bool>& imgAvailable, std::atomic<bool>& stopThreads, std::vector<cv::Rect2d>& trackBoxVec, std::vector<int> &trackingStatus, std::vector<std::vector<cv::Vec4i>>& lines, std::mutex& imgAvailableGuard, std::mutex& trackingStatusGuard, std::mutex& mt_trackbox, std::mutex& lines_reserve) {

	return std::thread(&DashboardTracker::runThread, this, std::ref(imgAvailable), std::ref(stopThreads), std::ref(trackBoxVec), std::ref(trackingStatus), std::ref(lines), std::ref(imgAvailableGuard), std::ref(trackingStatusGuard), std::ref(mt_trackbox), std::ref(lines_reserve));
}

// old runThread method (had worked with old runThreadsOnHeap method)
#if 0
void DashboardTracker::runThread__(std::vector<bool>& imgAvail, std::atomic<bool>& stopThreads, std::vector<cv::Rect2d>& trackBoxVec, std::vector<int>& trackingStatus, std::vector<std::vector<cv::Vec4i>>& lines, std::mutex & imgAvailableGuard, std::mutex & trackingStatusGuard, std::mutex & mt_trackbox, std::mutex & lines_reserve)
{
	// Reusable variables
	int trackStatus;
	Rect2d trackBox;
	Rect2d trackBox_empty;
	Rect2d roi_tracker_box = m_td.getRoiBox();
	bool updateSuccess;
	bool imgAvailable;
	m_td.setId(this->m_id);
	m_ld.setId(this->m_id);
	m_ct.setId(this->m_id);
	std::string CLASSES[21] = { "background", "aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable",
									"dog", "horse", "motorbike", "person", "pottedplant", "sheep","sofa", "train", "tvmonitor" };
	int countsSinceLastSearch = 20;
	int failureCounter = 0;


	while (true) {

		if (stopThreads) {
			break;
			return;
		}
		imgAvailable = false;
		{
			const std::lock_guard<mutex> lock(imgAvailableGuard);
			imgAvailable = imgAvail[m_id];
		}

		if (imgAvailable) {
			Mat img;

			{   // Writing
				const std::lock_guard<mutex> lock(imgAvailableGuard);
				m_imgProcessed = false;
				imgAvail[m_id] = false;
				img = m_currImg.clone();
			}

			// Step 1: Lane Detection
			auto start = std::chrono::high_resolution_clock::now();
			m_ld.detectObject();

			{
				// Writing
				const std::lock_guard<mutex> lock(lines_reserve);
				lines[m_id] = m_ld.getHoughParams().lines;
				trackStatus = trackingStatus[m_id];
				//auto finish = std::chrono::high_resolution_clock::now();
				//std::chrono::duration<double> elapsed = finish - start;
				//std::cout << "Line detection took : " << elapsed.count() << " s\n";
			}

			// Optional Step 2: If no tracker instantiated, keep detecting
			if ((trackStatus == 0) & (countsSinceLastSearch >= 20))

			{
				countsSinceLastSearch = 0;
				auto start = std::chrono::high_resolution_clock::now();
				m_td.detectObject(trackBoxVec, mt_trackbox);
				auto finish = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> elapsed = finish - start;
				{
					// Writing to 'cout'
					const std::lock_guard<mutex> lock(imgAvailableGuard);
					std::cout << "Traffic detection took : " << elapsed.count() << " s\n";
				}

				trackStatus = m_td.getTrackStatus();
				if (trackStatus == 1) {
					trackBox = m_td.getTrackbox();
					m_ct.initTracker(img(roi_tracker_box), trackBox);

					{
						// Writing
						const std::lock_guard<mutex> lock(trackingStatusGuard);
						trackingStatus[m_id] = 1;
						trackStatus = 1;
					}
				}
			}

			else if ((trackStatus == 1) | (trackStatus == 2)) { // DO NOT LET IT SET A NEW TRACKER!! IT UPDATES ON A DIFFERENT AREA AFTER TRACKER LOST??
				{
					// Writing
					const std::lock_guard<mutex> lock(trackingStatusGuard);
					auto start = std::chrono::high_resolution_clock::now();
					updateSuccess = m_ct.updateTracker(img(roi_tracker_box), trackBox);
					trackBoxVec[m_id] = trackBox;
					auto finish = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double> elapsed = finish - start;
					std::cout << "Update took : " << elapsed.count() << " s\n";
					/*
					bool trackBoxOk = true;
					for (int m = 0; m < trackBoxVec.size(); m++) {
						Rect2d trackBox_m = trackBoxVec[m];
						Point center_of_rect_m = (trackBox_m.br() + trackBox_m.tl())*0.5;
						if (center_of_rect_m.x != 0 & center_of_rect_m.y != 0) {
							for (int n = 0; n < trackBoxVec.size(); n++) {
								if (m != n) {
									Rect2d trackBox_n = trackBoxVec[n];
									Point center_of_rect_n = (trackBox_n.br() + trackBox_n.tl())*0.5;
									if (center_of_rect_n.x != 0 & center_of_rect_n.y != 0) {
										Point diff = center_of_rect_m - center_of_rect_n;
										float dist_Bboxes = cv::sqrt(diff.x*diff.x + diff.y*diff.y);

										// Threshold for another tracked object should be the distance between current bounding box centers
										if (dist_Bboxes < 100) {
											updateSuccess = false;
											trackBoxVec[id] = trackBox_empty;
											break;

										}
									}
								}
							}
						}
					}
					*/
				}


				if (updateSuccess)
				{
					failureCounter = 0;
					if (trackStatus == 2) {
						{   // Writing
							const std::lock_guard<mutex> lock(trackingStatusGuard);
							trackingStatus[m_id] = 1;
							trackStatus = 1;
						}
					}
				}

				else {
					failureCounter++;
					if (failureCounter > 5) {
						//putText(frame, "Tracking abandoned: too many frames without successful tracking", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
						{   // Writing
							const std::lock_guard<mutex> lock(mt_trackbox);
							trackStatus = 0;
							trackingStatus[m_id] = trackStatus;
							trackBoxVec[m_id] = trackBox_empty;
						}
					}

					else {
						{   // Writing
							const std::lock_guard<mutex> lock(trackingStatusGuard);
							trackingStatus[m_id] = 2;
						}
					}
				}
			}


			else {
				countsSinceLastSearch++;
			}
			m_imgProcessed = true;
		}
	}
	this_thread::sleep_for(chrono::milliseconds(10));
}
#endif
void DashboardTracker::runThread(std::vector<bool>& imgAvail, std::atomic<bool>& stopThreads, std::vector<cv::Rect2d>& trackBoxVec, std::vector<int>& trackingStatus, std::vector<std::vector<cv::Vec4i>>& lines, std::mutex& imgAvailableGuard, std::mutex& trackingStatusGuard, std::mutex& mt_trackbox, std::mutex& lines_reserve)
{
	// Reusable variables
	m_td.setId(this->m_id);
	m_ld.setId(this->m_id);
	m_ct.setId(this->m_id);
	int trackStatus;
	Rect2d trackBox;
	Rect2d trackBox_empty;
	bool updateSuccess;
	bool imgAvailable;
	Rect2d roi_tracker_box = m_td.getRoiBox();
	std::string CLASSES[21] = { "background", "aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable",
									"dog", "horse", "motorbike", "person", "pottedplant", "sheep","sofa", "train", "tvmonitor" };
	int countsSinceLastSearch = 20;
	int failureCounter = 0;


	while (true) {

		if (stopThreads) {
			break;
			return;
		}
		imgAvailable = false;

		// Reading
		imgAvailable = imgAvail[m_id];

		if (imgAvailable) {
			Mat img;

			{   // Writing
				const std::lock_guard<mutex> lock(imgAvailableGuard);
				m_imgProcessed = false;
				imgAvail[m_id] = false;
				img = m_currImg.clone();
			}

			// Step 1: Lane Detection
			auto start = std::chrono::high_resolution_clock::now();
			m_ld.detectObject();

			{	// Writing
				const std::lock_guard<mutex> lock(lines_reserve);
				lines[m_id] = m_ld.getHoughParams().lines;
				trackStatus = trackingStatus[m_id];
				//auto finish = std::chrono::high_resolution_clock::now();
				//std::chrono::duration<double> elapsed = finish - start;
				//std::cout << "Line detection took : " << elapsed.count() << " s\n";
			}

			// Optional Step 2: If no tracker instantiated, keep detecting

			if (trackStatus == 0 & countsSinceLastSearch >= 20)
			{
				countsSinceLastSearch = 0;
				// Reading
				m_td.setTrackStatus(0);
				m_td.detectObject(trackBoxVec, mt_trackbox); // OR when Cuda works with dnn::Net -----> td_.detectObject()							 
				trackStatus = m_td.getTrackStatus();


				if (trackStatus == 1) {

					// Reading
					trackBox = m_td.getTrackbox();
					countsSinceLastSearch = 20;
					m_ct.declareTracker(m_ct.getTrackerType());
					m_ct.initTracker(img(roi_tracker_box), trackBox);
					{
						// Writing
						const std::lock_guard<mutex> lock(trackingStatusGuard);
						trackingStatus[m_id] = 1;
					}
				}
			}

			else if ((trackStatus == 1) | (trackStatus == 2)) { // DO NOT LET IT SET A NEW TRACKER!! IT UPDATES ON A DIFFERENT AREA AFTER TRACKER LOST??

			
				{
					// Writing
					const std::lock_guard<mutex> lock(mt_trackbox);
					updateSuccess = m_ct.updateTracker(img(roi_tracker_box), trackBoxVec[m_id]);
					if (!updateSuccess) {
						cout << "Tracker Update Fail!" << endl;
					}
				}


				if (updateSuccess)
				{
					failureCounter = 0;
					if (trackStatus == 2) {
						{
							// Writing
							const std::lock_guard<mutex> lock(trackingStatusGuard);
							trackingStatus[m_id] = 1;
						}
						trackStatus = 1;
					}
				}

				else {
					failureCounter++;
					if (failureCounter > 5) {
						//putText(frame, "Tracking abandoned: too many frames without successful tracking", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
						{
							const std::lock_guard<mutex> lock(mt_trackbox);
							trackStatus = 0;
							trackingStatus[m_id] = trackStatus;
							trackBoxVec[m_id] = trackBox_empty;
						}
					}

					else {
						{
							const std::lock_guard<mutex> lock(trackingStatusGuard);
							trackingStatus[m_id] = 2;
							trackStatus == 2;
						}
					}
				}
			}

			else {
				countsSinceLastSearch++;
				trackStatus == 0;
			}
			m_imgProcessed = true;
		}
		this_thread::sleep_for(chrono::milliseconds(10));
	}
}

void DashboardTracker::staticMethodThread(DashboardTracker& dt, vector<bool>& imgAvail, atomic<bool>& stopThreads, vector<Rect2d>& trackBoxVec, vector<int> &trackingStatus, vector<vector<Vec4i>>& lines, mutex& imgAvailableGuard, mutex& trackingStatusGuard, mutex& mt_trackbox, mutex& lines_reserve) {

	// Reusable variables
	int id = dt.getId();
	int trackStatus;
	Rect2d trackBox;
	Rect2d trackBox_empty;
	bool updateSuccess;
	bool imgAvailable;
	LineDetector ld_;
	CarTracker ct_;
	TrafficDetector td_ = dt.getTd();
	Rect2d roi_tracker_box = td_.getRoiBox();
	//const std::string CLASSES = *aD.getTd().getClasses();
	std::string CLASSES[21] = { "background", "aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable",
									"dog", "horse", "motorbike", "person", "pottedplant", "sheep","sofa", "train", "tvmonitor" };
	int countsSinceLastSearch = 20;
	int failureCounter = 0;


	while (true) {

		if (stopThreads) {
			break;
			return;
		}
		imgAvailable = false;

		// Reading
		imgAvailable = imgAvail[id];

		if (imgAvailable) {
			Mat img;

			{	// Writing
				const std::lock_guard<mutex> lock(imgAvailableGuard);
				dt.setImgProcessed(false);
				imgAvail[id] = false;
				img = dt.getCurrImg().clone();
			}

			// Step 1: Lane Detection
			auto start = std::chrono::high_resolution_clock::now();
			ld_ = dt.getLd();
			ld_.detectObject();

			{	// Writing
				const std::lock_guard<mutex> lock(lines_reserve);
				lines[id] = ld_.getHoughParams().lines;
				trackStatus = trackingStatus[id];
				//auto finish = std::chrono::high_resolution_clock::now();
				//std::chrono::duration<double> elapsed = finish - start;
				//std::cout << "Line detection took : " << elapsed.count() << " s\n";
			}

			// Optional Step 2: If no tracker instantiated, keep detecting

			if (trackStatus == 0 & countsSinceLastSearch >= 20)
			{	
				countsSinceLastSearch = 0;
				// Reading
				td_ = dt.getTd();
				td_.setTrackStatus(0);
				td_.detectObject(trackBoxVec, mt_trackbox); // OR when Cuda works with dnn::Net -----> td_.detectObject()
				trackStatus = td_.getTrackStatus();
				{
					// Writing
					const std::lock_guard<mutex> lock(trackingStatusGuard);
					dt.setTd(td_);
				}

				if (trackStatus == 1) {

					// Reading
					ct_ = dt.getCt();
					trackBox = td_.getTrackbox();
					countsSinceLastSearch = 20;
					ct_.declareTracker(ct_.getTrackerType());
					ct_.initTracker(img(roi_tracker_box), trackBox);
					{
						// Writing
						const std::lock_guard<mutex> lock(trackingStatusGuard);
						dt.setCt(ct_);
						trackingStatus[id] = 1;
					}
				}
			}

			else if ((trackStatus == 1) | (trackStatus == 2)) { // DO NOT LET IT SET A NEW TRACKER!! IT UPDATES ON A DIFFERENT AREA AFTER TRACKER LOST??

				// Reading
				ct_ = dt.getCt();

				{	
					// Writing
					const std::lock_guard<mutex> lock(mt_trackbox);
					updateSuccess = ct_.updateTracker(img(roi_tracker_box), trackBoxVec[id]);
					if (!updateSuccess) {
						cout << "Tracker Update Fail!" << endl;
					}
					dt.setCt(ct_);
				}

				
				if (updateSuccess)
				{
					failureCounter = 0;
					if (trackStatus == 2) {
						{	
							// Writing
							const std::lock_guard<mutex> lock(trackingStatusGuard);
							trackingStatus[id] = 1;
						}
						trackStatus = 1;
					}	
				}

				else {
					failureCounter++;
					if (failureCounter > 5) {
						//putText(frame, "Tracking abandoned: too many frames without successful tracking", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
						{
							const std::lock_guard<mutex> lock(mt_trackbox);
							trackStatus = 0;
							trackingStatus[id] = trackStatus;
							trackBoxVec[id] = trackBox_empty;
						}
					}

					else {
						{
							const std::lock_guard<mutex> lock(trackingStatusGuard);
							trackingStatus[id] = 2;
							trackStatus == 2;
						}
					}
				}
			}

			else {
				countsSinceLastSearch++;
				trackStatus == 0;
			}
			dt.setImgProcessed(true);
		}
		this_thread::sleep_for(chrono::milliseconds(10));
	}

}


/*
void DashboardTracker::dashboardTrackersThread(DashboardTracker& aD, vector<bool>& imgAvail, atomic<bool>& stopThreads, vector<Rect2d>& trackBoxVec, vector<int> &trackingStatus, vector<vector<Vec4i>>& lines, mutex& imgAvailableGuard, mutex& trackingStatusGuard, mutex& mt_trackbox, mutex& lines_reserve) {

	// Reusable variables
	int id = aD.getId();
	int trackStatus;
	Rect2d trackBox;
	Rect2d trackBox_empty;
	bool updateSuccess;
	bool imgAvailable;
	LineDetector ld_;
	CarTracker ct_;
	TrafficDetector td_ = aD.getTd();
	Rect2d roi_tracker_box = td_.getRoiBox();
	//const std::string CLASSES = *aD.getTd().getClasses();
	std::string CLASSES[21] = { "background", "aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable",
									"dog", "horse", "motorbike", "person", "pottedplant", "sheep","sofa", "train", "tvmonitor" };
	int countsSinceLastSearch = 20;
	int failureCounter = 0;


	while (true) {

		if (stopThreads) {
			break;
			return;
		}
		imgAvailable = false;
		{
			const std::lock_guard<mutex> lock(imgAvailableGuard);
			imgAvailable = imgAvail[id];
		}

		if (imgAvailable) {
			Mat img;
			{
				const std::lock_guard<mutex> lock(imgAvailableGuard);
				aD.setImgProcessed(false);
				imgAvail[id] = false;
				img = aD.getCurrImg().clone();
			}

			// Step 1: Lane Detection
			ld_ = aD.getLd();
			ld_.detectObject();

			{
				const std::lock_guard<mutex> lock(lines_reserve);
				lines[id] = ld_.getHoughParams().lines;
				trackStatus = trackingStatus[id];
			}

			// Optional Step 2: If no tracker instantiated, keep detecting

			if (trackStatus == 0 & countsSinceLastSearch >= 20)
			{
				countsSinceLastSearch = 0;
				td_ = aD.getTd();
				td_.detectObject(trackBoxVec, mt_trackbox);
				trackStatus = td_.getTrackStatus();
				aD.setTd(td_);
				if (trackStatus == 1) {
					ct_ = aD.getCt();
					trackBox = td_.getTrackbox();
					ct_.initTracker(img(roi_tracker_box), trackBox);
					aD.setCt(ct_);
					{
						const std::lock_guard<mutex> lock(trackingStatusGuard);
						trackingStatus[id] = 1;
						trackStatus = 1;
					}
				}
			}

			else if ((trackStatus == 1) | (trackStatus == 2)){ // DO NOT LET IT SET A NEW TRACKER!! IT UPDATES ON A DIFFERENT AREA AFTER TRACKER LOST??
				ct_ = aD.getCt();

				{
					const std::lock_guard<mutex> lock(mt_trackbox);
					updateSuccess = ct_.updateTracker(img(roi_tracker_box), trackBoxVec[id]);
				}
				aD.setCt(ct_);
				if (updateSuccess)
				{
					failureCounter = 0;
					if (trackStatus == 2) {
						{
							const std::lock_guard<mutex> lock(trackingStatusGuard);
							trackingStatus[id] = 1;
						}
					}
				}

				else {
					failureCounter++;
					if (failureCounter > 5) {
						//putText(frame, "Tracking abandoned: too many frames without successful tracking", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
						{
							const std::lock_guard<mutex> lock(mt_trackbox);
							trackStatus = 0;
							trackingStatus[id] = trackStatus;
							trackBoxVec[id] = trackBox_empty;
						}
					}
					else {
						{
							const std::lock_guard<mutex> lock(trackingStatusGuard);
							trackingStatus[id] = 2;
						}
					}
				}
			}

			else {
				countsSinceLastSearch++;
			}
			aD.setImgProcessed(true);
		}
		this_thread::sleep_for(chrono::milliseconds(10));
	}

}
*/

void DashboardTracker::setId(const int& iD)
{
	m_id = iD;
	m_ld.setId(m_id);
	m_td.setId(m_id);
	m_ct.setId(m_id);
}



