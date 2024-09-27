#include "mask_generator.h"
#include "data_dir.h"
#include "parameter.h"

vector<vector<cv::Point>> MaskGenerator::get_points(std::string name) {

    vector<vector<cv::Point>> points;
    if (name == "front") {
        points = { {{0, 0} ,
                {BEV_WIDTH, 0 },
                {BEV_WIDTH, BEV_HEIGHT / 5},
                {(BEV_WIDTH + CAR_WIDTH) / 2, (BEV_HEIGHT - CAR_HEIGHT) / 2},
                {(BEV_WIDTH - CAR_WIDTH) / 2, (BEV_HEIGHT - CAR_HEIGHT) / 2 },
                {0, BEV_HEIGHT / 5}}
        };
    }
    else if (name == "back") {
        points = { { {0, BEV_HEIGHT} ,
                { BEV_WIDTH ,  BEV_HEIGHT },
                {BEV_WIDTH, BEV_HEIGHT - BEV_HEIGHT / 5},
                {(BEV_WIDTH + CAR_WIDTH) / 2, (BEV_HEIGHT + CAR_HEIGHT) / 2},
                {(BEV_WIDTH - CAR_WIDTH) / 2, (BEV_HEIGHT + CAR_HEIGHT) / 2 },
                {0, BEV_HEIGHT - BEV_HEIGHT / 5}}
        };
    }
    else if (name == "left") {
        points = { { {0, 0} ,
                { 0 , BEV_HEIGHT },
                {BEV_WIDTH / 5, BEV_HEIGHT},
                {(BEV_WIDTH - CAR_WIDTH) / 2, (BEV_HEIGHT + CAR_HEIGHT) / 2},
                {(BEV_WIDTH - CAR_WIDTH) / 2, (BEV_HEIGHT - CAR_HEIGHT) / 2 },
                {BEV_WIDTH / 5, 0}}
        };
    }
    else if (name == "right") {
        points = { { {BEV_WIDTH, 0} ,
                { BEV_WIDTH , BEV_HEIGHT },
                {BEV_WIDTH - BEV_WIDTH / 5, BEV_HEIGHT},
                {(BEV_WIDTH + CAR_WIDTH) / 2, (BEV_HEIGHT + CAR_HEIGHT) / 2},
                {(BEV_WIDTH + CAR_WIDTH) / 2, (BEV_HEIGHT - CAR_HEIGHT) / 2 },
                {BEV_WIDTH - BEV_WIDTH / 5, 0}}
        };
    }

    return points;

}

void MaskGenerator::get_lines() {
    lines.lineFL = {
        {0, BEV_HEIGHT / 5} ,
        {(BEV_WIDTH - CAR_WIDTH) / 2, (BEV_HEIGHT - CAR_HEIGHT) / 2}
    };

    lines.lineFR = {
        {BEV_WIDTH, BEV_HEIGHT / 5} ,
        {(BEV_WIDTH + CAR_WIDTH) / 2, (BEV_HEIGHT - CAR_HEIGHT) / 2}
    };
    lines.lineBL = {
        {0, BEV_HEIGHT - BEV_HEIGHT / 5} ,
        {(BEV_WIDTH - CAR_WIDTH) / 2, (BEV_HEIGHT + CAR_HEIGHT) / 2}
    };
    lines.lineBR = {
        {BEV_WIDTH, BEV_HEIGHT - BEV_HEIGHT / 5} ,
        {(BEV_WIDTH + CAR_WIDTH) / 2, (BEV_HEIGHT + CAR_HEIGHT) / 2},
    };
    lines.lineLF = {
        {BEV_WIDTH / 5, 0},
        {(BEV_WIDTH - CAR_WIDTH) / 2, (BEV_HEIGHT - CAR_HEIGHT) / 2}
    };
    lines.lineLB = {
        {BEV_WIDTH / 5, BEV_HEIGHT} ,
        {(BEV_WIDTH - CAR_WIDTH) / 2, (BEV_HEIGHT + CAR_HEIGHT) / 2}
    };
    lines.lineRF = {
        {BEV_WIDTH - BEV_WIDTH / 5, 0},
        {(BEV_WIDTH + CAR_WIDTH) / 2, (BEV_HEIGHT - CAR_HEIGHT) / 2}
    };
    lines.lineRB = {
        {BEV_WIDTH - BEV_WIDTH / 5, BEV_HEIGHT},
        {(BEV_WIDTH + CAR_WIDTH) / 2, (BEV_HEIGHT + CAR_HEIGHT) / 2}
    };
}

Mat MaskGenerator::get_mask(std::string name) {
    Mat mask = Mat::zeros(BEV_HEIGHT, BEV_WIDTH, CV_8U);
    //it supposed to be defined as an list[BEV_WIDTH][BEV_HEIGHT]
    vector < vector < cv::Point >> points = get_points(name);
    fillPoly(mask, points, cv::Scalar(255));
    return mask;
}

Mat MaskGenerator::get_blend_mask(Mat mask_a,
    Mat mask_b,
    vector<cv::Point> line_a,
    vector<cv::Point> line_b) {
    Mat overlap;
    bitwise_and(mask_a, mask_b, overlap);
    vector<vector<int>> overlap_vec = mat2vector(overlap);
    vector<cv::Point> indices = not_zero(overlap_vec);
    //cout << indices << endl;
    for (int k = 0; k < indices.size(); ++k) {
        long double dist_a, dist_b;
        dist_a = pointPolygonTest(line_a, cv::Point(indices[k].y, indices[k].x), 1);
        dist_b = pointPolygonTest(line_b, cv::Point(indices[k].y, indices[k].x), 1);
        mask_a.at<uchar>(cv::Point(indices[k].y, indices[k].x)) =
            int(dist_a * dist_a / (dist_a * dist_a + dist_b * dist_a + 1e-6) * 255 + 0.5);
    }
    return mask_a;
}

MaskGenerator::MaskGenerator(std::string direction_name) {
    Mat mf, mb, ml, mr;
    mf = get_mask("front");
    mb = get_mask("back");
    ml = get_mask("left");
    mr = get_mask("right");
    get_lines();

    if (direction_name == "front") {
        mf = get_blend_mask(mf, ml, lines.lineFL, lines.lineLF);
        mf = get_blend_mask(mf, mr, lines.lineFR, lines.lineRF);
        this->final_mask = mf;
    }
    else if (direction_name == "back") {
        mb = get_blend_mask(mb, ml, lines.lineBL, lines.lineLB);
        mb = get_blend_mask(mb, mr, lines.lineBR, lines.lineRB);
        this->final_mask = mb;
    }
    else if (direction_name == "left") {
        ml = get_blend_mask(ml, mf, lines.lineLF, lines.lineFL);
        ml = get_blend_mask(ml, mb, lines.lineLB, lines.lineBL);
        this->final_mask = ml;

    }
    else if (direction_name == "right") {
        mr = get_blend_mask(mr, mf, lines.lineRF, lines.lineFR);
        mr = get_blend_mask(mr, mb, lines.lineRB, lines.lineBR);
        this->final_mask = mr;
    }

    Mat mask_float;
    final_mask.convertTo(mask_float, CV_32F, 1.0 / 255.0);
    // convert the pixcel value to a [0,1] float
    Mat mask_3_channels;
    vector<Mat> mask_channels(3, mask_float); // repeat the weight 3 times
    cv::merge(mask_channels, mask_3_channels);
    this->final_mask = mask_3_channels;
}
Mat MaskGenerator::return_mask() {
    return final_mask;//weight;
}

