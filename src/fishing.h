#include <Windows.h>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>

#include "json.hpp"
using json = nlohmann::json;

#include "config.h"
#include "rodnet.h"
#include "screenshot.h"
#include "writer.h"
#include "yolov8.h"

// music notes! qwq
#define A3 220
#define C4 262
#define D4 293
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define B4 494
#define C5 523

const std::vector<std::string> fishNames{"medaka",
                                         "large_medaka",
                                         "stickleback",
                                         "koi",
                                         "butterflyfish",
                                         "pufferfish",
                                         "ray",
                                         "angler",
                                         "axe_marlin",
                                         "heartfeather_bass",
                                         "maintenance_mek"};

class Fisher {
 private:
  const int processShape[2] = {1024, 576};
  bool processWithInputShape;
#ifdef RELEASE
  const std::string logPath = "log";
#else
  const std::string logPath = "..\\..\\log";
#endif

  const double steps[40][2] = {
      {0, 2},    {0, 2},    {0, 2},    {0, 2},    {0, -1},   {0, -1},
      {0, -1},   {0, -1},   {1, 0},    {1, 0},    {1, 0},    {1, 0},
      {1, 0},    {1, 0},    {1, 0},    {1, 0},    {1, 0},    {1, 0},
      {0, -1},   {1, 0},    {1, 0},    {1, 0},    {1, 0},    {1, 0},
      {1, 0},    {1, 0},    {1, 0},    {1, 0},    {1, 0},    {0, -1},
      {0.75, 0}, {0.75, 0}, {0.75, 0}, {0.75, 0}, {0.75, 0}, {0.75, 0},
      {0.75, 0}, {0.75, 0}, {0.75, 0}, {0.75, 0}};
  int MaxFailNum = 3;
  int MaxRodTries = 25;
  int MaxThrowFailNum = 5;  // at most 5 times miss fish can be tolerated
  // unit: second
  double MaxThrowWaiting = 3;
  double MaxBiteWaiting[FISH_CLASS_NUM] = {
      8,    8.5, 9.5, 10.5, 8.5, 11.5,
      11.5, 9.5, 8.5, 10,   10};  // index is fish label
  double MaxControlWaiting = 3;

  bool typeToFish[FISH_CLASS_NUM];
  bool logAllImgs;
  bool logData;

  cv::Mat hookImg, pullImg, centralBarImg, centralBarMask, leftEdgeImg,
      cursorImg, rightEdgeImg, leftEdgeMask, rightEdgeMask;
  cv::Mat baitImgs[BAIT_CLASS_NUM];

  YOLOV8 *fishNet;
  //   RodNet *rodNet;
  Screen *screen;
  float fishnetRatio[2];  // the ratio between process and fishnet's input size

  int bait;  //-1: undetermined bait

  std::mt19937 random_engine;
  Writer writer;

  cv::Mat screenImage;
  std::vector<BoxInfo> bboxes;
  BoxInfo targetFish;
  BoxInfo rod;

  void checkWorking();
  void getBBoxes(bool cover);

  bool scanFish();
  void selectFish();
  void chooseBait();
  void throwRod();
  void checkBite();
  void control();
  int getRodState(BoxInfo rod, BoxInfo fish);

  void imgLog(char name[], bool bbox, int logTime = NULL);
  void errLog();


 public:
  Fisher(YOLOV8 *fishnet, Screen *screen, std::string imgPath, json config);
  ~Fisher();

  bool working;

  void fishing();
};

double bboxDist(BoxInfo rod, BoxInfo fish1, BoxInfo fish2);
