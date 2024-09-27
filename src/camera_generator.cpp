#include "camera_generator.h"
#include <opencv2/opencv.hpp>
#include<vector>
#include<experimental/filesystem>
#include <cstring>

#include "data_dir.h"
#include "parameter.h"
#include "debug_utils.h"

using std::vector;
using cv::Mat;
namespace fs = std::experimental::filesystem;

Mat CameraGenerator::calculate_camera_mat_dst() {
    vector<vector<double>> cam = camera_m;
    cam[0][0] *= (double)FOCAL_SCALE;
    cam[1][1] *= (double)FOCAL_SCALE;
    cam[0][2] = (double)FRAME_WIDTH / 2 * SIZE_SCALE;
    cam[1][2] = (double)FRAME_HEIGHT / 2 * SIZE_SCALE;
    Mat camera_matrix = vector2Mat(cam);
    return camera_matrix;
}

vector<Mat> CameraGenerator::calculate_undistort_maps() {
    Mat remap_x, remap_y;
    vector<Mat> remaps;
    cv::fisheye::initUndistortRectifyMap(
        camera_mat, dist_coeff,
        Mat::eye(3, 3, CV_64F), camera_mat_dst,
        cv::Size(FRAME_WIDTH * SIZE_SCALE,
                 FRAME_HEIGHT * SIZE_SCALE),
        CV_16SC2, remap_x, remap_y);
    remaps.push_back(remap_x);
    remaps.push_back(remap_y);
    return remaps;
}

vector<Mat> CameraGenerator::calculate_rotate_undistort_maps(){
    Mat rotated_x,rotated_y;  
    cv::Point2f center(FRAME_WIDTH * SIZE_SCALE / 2.0, FRAME_HEIGHT * SIZE_SCALE / 2.0);  
    Mat rotationMatrix = cv::getRotationMatrix2D(center, 180, 1.0);  
    cv::warpAffine(undistort_maps[0], rotated_x, rotationMatrix, cv::Size(FRAME_WIDTH * SIZE_SCALE, FRAME_HEIGHT * SIZE_SCALE));
    cv::warpAffine(undistort_maps[1], rotated_y, rotationMatrix, cv::Size(FRAME_WIDTH * SIZE_SCALE, FRAME_HEIGHT * SIZE_SCALE));
    rotate_undistort_maps.push_back(rotated_x);
    rotate_undistort_maps.push_back(rotated_y);
    return rotate_undistort_maps;
}

vector<Mat> CameraGenerator::calculate_bev_maps() {
    vector<Mat> bev;
    Mat m1, m2;
    warpPerspective(rotate_undistort_maps[0], m1, homography,
                    cv::Size(project_shape[0],project_shape[1]));
    warpPerspective(rotate_undistort_maps[1], m2, homography,
                    cv::Size(project_shape[0],project_shape[1]));
    bev.push_back(m1);
    bev.push_back(m2);
    return bev;
}

CameraGenerator::CameraGenerator(std::string direction_name) {
    this->name = direction_name;
    if(name == "front") std::memcpy(project_shape, PROJECT_SHAPES[0], sizeof(PROJECT_SHAPES[0]));
    else if(name == "back") std::memcpy(project_shape, PROJECT_SHAPES[1], sizeof(PROJECT_SHAPES[1]));
    else if(name == "left") std::memcpy(project_shape, PROJECT_SHAPES[2], sizeof(PROJECT_SHAPES[2]));
    else if(name == "right") std::memcpy(project_shape, PROJECT_SHAPES[3], sizeof(PROJECT_SHAPES[3]));
    else cout<<"input name error"<< "\n";
    fs::path cur_path= get_current_directory();
    fs::path parent_path =  cur_path.parent_path();      
    fs::path K_path = parent_path / "data" / name / ("camera_" + name + "_K.csv");
    fs::path D_path = parent_path / "data" / name / ("camera_" + name + "_D.csv");
    fs::path H_path = parent_path / "data" / name / ("camera_" + name + "_H.csv");
    camera_m = readCSV(K_path);
    dist_c = readCSV(D_path);
    homo = readCSV(H_path);
    camera_mat = vector2Mat(camera_m);
    dist_coeff = vector2Mat(dist_c);
    homography = vector2Mat(homo);
    camera_mat_dst = calculate_camera_mat_dst();
    undistort_maps = calculate_undistort_maps();
    camera_rotate_mat = calculate_rotate_undistort_maps();
    bev_maps = calculate_bev_maps();
}


vector<Mat> CameraGenerator::get_bev_maps() const{
    return bev_maps;
}

vector<Mat> CameraGenerator::get_undistort_maps() const{
    return undistort_maps;
}
