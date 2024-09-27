#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H
#include <iostream>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

using std::cout;
using std::endl;
using std::vector;

std::map<int, std::string> listDevices();

cv::Mat total_picture(cv::Mat img1, cv::Mat img2, cv::Mat img3, cv::Mat img4); 

template<typename T>
void show2dvector(vector<vector<T>> ptr) {
    int size_rows = ptr.size(), size_cols = ptr[0].size();
    if (size_rows < 6 && size_cols < 6) {
        for (vector<T> rows : ptr) {
            for (T elem : rows) {
                std::cout << elem << "  ";
            }
            std::cout << std::endl;
        }
    }
    else if (size_rows < 6 && size_cols > 6) {
        for (vector<T> rows : ptr) {
            for (int i = 0; i < 6; ++i) {
                if (i < 3)
                    std::cout << rows[i] << "  ";
                else if (i == 2)
                    std::cout << "...";
                else
                    std::cout << rows[size_cols - 6 + i] << "  ";
            }
            std::cout << std::endl;
        }
    }
    else if (size_rows > 6 && size_cols < 6) {
        for (int i = 0; i < 6; ++i) {
            if (i < 3) {
                vector<T> rows = ptr[i];
                for (T elem : rows)
                    cout << elem << "  ";
                std::cout << endl;
            }
            else if (i == 2) {
                cout << "." << endl << "." << endl << "." << endl;
            }
            else {
                vector<T> rows = ptr[size_rows - 6 + i];
                for (T elem : rows)
                    cout << elem << "  ";
                cout << endl;
            }
        }
    }
    else {
        for (int i = 0; i < 6; ++i) {
            if (i < 3) {
                vector<T> rows = ptr[i];
                for (int i = 0; i < 6; ++i) {
                    if (i < 3)
                        cout << rows[i] << "  ";
                    else if (i == 2)
                        cout << "...";
                    else
                        cout << rows[size_cols - 6 + i] << "  ";
                }
                std::cout << std::endl;
            }
            else if (i == 2) {
                cout << "." << endl << "." << endl << "." << endl;
            }
            else {
                vector<T> rows = ptr[size_rows - 6 + i];
                for (int i = 0; i < 6; ++i) {
                    if (i < 3)
                        cout << rows[i] << "  ";
                    else if (i == 2)
                        cout << "...";
                    else
                        cout << rows[size_cols - 6 + i] << "  ";
                }
                std::cout << std::endl;
            }
        }
    }

}

#endif
