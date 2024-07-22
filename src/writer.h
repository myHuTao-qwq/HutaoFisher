#ifndef IMAGE_WRITER_H
#define IMAGE_WRITER_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <queue>
#include <string>
#include <thread>

class Writer {
 public:
  Writer();
  ~Writer();

  void addImage(char* filename, const cv::Mat& image);

 private:
  void writeImages();

  std::queue<std::string> filenames;
  std::queue<cv::Mat> images;
  std::thread writer_thread;
  std::mutex queue_mutex;
  std::condition_variable cv;
  bool stop_thread;
};

#endif  // IMAGE_WRITER_H
