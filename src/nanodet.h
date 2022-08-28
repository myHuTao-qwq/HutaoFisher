//
// Create by RangiLyu
// 2020 / 10 / 2
//

#ifndef NANODET_H
#define NANODET_H

#include <ncnn/net.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "config.h"


const float SCORE_THRESHOLD = 0.5;
const float NMS_THRESHOLD = 0.4;

typedef struct HeadInfo {
  std::string cls_layer;
  std::string dis_layer;
  int stride;
};

struct CenterPrior {
  int x;
  int y;
  int stride;
};

typedef struct BoxInfo {
  float x1;
  float y1;
  float x2;
  float y2;
  float score;
  int label;
} BoxInfo;

struct object_rect {
  int x;
  int y;
  int width;
  int height;
};

const int color_list[80][3] = {
    //{255 ,255 ,255}, //bg
    {216, 82, 24},   {236, 176, 31},  {125, 46, 141},  {118, 171, 47},
    {76, 189, 237},  {238, 19, 46},   {76, 76, 76},    {153, 153, 153},
    {255, 0, 0},     {255, 127, 0},   {190, 190, 0},   {0, 0, 255},
    {0, 255, 0},     {170, 0, 255},   {84, 84, 0},     {84, 170, 0},
    {84, 255, 0},    {170, 84, 0},    {170, 170, 0},   {170, 255, 0},
    {255, 84, 0},    {255, 170, 0},   {255, 255, 0},   {0, 84, 127},
    {0, 170, 127},   {0, 255, 127},   {84, 0, 127},    {84, 84, 127},
    {84, 170, 127},  {84, 255, 127},  {170, 0, 127},   {170, 84, 127},
    {170, 170, 127}, {170, 255, 127}, {255, 0, 127},   {255, 84, 127},
    {255, 170, 127}, {255, 255, 127}, {0, 84, 255},    {0, 170, 255},
    {0, 255, 255},   {84, 0, 255},    {84, 84, 255},   {84, 170, 255},
    {84, 255, 255},  {170, 0, 255},   {170, 84, 255},  {170, 170, 255},
    {170, 255, 255}, {255, 0, 255},   {255, 84, 255},  {255, 170, 255},
    {42, 0, 0},      {84, 0, 0},      {127, 0, 0},     {170, 0, 0},
    {212, 0, 0},     {255, 0, 0},     {0, 42, 0},      {0, 84, 0},
    {0, 127, 0},     {0, 170, 0},     {0, 212, 0},     {0, 255, 0},
    {0, 0, 42},      {0, 0, 84},      {0, 0, 127},     {0, 0, 170},
    {0, 0, 212},     {0, 0, 255},     {0, 0, 0},       {36, 36, 36},
    {72, 72, 72},    {109, 109, 109}, {145, 145, 145}, {182, 182, 182},
    {218, 218, 218}, {0, 113, 188},   {80, 182, 188},  {127, 127, 0},
};

const int NanoDet_InputSize[2] = {576, 1024};

class NanoDet {
 public:
  NanoDet(const char* param, const char* bin, bool useGPU);

  ~NanoDet();

  static NanoDet* detector;
  ncnn::Net* Net;
  static bool hasGPU;
  // modify these parameters to the same with your config if you want to use
  // your own model
  int input_size[2] = {NanoDet_InputSize[0],
                       NanoDet_InputSize[1]};  // input height and width
  int num_class = TOTAL_CLASS_NUM;             // number of classes. 80 for COCO
  int reg_max = 7;  // `reg_max` set in the training config. Default: 7.
  std::vector<int> strides = {8, 16, 32,
                              64};  // strides of the multi-level feature.

  std::vector<BoxInfo> detect(cv::Mat image, float score_threshold,
                              float nms_threshold);

 private:
  void preprocess(cv::Mat& image, ncnn::Mat& in);
  void decode_infer(ncnn::Mat& feats, std::vector<CenterPrior>& center_priors,
                    float threshold,
                    std::vector<std::vector<BoxInfo>>& results);
  BoxInfo disPred2Bbox(const float*& dfl_det, int label, float score, int x,
                       int y, int stride);
  static void nms(std::vector<BoxInfo>& result, float nms_threshold);
};

#endif  // NANODET_H
