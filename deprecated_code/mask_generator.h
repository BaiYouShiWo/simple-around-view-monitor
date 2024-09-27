#ifndef MASK_GENERATOR_H
#define MASK_GENERATOR_H
#include <opencv2/opencv.hpp>
#include<vector>

using std::vector;
using cv::Mat;

struct line_set {
    vector<cv::Point>   lineFL,
        lineFR,
        lineBL,
        lineBR,
        lineLF,
        lineLB,
        lineRF,
        lineRB;
};

class MaskGenerator {
private:
    Mat final_mask;
    line_set lines;
    Mat weight;

    vector<vector<cv::Point>> get_points(std::string name);

    void get_lines();

    Mat get_mask(std::string name);

    Mat get_blend_mask(Mat mask_a,
        Mat mask_b,
        vector<cv::Point> line_a,
        vector<cv::Point> line_b);
public:
    MaskGenerator(std::string direction_name);
    Mat return_mask();
};


#endif