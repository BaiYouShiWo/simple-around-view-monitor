#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H
#include <mutex>
#include <memory>
#include <opencv2/opencv.hpp>
#include <condition_variable>

class CircularBuffer {
public:
    CircularBuffer(size_t size);
	CircularBuffer();
    void push(const cv::Mat& image);
    cv::Mat pop();
    bool empty();

private:
    size_t buffer_size;
    std::queue<cv::Mat> buffer_;
    std::mutex mutex_;
    std::condition_variable condition_empty_;
    std::condition_variable condition_full_;
};
#endif