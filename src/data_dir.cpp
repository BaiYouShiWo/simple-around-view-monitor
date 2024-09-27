#include "data_dir.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include <opencv2/opencv.hpp>
#if defined(_MSC_VER)
#include <direct.h>
#define GetCurrentDir _getcwd
#elif defined(__unix__)
#include <unistd.h>
#define GetCurrentDir getcwd
#else
#endif

using cv::Mat;
using std::vector;

Mat padding(Mat img, int width, int height) {
    int H = img.rows;
    int W = img.cols;

    int top = (height - H) / 2;
    int bottom = (height - H) / 2;

    if (top + bottom + H < height)
        bottom += 1;

    int left = (width - W) / 2;
    int right = (width - W) / 2;

    if (left + right + W < width)
        right += 1;

    Mat car_border;
    copyMakeBorder(img, car_border, top, bottom, left, right,
        cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

    return car_border;
}

Mat color_balence(Mat image) {
    Mat rgb[3];
    split(image, rgb);
    double R = mean(rgb[0]);
    double G = mean(rgb[1]);
    double B = mean(rgb[2]);
    double K = (R + G + B) / 3;
    double Kr = K / R;
    double Kg = K / G;
    double Kb = K / B;
    vector<Mat> mbgr(3);
    addWeighted(rgb[0], Kb, 0, 0, 0, rgb[0]);
    addWeighted(rgb[1], Kg, 0, 0, 0, rgb[1]);
    addWeighted(rgb[2], Kr, 0, 0, 0, rgb[2]);
    mbgr[0] = rgb[0];
    mbgr[1] = rgb[1];
    mbgr[2] = rgb[2];
    Mat out_put;
    merge(mbgr, out_put);
    return out_put;
}

std::vector<cv::Mat> luminance_balance(const std::vector<cv::Mat>& images) {
    // 将BGR图像转换为HSV
    std::vector<cv::Mat> hsvImages(4);
    for (int i = 0; i < 4; ++i) {
        cv::cvtColor(images[i], hsvImages[i], cv::COLOR_BGR2HSV);
    }

    // 分割通道
    std::vector<cv::Mat> h(4), s(4), v(4);
    for (int i = 0; i < 4; ++i) {
        cv::Mat channels[3];
        cv::split(hsvImages[i], channels);
        h[i] = channels[0];
        s[i] = channels[1];
        v[i] = channels[2];
    }

    // 计算每个通道的平均亮度值
    double V_f = cv::mean(v[0])[0];
    double V_b = cv::mean(v[1])[0];
    double V_l = cv::mean(v[2])[0];
    double V_r = cv::mean(v[3])[0];

    double V_mean = (V_f + V_b + V_l + V_r) / 4.0;

    // 调整V通道的亮度
    v[0] += (V_mean - V_f);
    v[1] += (V_mean - V_b);
    v[2] += (V_mean - V_l);
    v[3] += (V_mean - V_r);

    // 合并HSV通道并转换回BGR
    std::vector<cv::Mat> balancedImages(4);
    for (int i = 0; i < 4; ++i) {
        cv::merge(std::vector<cv::Mat>{h[i], s[i], v[i]}, hsvImages[i]);
        cv::cvtColor(hsvImages[i], balancedImages[i], cv::COLOR_HSV2BGR);
    }

    return balancedImages;
}

std::string get_current_directory() {
    char buff[250];
    GetCurrentDir(buff, 250);
    std::string current_working_directory(buff);
    return current_working_directory;
}

Mat vector2Mat(const vector<vector<double>>& vec) {
    
    int rows = vec.size();
    if(rows == 0){
        std::cerr<<"read vector failed"<<std::endl;
        return Mat();
    }
    int cols = vec[0].size();
    Mat mat(rows, cols, CV_64F);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            mat.at<double>(i, j) = vec[i][j];
        }
    }
    return mat;
}

vector<vector<double>> readCSV(const std::string& filename) {
    vector<vector<double>> data;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        vector<double> row;
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            row.push_back(std::stof(cell));
        }
        data.push_back(row);
    }

    return data;
}

vector<cv::Point> not_zero(vector<vector<int>> vec_2d) {
    vector<cv::Point> result;
    for (int i = 0; i < vec_2d.size(); ++i) {
        for (int j = 0; j < vec_2d[i].size(); ++j) {
            if (vec_2d[i][j] != 0) {
                result.push_back(cv::Point(i, j));
            }
        }
    }
    return result;
}

vector<vector<int>> mat2vector(const Mat mat) {
    int cols = mat.cols, rows = mat.rows;
    vector<vector<int>> vec_2d(rows, vector<int>(cols));
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < rows; ++j) {
            vec_2d[j][i] = mat.at<uchar>(j, i);
        }
    }
    return vec_2d;
}

double mean(Mat list) {
    int cols = list.cols, rows = list.rows;
    uchar* ptr = list.ptr(0);
    int sum = 0;
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < rows; ++j) {
            sum += ptr[i * cols];
        }
    }
    return (double)sum / (cols * rows);
}
