#include "mask_generator.h"
#include "data_dir.h"
#include "parameter.h"
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

void MaskGenerator::load_weights(){

    fs::path left_image_path  = weightsImagePath / ("weight_" + weight_name[0].first + "." + "png");
    fs::path right_image_path = weightsImagePath / ("weight_" + weight_name[1].first + "." + "png");

    //std::cout<<left_image_path<<"\n"<<right_image_path<<"\n";
    Mat weight_left_raw = cv::imread(left_image_path,cv::IMREAD_UNCHANGED);
    Mat weight_right_raw = cv::imread(right_image_path,cv::IMREAD_UNCHANGED);  
    if(weight_left_raw.data == nullptr || weight_right_raw.data == nullptr){
        std::cerr<<"weight read failed"<<std::endl;
        exit(0);
    }
    
    Mat weight_left_rotate,weight_right_rotate;

    if(weight_rotate_angle == 0){
        weight_left_rotate = weight_left_raw;
        weight_right_rotate = weight_right_raw;
    }else if(weight_rotate_angle == 90){
        cv::rotate(weight_left_raw, weight_left_rotate, cv::ROTATE_90_CLOCKWISE);
        cv::rotate(weight_right_raw, weight_right_rotate, cv::ROTATE_90_CLOCKWISE);
    }else if(weight_rotate_angle == 180){
        cv::rotate(weight_left_raw, weight_left_rotate, cv::ROTATE_180);
        cv::rotate(weight_right_raw, weight_right_rotate, cv::ROTATE_180);
    }else if(weight_rotate_angle == 270){
        cv::rotate(weight_left_raw, weight_left_rotate, cv::ROTATE_90_COUNTERCLOCKWISE);
        cv::rotate(weight_right_raw, weight_right_rotate, cv::ROTATE_90_COUNTERCLOCKWISE);
    }else{
        std::cerr<<"angle input error";
    }

    if(weight_name[0].second == -1){
        cv::bitwise_not(weight_left_rotate,weight_left);
        cv::bitwise_not(weight_right_rotate,weight_right);
    }else{
        weight_left = weight_left_rotate;
        weight_right = weight_right_rotate;
    }
    weight_mid = Mat::zeros(mask_shape[1], mask_shape[0] - (weight_left.cols + weight_right.cols) , CV_8U);
    weight_mid.setTo(cv::Scalar(255));
}

Mat MaskGenerator::add_weights(){
    Mat mask = Mat::zeros(mask_shape[1], mask_shape[0], CV_8U);
    cv::Point position_img_left(0, 0); 
    cv::Point position_blank(weight_left.cols,0);
    cv::Point position_img_right(weight_left.cols + weight_mid.cols,0); 
    cv::Rect roi_left(position_img_left, weight_left.size());  
    cv::Rect roi_mid(position_blank, weight_mid.size());  
    cv::Rect roi_right(position_img_right, weight_right.size());  

    weight_left.copyTo(mask(roi_left));
    weight_mid.copyTo(mask(roi_mid));
    weight_right.copyTo(mask(roi_right));
    Mat mask_float;
    mask.convertTo(mask_float, CV_32F, 1.0 / 255.0);// convert the pixcel value to a [0,1] float
    Mat mask_3_channels;
    vector<Mat> mask_channels(3, mask_float); // repeat the weight 3 times
    cv::merge(mask_channels, mask_3_channels);
    final_mask = mask_3_channels;
    return final_mask;
}


MaskGenerator::MaskGenerator(std::string direction_name) {    
    name = direction_name;

    fs::path cur_path = get_current_directory();
    fs::path parent_path = cur_path.parent_path();     

    weightsImagePath = parent_path/"data"/"weights";
    masksImagePath = parent_path/"data"/"weights";

    if (name == "front") {
      weight_name.push_back(std::make_pair("left_front",1));
      weight_name.push_back(std::make_pair("front_right",1));
      std::memcpy(mask_shape, PROJECT_SHAPES[0], sizeof(PROJECT_SHAPES[0]));
      weight_rotate_angle = 0;
    } else if (name == "back") {
      weight_name.push_back(std::make_pair("right_back",1));
      weight_name.push_back(std::make_pair("back_left",1));
      std::memcpy(mask_shape, PROJECT_SHAPES[1], sizeof(PROJECT_SHAPES[1]));
      weight_rotate_angle = 180;
    } else if (name == "left") {
      weight_name.push_back(std::make_pair("back_left",-1));
      weight_name.push_back(std::make_pair("left_front",-1));
      std::memcpy(mask_shape, PROJECT_SHAPES[2], sizeof(PROJECT_SHAPES[2]));
      weight_rotate_angle = 90;
    } else if (name == "right") {
      weight_name.push_back(std::make_pair("front_right",-1));
      weight_name.push_back(std::make_pair("right_back",-1));
      std::memcpy(mask_shape, PROJECT_SHAPES[3], sizeof(PROJECT_SHAPES[3]));
      weight_rotate_angle = 270;
    } else {
      std::cerr << "name error" << std::endl;
    }

    load_weights();
    final_mask = add_weights();

}

Mat MaskGenerator::return_mask() {
    return final_mask;//weight;
}

