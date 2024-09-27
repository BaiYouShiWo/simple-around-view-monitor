#include <mutex>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <string>
#include <thread>
#include <unistd.h>
#include <iostream>
#include <condition_variable>
#include <experimental/filesystem>

#include <opencv2/opencv.hpp> 
#include <opencv2/core/utils/logger.hpp>

#include "data_dir.h"
#include "streamer.h"
#include "parameter.h"
#include "debug_utils.h"
#include "mask_generator.h"
#include "circular_buffer.h"
#include "camera_generator.h"

namespace fs = std::experimental::filesystem;
using std::vector;
using cv::Mat;


struct final_data {
    vector<Mat> maps;
    Mat mask;
};

void consumer(CircularBuffer& buff_front,
              CircularBuffer& buff_back, 
              CircularBuffer& buff_left, 
              CircularBuffer& buff_right);
void producer(std::string name, final_data data, CircularBuffer& buff,int* index_list);
Mat remap_and_mask(Mat &img, final_data &data);
Mat only_remap(Mat &img, final_data &data);
Mat only_mask(Mat &img, final_data &data);

std::mutex mtx;
std::condition_variable condi;
vector<std::queue<Mat>> vector_image_queue(4,std::queue<Mat>());

void producer(std::string name, final_data data, CircularBuffer& buff,int* index_list) {
    int* ptr = nullptr;
    ptr = index_list;

    int rorate_angle = -1;
    
    auto vw = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');//The encode form -> MJPG
    int camera_index = 0;

    Mat part_image = Mat::zeros(cv::Size(BEV_WIDTH,BEV_HEIGHT),CV_8UC3);
    cv::Point position_masked_image; 
    cv::Rect roi_masked_image;  
    if(ptr == nullptr){
        camera_index = -1;
    }else{
        if (name == "front"){
            camera_index = index_list[0];
        }else if (name == "back"){
            camera_index = index_list[1];
        }else if (name == "left"){
            camera_index = index_list[2];
        }else if (name == "right"){
            camera_index = index_list[3];
        }
    }
    rub:
    cv::VideoCapture cap = cv::VideoCapture(camera_index,cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FOURCC, vw);
    cap.set(cv::CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
    cap.set(cv::CAP_PROP_FPS, 30);
    if (!cap.isOpened()) {
        std::cerr << "ERROR: " << name << " camera launch failed!" << std::endl;
        goto rub;
	}else{
        std::cout << name << " launch sucessfully" << "-camera_id="<<camera_index<<std::endl;
    }
    Mat image,undistort_image,masked_image;

    Mat rotate;
    while (cap.isOpened()) {
        cap.read(image);
        if (image.empty()) {
            std::cerr << "ERROR:"<< name <<" 帧为空!" << std::endl;
            continue;
        }
        undistort_image = only_remap(image, data);
        masked_image = only_mask(undistort_image,data);
        if(name == "front"){
            rotate = masked_image;
            position_masked_image = cv::Point(0,0);
            roi_masked_image = cv::Rect(position_masked_image,rotate.size());
            rotate.copyTo(part_image(roi_masked_image));
        }else if(name == "back"){
            cv::rotate(masked_image, rotate, cv::ROTATE_180);
            position_masked_image = cv::Point(0,640);
            roi_masked_image = cv::Rect(position_masked_image,rotate.size());
            rotate.copyTo(part_image(roi_masked_image));
        }else if(name == "left"){
            cv::rotate(masked_image, rotate, cv::ROTATE_90_COUNTERCLOCKWISE);
            position_masked_image = cv::Point(0,0);
            roi_masked_image = cv::Rect(position_masked_image,rotate.size());
            rotate.copyTo(part_image(roi_masked_image));
        }else if(name == "right"){
            cv::rotate(masked_image, rotate, cv::ROTATE_90_CLOCKWISE);
            position_masked_image = cv::Point(480,0);
            roi_masked_image = cv::Rect(position_masked_image,rotate.size());
            rotate.copyTo(part_image(roi_masked_image));
        }
        // cv::imshow(name,part_image);
        // cv::waitKey(1);
        buff.push(part_image);
    }
    cap.release();
    cv::destroyAllWindows();
}

void consumer(CircularBuffer& buff_front,
              CircularBuffer& buff_back, 
              CircularBuffer& buff_left, 
              CircularBuffer& buff_right){
    Mat front_img, back_img, left_img, right_img, surround;

    // double width = 1280;
    // double height = 960;
    // double fps = 30;
    // if( !width || !height || !fps ){
    //     std::cerr<<"camera launch failed"<<std::endl;
    //     exit(0);
    // }

    // Streamer streamer;
    // if(!streamer.streamer_init(width, height, fps)){
    //     std::cerr<<"streamer init failed"<<std::endl;
    //     exit(0);
    // }

    fs::path cur_path = get_current_directory();
    fs::path parent_path = cur_path.parent_path();     
    Mat car = cv::imread(parent_path/"data"/"car.jpg");
    car = padding(car, BEV_WIDTH, BEV_HEIGHT);//create a canvas that the car was in the mid
    cv::namedWindow("add_pic",cv::WINDOW_NORMAL|cv::WINDOW_KEEPRATIO);
    while (true) {

        front_img = buff_front.pop();
        back_img = buff_back.pop();
        left_img = buff_left.pop();
        right_img = buff_right.pop();


        // Mat mid_total = total_picture(front_img, back_img, left_img, right_img);
        // cv::imshow("Vertical Concatenation", mid_total);
        // streamer.matToRTSP(mid_total);
        

        surround = car.clone();
        add(surround, front_img, surround);
        add(surround, back_img, surround);
        add(surround, left_img, surround);
        add(surround, right_img, surround);
        cv::imshow("add_pic", surround);


        cv::waitKey(1);
    }
    cv::destroyAllWindows();
}


Mat only_remap(Mat &img, final_data &data) {//this func will generate one forth of the total image
    Mat image_undistort;
    remap(img, image_undistort, data.maps[0], data.maps[1], cv::INTER_LINEAR);
    return  image_undistort;
}

Mat only_mask(Mat &img, final_data &data){
    Mat image_masked,image_display;
    multiply(img, data.mask, image_masked, 1, CV_32F);

    image_masked.convertTo(image_display, CV_8UC3); 
    return  image_display;
}


int main(int argc, char **argv){

    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);

    CameraGenerator front_cam = CameraGenerator("front"),
                    back_cam = CameraGenerator("back"),
                    left_cam = CameraGenerator("left"),
                    right_cam = CameraGenerator("right");

    vector<Mat> front_map = front_cam.get_bev_maps(),
                back_map = back_cam.get_bev_maps(),
                left_map = left_cam.get_bev_maps(),
                right_map = right_cam.get_bev_maps();          

    MaskGenerator front_da = MaskGenerator("front"),
                  back_da = MaskGenerator("back"),
                  left_da = MaskGenerator("left"),
                  right_da = MaskGenerator("right");

    Mat front_mask = front_da.return_mask(),
        back_mask = back_da.return_mask(),
        left_mask = left_da.return_mask(),
        right_mask = right_da.return_mask();

    final_data front_data = { front_map,front_mask },
               back_data = { back_map,back_mask },
               left_data = { left_map,left_mask },
               right_data = { right_map,right_mask };
    
    // each buffer can contain 40 images
    int producer_buffer_size = 2;
    CircularBuffer front_qeue(producer_buffer_size);
    CircularBuffer back_qeue(producer_buffer_size);
    CircularBuffer left_qeue(producer_buffer_size);
    CircularBuffer right_qeue(producer_buffer_size);
    std::unique_ptr<CircularBuffer> buffer(new CircularBuffer(2));

    auto devices_list = listDevices();
    int index_list[4] = { 0, 2, 4, 6};
    for (auto ele : devices_list) {

        if (ele.first == 1)
            index_list[0] = stoi(ele.second);//USB_camera_1 -> front
        else if (ele.first == 2)
            index_list[1] = stoi(ele.second);//USB_camera_2 -> back
        else if (ele.first == 3)
            index_list[2] = stoi(ele.second);//USB_camera_3 -> left
        else if (ele.first == 4)
            index_list[3] = stoi(ele.second);//USB_camera_4 -> right

    }
    std::thread producer_thread1([&] {producer("front", front_data, front_qeue , index_list); });
    std::thread producer_thread2([&] {producer("back", back_data, back_qeue , index_list); });
    std::thread producer_thread3([&] {producer("left", left_data, left_qeue , index_list); });
    std::thread producer_thread4([&] {producer("right", right_data, right_qeue, index_list); });
    std::thread consumer_thread([&] {consumer(front_qeue, back_qeue, left_qeue ,right_qeue); });


    producer_thread1.join();
    producer_thread2.join();
    producer_thread3.join();
    producer_thread4.join();
    consumer_thread.join();
	
    return 0;
}
