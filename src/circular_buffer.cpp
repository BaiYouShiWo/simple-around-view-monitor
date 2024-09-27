#include "circular_buffer.h"
#include <condition_variable>

CircularBuffer::CircularBuffer(size_t size) : buffer_size(size) {}

CircularBuffer::CircularBuffer():buffer_size(40){}

void CircularBuffer::push(const cv::Mat& image) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (buffer_.size() >= buffer_size) {
        // if the buffer full,wait for the consumer 
        condition_full_.wait(lock);
    }
    buffer_.push(image);
    condition_empty_.notify_one();
}

cv::Mat CircularBuffer::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (buffer_.empty()) {
        // buffer is empty ,wait for the producer
        condition_empty_.wait(lock);
    }
    cv::Mat image = buffer_.front();
    buffer_.pop();
    condition_full_.notify_one();
    return image;
}

bool CircularBuffer::empty() {
    return buffer_.empty();
}
