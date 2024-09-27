#ifndef STREAMER_H
#define STREAMER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <opencv2/opencv.hpp> 
#include <opencv2/core/utils/logger.hpp>

using cv::Mat;

class Streamer
{
public: 

private:
    int WIDTH = 0;
    int HEIGHT = 0;
    int FPS = 0;
    char errbuf[128];
    int ret;

    AVFormatContext* fmt_ctx;
    AVCodec* codec;
    AVStream* stream;
    AVCodecContext* codec_ctx;

public:
    
    explicit Streamer();
    ~Streamer();
    int streamer_init(int width,int height,int fps);
    int matToRTSP(const Mat& frame);

private:


};

#endif