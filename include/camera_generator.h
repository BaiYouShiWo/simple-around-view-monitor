#ifndef CAMERA_GENERATOR_H
#define CAMERA_GENERATOR_H
#include <opencv2/opencv.hpp>
#include<vector>

using std::vector;
using cv::Mat;

class CameraGenerator {
private:

    int project_shape[2];

    vector<vector<double>> camera_m;
    vector<vector<double>> dist_c;
    vector<vector<double>> homo;

    Mat camera_mat;
    Mat dist_coeff;
    Mat homography;
    Mat camera_mat_dst;
    vector<Mat> camera_rotate_mat;
    vector<Mat> undistort_maps;
    vector<Mat> rotate_undistort_maps;
    vector<Mat> bev_maps;

    Mat calculate_camera_mat_dst();

    vector<Mat> calculate_undistort_maps();

    vector<Mat> calculate_rotate_undistort_maps();

    vector<Mat> calculate_bev_maps();

public:
    std::string name;
    CameraGenerator(std::string direction_name);
    vector<Mat> get_bev_maps() const;
    vector<Mat> get_undistort_maps() const;
};

#endif