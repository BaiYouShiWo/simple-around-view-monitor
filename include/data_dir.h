#ifndef DATA_DIR_H
#define DATA_DIR_H

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

using cv::Mat;
using std::vector;

Mat padding(Mat img, int width, int height);

Mat color_balence(Mat image);

std::string get_current_directory();

Mat vector2Mat(const vector<vector<double>>& vec);

vector<vector<double>> readCSV(const std::string& filename);

vector<cv::Point> not_zero(vector<vector<int>> vec_2d);

vector<vector<int>> mat2vector(const Mat mat);

double mean(Mat list);

#endif