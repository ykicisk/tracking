#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <boost/filesystem.hpp>

const double SAME_PERSON_RECT_INSTERSECTION_RATE = 0.5;
const cv::Size DETECT_MAX_SIZE = cv::Size(72, 200);
const int TRACKER_MAX_NO_DETECT_FRAME = 15;

struct MyTracker {
	int id = 0;
	cv::Rect rect;
	cv::Ptr<cv::Tracker> cv_tracker;
	int no_detect_frame = 0;
	MyTracker(){
		cv_tracker = cv::Tracker::create("BOOSTING");
	}
};

int main(int argc, char* argv[]){
	if(argc != 2){
		std::cout << "usage: " << argv[0] << " videodir" << std::endl;
		exit(1);
	}
	// video読み込み
	namespace fs = boost::filesystem;
	std::vector<std::string> frame_paths;
	for(auto it = fs::directory_iterator(argv[1]); it != fs::directory_iterator(); ++it){
		frame_paths.push_back(it->path().string());
	}
	// 準備
	int numbering = 0;
	cv::Mat frame;
	cv::HOGDescriptor detector;
	detector.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	std::vector<MyTracker> trackers;
	for (auto& frame_path : frame_paths){
		cv::Mat frame = cv::imread(frame_path);
		// 人物検出
		std::cout << "frame : " << frame_path << ":" << frame.size() << std::endl;
		std::cout << "detect" << std::endl;
		std::vector<cv::Rect> detections;
		detector.detectMultiScale(frame, detections);
		// 人物追跡(更新) (trackingに失敗したら削除)
		std::cout << "update" << std::endl;
		for (auto t_it = trackers.begin(); t_it != trackers.end(); ){
			cv::Rect_<double> next_rect;
			bool tracker_survival_flag = t_it->cv_tracker->update(frame, next_rect) 
				&& t_it->no_detect_frame < TRACKER_MAX_NO_DETECT_FRAME;
			std::cout << "rect : " << next_rect << std::endl;
			if(tracker_survival_flag){
				t_it->no_detect_frame++;
				t_it->rect = next_rect;
				++t_it;
			}else{
				t_it = trackers.erase(t_it);
			}
		}
		// 新しいdetectionがあればtrackerを作成
		std::cout << "create new tracker" << std::endl;
		for(auto& d_rect : detections){
			if (d_rect.size().area() > DETECT_MAX_SIZE.area()) continue;
			bool new_detection_flag = true;
			for (auto& t : trackers){
				double intersection_rate = 1.0 * (d_rect & t.rect).area() / (d_rect | t.rect).area();
				if (intersection_rate > SAME_PERSON_RECT_INSTERSECTION_RATE){
					new_detection_flag = false;
					t.no_detect_frame = 0;
				}
			}
			if(new_detection_flag){
				MyTracker new_tracker;
				new_tracker.rect = d_rect;
				new_tracker.id = numbering;
				new_tracker.cv_tracker->init(frame, d_rect);
				trackers.push_back(new_tracker);
				numbering++;
			}
		}
		// 表示
		std::cout << "drawing" << std::endl;
		cv::Mat image = frame.clone();
		for(auto& d_rect : detections){
			cv::rectangle(image, d_rect, cv::Scalar(0, 255, 0), 2, 1);
		}
		for(auto& t : trackers){
			cv::rectangle(image, t.rect, cv::Scalar(255, 0, 0), 2, 1);
			cv::putText(image, cv::format("%03d", t.id), 
					t.rect.tl(), cv::FONT_HERSHEY_SIMPLEX, 1.0, 
					cv::Scalar(255,0,0), 2, CV_AA);
		}
		cv::imshow("demo", image);
		cv::waitKey(1);
	}
	return 0;
}
