#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <boost/filesystem.hpp>

const cv::Size MAX_DETECT_SIZE = cv::Size(100, 200);
const int MAX_MISS_FRAME = 10;
const double MIN_NEW_DETECT_INTERSECTION_RATE = 0.5;

// cv::Trackerのラッパー。現在地と
class MyTracker {
private:
	static int next_id;
	int id;
	int n_miss_frame = 0;
	cv::Rect2d rect;
	cv::Ptr<cv::Tracker> cv_tracker;
public:
	// フレーム画像と追跡対象(Rect)で初期化
	MyTracker(const cv::Mat& _frame, const cv::Rect2d& _rect) 
		: id(next_id++), rect(_rect) {
		cv_tracker = cv::Tracker::create("BOOSTING"); //  or "MIL"
		cv_tracker->init(_frame, _rect);
	}
	// 次フレームを入力にして、追跡対象の追跡(true)
	// MAX_MISS_FRAME以上検出が登録されていない場合は追跡失敗(false)
	bool update(const cv::Mat& _frame){
		n_miss_frame++;
		return cv_tracker->update(_frame, rect) && n_miss_frame < MAX_MISS_FRAME;
	}
	// 新しい検出(Rect)を登録。現在位置と近ければ受理してn_miss_frameをリセット(true)
	// そうでなければ(false)
	bool registerNewDetect(const cv::Rect2d& _new_detect){
		double intersection_rate = 1.0 * (_new_detect & rect).area() / (_new_detect | rect).area();
		bool is_registered = intersection_rate > MIN_NEW_DETECT_INTERSECTION_RATE;
		if (is_registered) n_miss_frame = 0;
		return is_registered;
	}
	// trackerの現在地を_imageに書き込む
	void draw(cv::Mat& _image) const{
		cv::rectangle(_image, rect, cv::Scalar(255, 0, 0), 2, 1);
		cv::putText(_image, cv::format("%03d", id), cv::Point(rect.x + 5, rect.y + 17), 
				cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,0,0), 1, CV_AA);
	}
};
int MyTracker::next_id = 0;


int main(int argc, char* argv[]){
	if(argc != 2){
		std::cout << "usage: " << argv[0] << " videodir" << std::endl;
		exit(1);
	}
	// フレーム画像列のパスを取得
	namespace fs = boost::filesystem;
	std::vector<std::string> frame_paths;
	for(auto it = fs::directory_iterator(argv[1]); it != fs::directory_iterator(); ++it){
		frame_paths.push_back(it->path().string());
	}
	// detector, trackerの宣言
	cv::HOGDescriptor detector;
	detector.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	std::vector<MyTracker> trackers;
	// 1フレームずつループ
	for (auto& frame_path : frame_paths){
		std::cout << "frame : " << frame_path << std::endl;
		cv::Mat frame = cv::imread(frame_path);
		// 人物検出
		std::vector<cv::Rect> detections;
		detector.detectMultiScale(frame, detections);
		// trackerの更新(追跡に失敗したら削除)
		for (auto t_it = trackers.begin(); t_it != trackers.end();){
			t_it = (t_it->update(frame)) ? std::next(t_it) : trackers.erase(t_it);
		}
		// 新しい検出があればそれを起点にtrackerを作成。(既存Trackerに重なる検出は無視)
		for(auto& d_rect : detections){
			if (d_rect.size().area() > MAX_DETECT_SIZE.area()) continue;
			bool is_exsisting = std::any_of(trackers.begin(), trackers.end(), 
					[&d_rect](MyTracker& t){return t.registerNewDetect(d_rect);});
			if(!is_exsisting) trackers.push_back(MyTracker(frame, d_rect));
		}
		// 人物追跡と人物検出の結果を表示
		cv::Mat image = frame.clone();
		for(auto& t : trackers) t.draw(image);
		for(auto& d_rect : detections) cv::rectangle(image, d_rect, cv::Scalar(0, 255, 0), 2, 1);
		cv::imshow("demo", image);
		cv::waitKey(1);
	}
	return 0;
}
