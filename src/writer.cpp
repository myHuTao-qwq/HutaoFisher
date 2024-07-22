#include "writer.h"

Writer::Writer()
    : stop_thread(false), writer_thread(&Writer::writeImages, this) {}

Writer::~Writer() {
  stop_thread = true;
  cv.notify_all();
  writer_thread.join();
}

void Writer::addImage(char* filename, const cv::Mat &image) {
  std::lock_guard<std::mutex> lock(queue_mutex);
  filenames.push(filename);
  images.push(image);
  cv.notify_one();
}

void Writer::writeImages() {
  while (!stop_thread) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv.wait(lock, [this]() { return !filenames.empty() || stop_thread; });

    while (!filenames.empty() && !images.empty()) {
      std::string filename = filenames.front();
      cv::Mat image = images.front();
      filenames.pop();
      images.pop();
      lock.unlock();

      cv::imwrite(filename, image);

      lock.lock();
    }
  }
}
