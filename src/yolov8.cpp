//
// Create by RangiLyu
// 2020 / 10 / 2
// Adapted for YOLOV8 by myHuTao_qwq
//

#include "yolov8.h"

bool YOLOV8::hasGPU = false;
YOLOV8* YOLOV8::detector = nullptr;

YOLOV8::YOLOV8(const char* param, const char* bin, bool useGPU) {
  this->Net = new ncnn::Net();
  // opt
#if NCNN_VULKAN
  this->hasGPU = ncnn::get_gpu_count() > 0;
#endif
  this->Net->opt.use_vulkan_compute = this->hasGPU && useGPU;
  this->Net->opt.use_fp16_arithmetic = true;
  this->Net->opt.num_threads = 4;
  this->Net->load_param(param);
  this->Net->load_model(bin);
}

YOLOV8::~YOLOV8() { delete this->Net; }

void YOLOV8::preprocess(cv::Mat& image, ncnn::Mat& in) {
  int img_w = image.cols;
  int img_h = image.rows;

  in = ncnn::Mat::from_pixels(image.data, ncnn::Mat::PIXEL_RGB2BGR, img_w,
                              img_h);

  const float mean_vals[3] = {0.f, 0.f, 0.f};
  const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
  in.substract_mean_normalize(mean_vals, norm_vals);
}

std::vector<BoxInfo> YOLOV8::detect(cv::Mat image, float score_threshold,
                                    float nms_threshold) {
  ncnn::Mat input;
  preprocess(image, input);

  // double start = ncnn::get_current_time();

  auto ex = this->Net->create_extractor();
  ex.set_light_mode(false);

  ex.input("in0", input);

  std::vector<std::vector<BoxInfo>> results;
  results.resize(this->num_class);

  ncnn::Mat out;
  ex.extract("out0", out);

  this->decode_infer(out, score_threshold, results);

  std::vector<BoxInfo> dets;
  for (int i = 0; i < (int)results.size(); i++) {
    this->nms(results[i], nms_threshold);
    for (auto box : results[i]) {
      dets.push_back(box);
    }
  }

  return dets;
}

void YOLOV8::decode_infer(ncnn::Mat& feats, float threshold,
                          std::vector<std::vector<BoxInfo>>& results) {
  const int num_points = feats.w;
  const int data_length = feats.h;
  float data[256];
  float* data_ptr[256];

  for (int i = 0; i < data_length; i++) {
    data_ptr[i] = feats.row(i);
  }

  for (int i = 0; i < num_points; i++) {
    for (int j = 0; j < data_length; j++) {
      data[j] = *data_ptr[j];
    }
    float* score = std::max_element(data + 4, data + data_length);
    if (*score > threshold) {
      int label = int(score - data - 4);
      float x1, y1, x2, y2;
      x1 = std::max(data[0] - data[2] / 2, 0.f);
      y1 = std::max(data[1] - data[3] / 2, 0.f);
      x2 = std::min(data[0] + data[2] / 2, float(input_size[1]));
      y2 = std::min(data[1] + data[3] / 2, float(input_size[0]));
      results[label].push_back(BoxInfo{x1, y1, x2, y2, *score, label});
    }
    for (int j = 0; j < data_length; j++) {
      data_ptr[j]++;
    }
  }
}

void YOLOV8::nms(std::vector<BoxInfo>& input_boxes, float NMS_THRESH) {
  std::sort(input_boxes.begin(), input_boxes.end(),
            [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
  std::vector<float> vArea(input_boxes.size());
  for (int i = 0; i < int(input_boxes.size()); ++i) {
    vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1) *
               (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
  }
  for (int i = 0; i < int(input_boxes.size()); ++i) {
    for (int j = i + 1; j < int(input_boxes.size());) {
      float xx1 = (std::max)(input_boxes[i].x1, input_boxes[j].x1);
      float yy1 = (std::max)(input_boxes[i].y1, input_boxes[j].y1);
      float xx2 = (std::min)(input_boxes[i].x2, input_boxes[j].x2);
      float yy2 = (std::min)(input_boxes[i].y2, input_boxes[j].y2);
      float w = (std::max)(float(0), xx2 - xx1 + 1);
      float h = (std::max)(float(0), yy2 - yy1 + 1);
      float inter = w * h;
      float ovr = inter / (vArea[i] + vArea[j] - inter);
      if (ovr >= NMS_THRESH) {
        input_boxes.erase(input_boxes.begin() + j);
        vArea.erase(vArea.begin() + j);
      } else {
        j++;
      }
    }
  }
}
