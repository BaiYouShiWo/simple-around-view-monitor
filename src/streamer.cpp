#include "streamer.h"

#include <chrono>
#include <thread>

#define MAX_RETRIES 10

Streamer::Streamer():
    fmt_ctx(nullptr),
    codec(nullptr),
    stream(nullptr),
    codec_ctx(nullptr){

    av_log_set_level(AV_LOG_ERROR);
}

int Streamer::streamer_init(int width,int height,int fps){
    WIDTH = width;
    HEIGHT = height;
    FPS = fps;
    // 初始化FFmpeg库
    avformat_network_init();

    // 设置输出文件格式（RTSP）
    avformat_alloc_output_context2(&fmt_ctx, nullptr, "rtsp", 
                                   "rtsp://localhost:8554/test");
    if (!fmt_ctx) {
        std::cerr << "Could not allocate output context." << std::endl;
        return -1;
    }
    // 编码器设置
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
	    fprintf(stderr, "H.264 encoder not found\n");
        return -1;
    } 
    //初始化视频流
    stream = avformat_new_stream(fmt_ctx, codec);
    if(!stream){
	    std::cerr<<"stream init failed"<<std::endl;
    }
    
    //初始化编码器上下文
    codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->codec_id = AV_CODEC_ID_H264;
    codec_ctx->width = WIDTH;
    codec_ctx->height = HEIGHT;
    codec_ctx->time_base = {1, FPS};
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->bit_rate = 2000000;
    codec_ctx->gop_size = 12;
    codec_ctx->max_b_frames = 1;
    av_opt_set(codec_ctx->priv_data, "preset", "fast", 0); // 使用更高质量的预设
    av_opt_set(codec_ctx->priv_data, "crf", "23", 0); // 设置 CRF 值
    avcodec_parameters_from_context(stream->codecpar, codec_ctx);
    stream->time_base = codec_ctx->time_base;
    avcodec_open2(codec_ctx, codec, nullptr);
    avcodec_parameters_from_context(stream->codecpar, codec_ctx);

    //连接rtsp服务器并发送编码器信息
    avio_open(&fmt_ctx->pb, "rtsp://localhost:8554/test", AVIO_FLAG_WRITE);
    ret = avformat_write_header(fmt_ctx, nullptr);
    if(ret < 0){
        memset(errbuf,0,sizeof(errbuf));
        av_strerror(ret, errbuf, sizeof(errbuf));
	    std::cerr<<"header write failed:" <<ret << " "<< errbuf << std::endl;
        return -1;
    }

    std::cout<<"successfully connected rtsp server" << std::endl;
    return 1;
}

int Streamer::matToRTSP(const Mat& frame){

    static auto lastFrameTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto frameDuration = std::chrono::milliseconds((int)(1000/FPS)); // For 24 FPS

    if (currentTime - lastFrameTime < frameDuration) {
        std::this_thread::sleep_for(frameDuration - (currentTime - lastFrameTime));
    }

    lastFrameTime = std::chrono::steady_clock::now();
    //初始化packet
    AVPacket *pkt = av_packet_alloc();
    pkt->data = nullptr;
    pkt->size = 0;

    //初始化存放一帧的av_frame
    AVFrame* av_frame = av_frame_alloc();
    av_frame->format = codec_ctx->pix_fmt;
    av_frame->width = codec_ctx->width;
    av_frame->height = codec_ctx->height;
    int ret = av_frame_get_buffer(av_frame, 32);
    char errbuf[128];
    if (ret < 0) {
        std::cerr << "Could not allocate AVFrame buffer." << std::endl;
        av_frame_free(&av_frame);
        return -1;
    }

    // 使用 libswscale 进行格式转换
    SwsContext* sws_ctx = sws_getContext(
        frame.cols, frame.rows, AV_PIX_FMT_BGR24,
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        std::cerr << "Could not create SwsContext." << std::endl;
        av_frame_free(&av_frame);
        return -1;
    }

    uint8_t* src_data[4];
    int src_linesize[4];
    src_data[0] = frame.data;
    src_linesize[0] = frame.step;

    // 将 BGR 数据转换为 YUV420P 并存储在 av_frame 中
    sws_scale(sws_ctx, src_data, src_linesize, 0, 
            frame.rows, av_frame->data, av_frame->linesize);
    sws_freeContext(sws_ctx);


    int retries = 0;
    bool frame_sent = false;
    static int64_t framecount = 0;
    do {
        av_frame->pts = framecount++;

        //向编码器发送数据包
        ret = avcodec_send_frame(codec_ctx, av_frame);
        if (ret < 0) {
            av_strerror(ret, errbuf, sizeof(errbuf));
            std::cerr << "Error sending frame to encoder: " << errbuf << std::endl;
            return -1;
        }
        // 从编码器接收数据包
        ret = avcodec_receive_packet(codec_ctx, pkt);
        if (ret == AVERROR(EAGAIN)) {

            // 编码器需要更多输入或缓冲区已满，继续尝试
            if (++retries > MAX_RETRIES) {
                std::cerr << "Error: Exceeded maximum retries for receiving packet." << std::endl;
                return -1;
            }
            return -2;//-2代表需要更多帧

        } else if (ret == AVERROR_EOF) {
            // 编码器完成了所有输入数据的处理
            frame_sent = false;
            break;
        } else if (ret < 0) {
            av_strerror(ret, errbuf, sizeof(errbuf));
            std::cerr << "Error receiving packet from encoder: " << errbuf << std::endl;
            return -1;
        }

        // 成功接收到数据包
        frame_sent = true;

    } while (!frame_sent);

    //转换packet的时间戳
    pkt->stream_index = stream->index;
    av_packet_rescale_ts(pkt, codec_ctx->time_base, stream->time_base);

    // 确保 pkt->pts 和 pkt->dts 已正确设置
    if (pkt->pts == AV_NOPTS_VALUE || pkt->dts == AV_NOPTS_VALUE) {
        std::cerr << "Error: PTS or DTS not set correctly!" << std::endl;
    }

    //向服务器写入packet
    ret = av_interleaved_write_frame(fmt_ctx, pkt);
    if(ret < 0){
        memset(errbuf,0,sizeof(errbuf));
        av_strerror(ret, errbuf, sizeof(errbuf));
        std::cout << "send failed:" << errbuf << std::endl;
    }

    //清空frame和packet
    av_frame_free(&av_frame);
    av_packet_unref(pkt);
    return 0;
}

Streamer::~Streamer(){

    //ffmpeg资源释放
    av_write_trailer(fmt_ctx);
    avio_close(fmt_ctx->pb);
    avcodec_free_context(&codec_ctx);
    avformat_free_context(fmt_ctx);
    avformat_network_deinit();
}