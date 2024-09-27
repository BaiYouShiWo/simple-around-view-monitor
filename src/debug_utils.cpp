#include"debug_utils.h"

#include<iostream>
#include <map>
#include <regex>
#include <glob.h>
#include <vector>
#include <string>
#include <fstream>
#include  <experimental/filesystem>

#include <opencv2/opencv.hpp> 

using std::vector;
using cv::Mat;
namespace fs = std::experimental::filesystem;

void showMat(Mat image) {
    cv::Mat_<uchar> ptr = image;
    int cols = image.cols, rows = image.rows;
    for (int j = 0; j < rows; ++j) {
        for (int i = 0; i < cols; ++i) {
            if ((abs(i - rows) < 4) && (abs(j - cols) < 4)) {
                std::cout << image.col(j).row(i);
            }
        }
    }
}



std::string getDeviceName(const std::string& path) {
    std::ifstream infile(path + "/name");
    std::string name;
    std::getline(infile, name);
    return name;
}

std::vector<std::string> getDevicePaths(const std::string& path) {
    std::vector<std::string> devicePaths;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_directory(entry.path())) {
            devicePaths.push_back(entry.path().string());
        }
    }
    return devicePaths;
}


std::map<int, std::string> listDevices() {
    const std::string basePath = "/sys/class/video4linux";
    auto devices = getDevicePaths(basePath);
    std::regex pattern_name(R"(USB_Camera_(\d+))");
    std::regex pattern_path(R"(/dev/video(\d+))");
    std::smatch match_name,match_path;

    std::map<int, std::string> cam_list;
    
    for (const auto& device : devices) {
        std::string name = getDeviceName(device);
        std::string devPath = "/dev/" + fs::path(device).filename().string();
        //std::cout << name << ": " << devPath << std::endl;
        if (std::regex_search(name, match_name, pattern_name)) {
            //std::cout << "Camera Number: " << match_name[1] << std::endl;
        }
        if (std::regex_search(devPath, match_path, pattern_path)) {
            //std::cout << "Device Path: " << match_path[1] << std::endl;
        }
        if(stoi(match_path[1])%2 == 1){
            continue;
        }
        cam_list[stoi(match_name[1])] = match_path[1];
    }

    return cam_list;
}

Mat total_picture(cv::Mat img1, cv::Mat img2, cv::Mat img3, cv::Mat img4){
    cv::Mat horizontal_concatenation1,horizontal_concatenation2;
    // 分别拼接四张图像
    cv::hconcat(img1, img2, 
                horizontal_concatenation1);
    cv::hconcat(img3, img4, 
                horizontal_concatenation2);
    // 垂直拼接两组图像
    cv::Mat vertical_concatenation;
    cv::vconcat(horizontal_concatenation1, horizontal_concatenation2,
                vertical_concatenation);
    return vertical_concatenation;
}


