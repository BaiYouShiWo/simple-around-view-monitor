#ifndef MASK_GENERATOR_H
#define MASK_GENERATOR_H

#include <vector>
#include <cstring>
#include <experimental/filesystem>

#include <opencv2/opencv.hpp>

namespace fs = std::experimental::filesystem;
using std::vector;
using cv::Mat;

class MaskGenerator {
private:
    std::string name;
    fs::path weightsImagePath, masksImagePath;
    vector<std::pair<std::string,int>> weight_name;
    int weight_rotate_angle = 0;
    int mask_shape[2];
    Mat weight_left,weight_right,weight_mid;//weight_mid is just a white board
    vector<Mat> masks;//need 2 weight to generate mask
    Mat final_mask;

public:


private:
    void load_weights();
    Mat add_weights();

public:
    MaskGenerator(std::string direction_name);
    Mat return_mask();
};


#endif