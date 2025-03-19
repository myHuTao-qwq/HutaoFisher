#include "fishing.h"

const BoxInfo refBBox = {float(InputSize[1]) / 2 - 16,
                         float(InputSize[0]) / 2 - 9,
                         float(InputSize[1]) / 2 + 16,
                         float(InputSize[0]) / 2 + 9,
                         1,
                         0};  // represent the center of image

/* 0: fruit paste 1: redrot 2: false worm 3: fake fly 4: sugardew 5: sour 6:
 * flashing*/
// index represents (fish_lable - 1)
const int baitList[FISH_CLASS_NUM] = {0, 0, 1, 3, 2, 3, 3, 4, 4, 5, 6};

const std::vector<std::string> typeNames{"rod",
                                         "err_rod",
                                         "medaka",
                                         "large_medaka",
                                         "stickleback",
                                         "koi",
                                         "koi_head",
                                         "butterflyfish",
                                         "pufferfish",
                                         "ray",
                                         "angler",
                                         "axe_marlin",
                                         "heartfeather_bass",
                                         "maintenance_mek"};

const std::map<std::string, int> fishLabels{{"medaka", 2},
                                            {"large_medaka", 3},
                                            {"stickleback", 4},
                                            {"koi", 5},
                                            {"butterflyfish", 7},
                                            {"pufferfish", 8},
                                            {"ray", 9},
                                            {"angler", 10},
                                            {"axe_marlin", 11},
                                            {"heartfeather_bass", 12},
                                            {"maintenance_mek", 13}};

const std::map<int, int> label2fish{{2, 0},  {3, 1},  {4, 2},  {5, 3},
                                    {6, 3},  {7, 4},  {8, 5},  {9, 6},
                                    {10, 7}, {11, 8}, {12, 9}, {13, 10}};

const std::vector<std::string> baitNames{"fruit paste",
                                         "redrot",
                                         "false worm",
                                         "fake fly",
                                         "sugardew",
                                         "sour",
                                         "flashing maintenance mek"};

const int controlColor = 250;  // grayscale

const int progressRingPx[][2] = {{15, 3},
                                 {19, 4},
                                 {22, 6},
                                 {24, 9},
                                 {25, 12},
                                 //  {25, 16},
                                 //  {24, 19},
                                 //  {22, 22},
                                 {19, 24},
                                 {15, 25},
                                 {12, 25},
                                 {8, 24},
                                 {5, 22},
                                 {3, 19},
                                 {2, 16},
                                 {2, 12},
                                 {3, 9},
                                 {5, 6},
                                 {8, 4},
                                 {12, 3}};

std::pair<double, double> xy;

inline void mouseEvent(DWORD dWflags, double dx, double dy) {
  Sleep(20);  // motherfucker why? but without sleeping mouse_event goes wrong
  mouse_event(dWflags, DWORD(dx), DWORD(dy), 0, 0);
  return;
}

inline void mouseEvent(DWORD dWflags, std::pair<double, double> xy) {
  mouseEvent(dWflags, xy.first, xy.second);
  return;
}

double bboxDist(BoxInfo rod, BoxInfo fish1,
                BoxInfo fish2) {  //"rectified" distance
  double ratio = (rod.x2 - rod.x1) / (rod.y2 - rod.y1);
  return sqrt(
      pow(((fish1.x1 + fish1.x2) - (fish2.x1 + fish2.x2)) / 2, 2) +
      pow(((fish1.y1 + fish1.y2) - (fish2.y1 + fish2.y2)) * ratio / 2, 2));
}

bool bboxEqual(BoxInfo box1, BoxInfo box2) {
  return box1.label == box2.label && box1.x1 == box2.x1 && box1.x2 == box2.x2 &&
         box1.y1 == box2.y1 && box1.y2 == box2.y2;
}

double bboxDist(BoxInfo rod, BoxInfo fish) { return bboxDist(rod, rod, fish); }

double colorDiff(const int refColor[], cv::Vec3b screenColor) {
  return sqrt(pow(refColor[0] - screenColor[0], 2) +
              pow(refColor[1] - screenColor[1], 2) +
              pow(refColor[2] - screenColor[2], 2));
}

cv::Rect bbox2ROI(BoxInfo box, const int *boundary) {
  int x, y, w, h;
  int pad = 10;
  x = std::max(int(floor(box.x1)) - pad, 0);
  y = std::max(int(floor(box.y1)) - pad, 0);
  w = std::min(int(ceil(box.x2)) + pad, boundary[0]) - x;
  h = std::min(int(ceil(box.y2)) + pad, boundary[1]) - y;
  return cv::Rect(x, y, w, h);
}

cv::Mat draw_bboxes(const cv::Mat &bgr, const std::vector<BoxInfo> &bboxes,
                    object_rect effect_roi) {
  cv::Mat image;

  image = bgr.clone();  // it looks that cvtColor makes error now?

  int src_w = image.cols;
  int src_h = image.rows;
  int dst_w = effect_roi.width;
  int dst_h = effect_roi.height;
  float width_ratio = (float)src_w / (float)dst_w;
  float height_ratio = (float)src_h / (float)dst_h;

  for (size_t i = 0; i < bboxes.size(); i++) {
    const BoxInfo &bbox = bboxes[i];
    cv::Scalar color =
        cv::Scalar(color_list[bbox.label][0], color_list[bbox.label][1],
                   color_list[bbox.label][2]);

    cv::rectangle(
        image,
        cv::Rect(cv::Point(int((bbox.x1 - effect_roi.x) * width_ratio),
                           int((bbox.y1 - effect_roi.y) * height_ratio)),
                 cv::Point(int((bbox.x2 - effect_roi.x) * width_ratio),
                           int((bbox.y2 - effect_roi.y) * height_ratio))),
        color);

    char text[256];
    sprintf_s(text, "%s %.1f%%", typeNames[bbox.label].c_str(),
              bbox.score * 100);

    int baseLine = 0;
    cv::Size label_size =
        cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    int x = int((bbox.x1 - effect_roi.x) * width_ratio);
    int y = int((bbox.y1 - effect_roi.y) * height_ratio - label_size.height -
                baseLine);
    if (y < 0) y = 0;
    if (x + label_size.width > image.cols) x = image.cols - label_size.width;

    cv::rectangle(
        image,
        cv::Rect(cv::Point(x, y),
                 cv::Size(label_size.width, label_size.height + baseLine)),
        color, -1);

    cv::putText(image, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
  }

  return image;
}

class fishingException : public std::exception {
 public:
  using std::exception::exception;
};

class fisherShutdown {};

Fisher::Fisher(YOLOV8 *fishnet, Screen *screen, std::string imgPath,
               json config) {
  this->working = false;
  this->fishNet = fishnet;
  // this->rodNet = rodnet;
  this->screen = screen;

  Writer writer;

  std::random_device seed;
  random_engine = std::mt19937(seed());

  processWithInputShape = (processShape[0] == fishNet->input_size[1] &&
                           processShape[1] == fishNet->input_size[0]);
  fishnetRatio[0] = float(fishNet->input_size[1]) / processShape[0];
  fishnetRatio[1] = float(fishNet->input_size[0]) / processShape[1];

  hookImg = cv::imread(imgPath + "/hook.png", cv::IMREAD_GRAYSCALE);
  pullImg = cv::imread(imgPath + "/pull.png", cv::IMREAD_GRAYSCALE);
  centralBarImg = cv::imread(imgPath + "/centralBar.png", cv::IMREAD_GRAYSCALE);
  centralBarMask =
      cv::imread(imgPath + "/centralBarMask.png", cv::IMREAD_GRAYSCALE);
  cursorImg = cv::imread(imgPath + "/cursor.png", cv::IMREAD_GRAYSCALE);
  leftEdgeImg = cv::imread(imgPath + "/leftEdge.png", cv::IMREAD_GRAYSCALE);
  rightEdgeImg = cv::imread(imgPath + "/rightEdge.png", cv::IMREAD_GRAYSCALE);
  leftEdgeMask =
      cv::imread(imgPath + "/leftEdgeMask.png", cv::IMREAD_GRAYSCALE);
  rightEdgeMask =
      cv::imread(imgPath + "/rightEdgeMask.png", cv::IMREAD_GRAYSCALE);

  for (int i = 0; i < BAIT_CLASS_NUM; i++) {
    baitImgs[i] = cv::imread(imgPath + "/" + baitNames[i] + ".png");
  }

  logAllImgs = config["logAllImgs"];
  logData = config["logData"];

  system(("if not exist " + logPath + "\\images mkdir " + logPath + "\\images")
             .c_str());
  if (logData) {
    std::fstream data;
    data.open(logPath + "\\data.csv", std::ios::in);
    if (!data.good()) {
      std::cout << "log file does not exist -- create log file." << std::endl;
      data.open(logPath + "\\data.csv", std::ios::out);
      data << "timestamp,bite_time,fish_label,success,fish_x1,fish_x2,fish_y1,"
              "fish_y2,rod_x1,rod_x2,rod_y1,rod_y2"
           << std::endl;
    }
    data.close();
  }

  for (int i = 0; i < FISH_CLASS_NUM; i++) {
    typeToFish[i] = config["typeToFish"][fishNames[i]];
  }

  MaxFailNum = config["MaxFailNum"];
  MaxRodTries = config["MaxRodTries"];
  MaxThrowFailNum = config["MaxThrowFailNum"];
  MaxThrowWaiting = config["MaxThrowWaiting"];
  for (int i = 0; i < FISH_CLASS_NUM; i++) {
    MaxBiteWaiting[i] = config["MaxBiteWaiting"][fishNames[i]];
  }
  MaxControlWaiting = config["MaxControlWaiting"];

  targetFish.label = -1;
  bait = -1;

  return;
}

Fisher::~Fisher() { return; }

inline void Fisher::checkWorking() {
  if (!working) {
    throw fisherShutdown();
  }
  return;
}

inline void cancelThrowRod(bool err) {
  mouseEvent(MOUSEEVENTF_LEFTUP, 0, 0);
  Sleep(500);
  if (!err) {
    Sleep(2000);
    mouseEvent(MOUSEEVENTF_LEFTDOWN, 0, 0);
    mouseEvent(MOUSEEVENTF_LEFTUP, 0, 0);
  }
  return;
}

void Fisher::getBBoxes(bool cover) {
  checkWorking();
  screenImage = screen->getScreenshot();
  cv::Mat resized_img;
  cv::resize(screenImage, resized_img,
             cv::Size(fishNet->input_size[1], fishNet->input_size[0]));
  if (cover) {  // cover the latest caught fish on the screen
    cv::rectangle(resized_img, cv::Rect(56, 288, 31, 52),
                  cv::Scalar(255, 255, 255), -1);
  }
  checkWorking();

  std::vector<BoxInfo> rawBBoxes, modBBoxes;
  rawBBoxes = fishNet->detect(resized_img, SCORE_THRESHOLD, NMS_THRESHOLD);
  for (auto i = rawBBoxes.begin(); i < rawBBoxes.end(); i++) {
    if (i->label < NON_FISH_CLASS_NUM || typeToFish[label2fish.at(i->label)]) {
      modBBoxes.push_back(*i);
    }
  }
  bboxes = modBBoxes;

  if (!processWithInputShape) {
    for (std::vector<BoxInfo>::iterator i = bboxes.begin(); i < bboxes.end();
         i++) {
      i->x1 *= fishnetRatio[0];
      i->x2 *= fishnetRatio[0];
      i->y1 *= fishnetRatio[1];
      i->y2 *= fishnetRatio[1];
    }
  }

  // #ifdef DATA_COLLECT
  //   // 1/50 probility to randomly record image
  //   std::uniform_int_distribution<int> intDist(1, 50);
  //   if (intDist(random_engine) == 1) {
  //     imgLog("random", true);
  //   }
  // #endif

  checkWorking();
  return;
}

int Fisher::getRodState(BoxInfo rod, BoxInfo fish) {
  // #ifdef DATA_COLLECT
  //   // 1/2 probility to make the fisher beleive the rod is in proper
  //   position,
  //   // ignoring whether it's a real proper position
  //   std::uniform_int_distribution<int> intDist(0, 1);
  //   if (intDist(random_engine) == 0) {
  //     return 0;
  //   }
  // #endif
  rodInput input;
  input.rod_x1 = rod.x1;
  input.rod_x2 = rod.x2;
  input.rod_y1 = rod.y1;
  input.rod_y2 = rod.y2;
  input.fish_x1 = fish.x1;
  input.fish_x2 = fish.x2;
  input.fish_y1 = fish.y1;
  input.fish_y2 = fish.y2;
  input.fish_label = label2fish.at(fish.label);
  int state = _getRodState(input);

#ifdef DATA_COLLECT
  if (state == 1) state = 0;
#endif

  return state;
}

bool Fisher::scanFish() {
  Beep(C4, 250);
  Beep(D4, 250);
  for (int i = 0; i < 1; i++) {     // try 1 times (maybe can be more)
    for (int j = 0; j < 40; j++) {  // move camera view
      checkWorking();
      getBBoxes(true);
      if (bboxes.empty()) {  // found no fish
        mouseEvent(MOUSEEVENTF_MOVE, int(650 * steps[j][0]),
                   int(800 * steps[j][1]));
        Sleep(150);
      } else {
        printf("    scan fish: find fish!\n");
        return true;  // found fish
      }
    }
  }
#ifdef DATA_COLLECT
  imgLog("scan", false);
#endif
  return false;
}

void Fisher::selectFish() {
  Beep(C4, 250);
  Beep(E4, 250);
  checkWorking();
  if (bboxes.empty()) {  // only can be triggered in test
    throw fishingException("select fish error: there is no fish in screen!");
    // return;
  }
  // std::sort(bboxes.begin(), bboxes.end(), [](BoxInfo bbox1, BoxInfo bbox2) {
  //   return bboxDist(refBBox, bbox1) < bboxDist(refBBox, bbox2);
  // });  // sort the bboxes with bboxDist; just select a fish, take it easy
  std::sort(bboxes.begin(), bboxes.end(), [](BoxInfo bbox1, BoxInfo bbox2) {
    return bbox1.score > bbox2.score;
  });  // sort the bboxes with confidence
  if (bboxes[0].label == 0) {
    cancelThrowRod(false);
    throw fishingException(
        "select fish error: there should be no rod in screen!");
  } else if (bboxes[0].label == 1) {
    cancelThrowRod(true);
    throw fishingException(
        "select fish error: there should be no err_rod in screen!");
  }

  targetFish = bboxes[0];

  if (targetFish.label == 5) {
    targetFish.label = 6;
  }

#ifdef DATA_COLLECT
  imgLog("select", true);
#endif
  BoxInfo lastTargetFish;

  float dx = 0, dy = 0;

  dx = (targetFish.x1 + targetFish.x2 - refBBox.x1 - refBBox.x2) / 2;
  dy = (targetFish.y1 + targetFish.y2 - refBBox.y1 - refBBox.y2) / 2;

  for (int round = 0; round < 10; round++) {
    lastTargetFish = targetFish;
    checkWorking();
    getBBoxes(true);
    Sleep(20);
    std::vector<BoxInfo> targetFishes;
    for (std::vector<BoxInfo>::iterator i = bboxes.begin(); i < bboxes.end();
         i++) {
      if (i->label == targetFish.label) {
        targetFishes.push_back(*i);
      }
    }

    if (targetFishes.empty()) {
      mouseEvent(MOUSEEVENTF_MOVE, dx / 2, dy / 1.5);
      continue;
      break;
    }

    targetFish = *std::min_element(
        targetFishes.begin(), targetFishes.end(),
        [lastTargetFish](BoxInfo bbox1, BoxInfo bbox2) {
          return bboxDist(refBBox, lastTargetFish, bbox1) <
                 bboxDist(refBBox, lastTargetFish, bbox2);
        });  // find the fish closest to where the target fish last appears

    if (bboxDist(refBBox, targetFish) < 100) {
      break;
    } else {
      dx = (targetFish.x1 + targetFish.x2 - refBBox.x1 - refBBox.x2) / 2;
      dy = (targetFish.y1 + targetFish.y2 - refBBox.y1 - refBBox.y2) / 2;
      checkWorking();
      mouseEvent(MOUSEEVENTF_MOVE, dx / 2, dy / 1.5);
    }
  }

  std::cout << "    select fish: " << fishNames[label2fish.at(targetFish.label)]
            << std::endl;
  return;
}

void Fisher::chooseBait() {
  Beep(C4, 250);
  Beep(F4, 250);
  checkWorking();

  if (baitList[label2fish.at(targetFish.label)] == bait) {
    return;
  } else {
    // all the errors will reset bait to -1, so there should be no bug here?
    bait = baitList[label2fish.at(targetFish.label)];
  }

  mouseEvent(MOUSEEVENTF_RIGHTDOWN, 0, 0);
  mouseEvent(MOUSEEVENTF_RIGHTUP, 0, 0);

  xy = this->screen->getAbsPos(23333, 45965);
  mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE,
             xy);  // move the mouse away from the baits to prevent
                   // mismatching, 23333 45965 is the pos of the cancel button

  Sleep(300);  // wait for the window to pop

  cv::Mat baitScreenshot;
  screenImage = screen->getScreenshot();
  cv::resize(screenImage, baitScreenshot,
             cv::Size(processShape[0], processShape[1]));

  cv::Mat score;
  double minScore;
  cv::Point minIdx;

  checkWorking();

  cv::matchTemplate(baitScreenshot, baitImgs[bait], score,
                    cv::TM_SQDIFF);  // just scanning over the whole screen in
                                     // fact doesn't cost much time (~30ms).....
  cv::minMaxLoc(score, &minScore, nullptr, &minIdx, nullptr);

  std::cout << "bait template diff: " << minScore << "/200000" << std::endl;

  if (minScore > 2e6) {  // not found, 2e6 is an empirical threshold
    // click cancel button
    xy = this->screen->getAbsPos(23333, 45965);
    mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, xy);
    mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, xy);
    throw fishingException("choose bait error: cannot find a proper bait!");
  }

  // click the position of bait
  checkWorking();
  xy = this->screen->getAbsPos(
      double(minIdx.x + 29) / double(processShape[0]) * 65535,
      double(minIdx.y + 29) / double(processShape[1]) * 65535);
  mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, xy);
  mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, xy);
  mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, xy);

  // click confirm button, 44543 45965 is its position
  checkWorking();
  Sleep(300);
  xy = this->screen->getAbsPos(44543, 45965);
  mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, xy);
  mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, xy);
  mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, xy);

  // check whether succeeed to exit (if click the previous selected bait it will
  // pop a mask so we need to click again)
  checkWorking();
  Sleep(300);
  const int backgroundColor[3] = {86, 69, 62};  // BGR
  cv::resize(screen->getScreenshot(), baitScreenshot,
             cv::Size(processShape[0], processShape[1]));
  cv::Vec3b &screencolor = baitScreenshot.at<cv::Vec3b>(cv::Point(700, 268));

  checkWorking();
  if (colorDiff(backgroundColor, screencolor) < 10) {
    xy = this->screen->getAbsPos(44543, 45965);
    mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, xy);
    mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, xy);
    mouseEvent(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, xy);
    // final check
    Sleep(300);
    cv::resize(screen->getScreenshot(), baitScreenshot,
               cv::Size(processShape[0], processShape[1]));
    screencolor = baitScreenshot.at<cv::Vec3b>(cv::Point(700, 268));
    if (colorDiff(backgroundColor, screencolor) < 10) {
      fishingException("choose bait error: cannot exit the select window!");
    }
  }

  std::cout << "    choose bait: select " << baitNames[bait] << " success!"
            << std::endl;
  return;
}

void Fisher::throwRod() {
  Beep(C4, 250);
  Beep(G4, 250);
  checkWorking();
  mouseEvent(MOUSEEVENTF_LEFTDOWN, 0, 0);  // prepare to throw
  Sleep(1000);                             // wait for the rod to appear
  checkWorking();

  const double pi = 3.14159265358979323846;
  std::uniform_real_distribution<> angDistrib(0, 2 * pi);

  // try to move the rod to a proper position
  int fishFailNum = 0;
  float dx = 0, dy = 0, dl = 0;

  std::vector<BoxInfo> targetFishes, rods;

  for (int i = 0; i < MaxRodTries; i++) {
    printf("        throw rod: round %d starts", i);
    Beep(C5, 250);
    checkWorking();
    getBBoxes(false);

    BoxInfo lastTargetFish = targetFish;
    targetFishes.clear();
    rods.clear();

    for (auto i = bboxes.begin(); i < bboxes.end(); i++) {
      if (i->label == 0) {
        rods.push_back(*i);
      } else if (i->label == targetFish.label) {
        targetFishes.push_back(*i);
      }
    }

    if (rods.empty()) {
      std::vector<BoxInfo> err_rods, fishes;
      for (std::vector<BoxInfo>::iterator i = bboxes.begin(); i < bboxes.end();
           i++) {
        if (i->label == 1) {
          err_rods.push_back(*i);
        } else {  // notice that there are no rods in bboxes
          fishes.push_back(*i);
        }
      }
      if (fishFailNum == MaxThrowFailNum) {
        if (err_rods.empty()) {
          cancelThrowRod(false);
          printf("\n");
          throw fishingException("throw rod error: find no rod!");
        } else if (fishes.empty()) {
          cancelThrowRod(true);
          printf("\n");
          throw fishingException(
              "throw rod error: rod position is unacceptably wrong!");
        }
      } else {
        if (err_rods.empty() || fishes.empty()) {
          printf("        find no rod!\n");
#ifdef DATA_COLLECT
          imgLog("err_rod", true);
#endif
          fishFailNum++;
          // random move to try to rediscover the rod and fish
          double ang = angDistrib(random_engine);
          mouseEvent(MOUSEEVENTF_MOVE, 160 * cos(ang), 45 + 90 * sin(ang));
        } else {
          printf("        the position of rod is out of range!\n");
          BoxInfo err_rod, farthestFish;
          err_rod = *std::max_element(err_rods.begin(), err_rods.end(),
                                      [](BoxInfo bbox1, BoxInfo bbox2) {
                                        return bbox1.score < bbox2.score;
                                      });  // find the most confident err_rod;

          farthestFish = *std::max_element(
              fishes.begin(), fishes.end(),
              [err_rod](BoxInfo bbox1, BoxInfo bbox2) {
                return bboxDist(err_rod, bbox1) < bboxDist(err_rod, bbox2);
              });  // find the fish farthest to err_rod

          dx =
              (farthestFish.x1 + farthestFish.x2 - err_rod.x1 - err_rod.x2) / 2;
          dy =
              (farthestFish.y1 + farthestFish.y2 - err_rod.y1 - err_rod.y2) / 2;

          mouseEvent(MOUSEEVENTF_MOVE, dx, dy);
        }
        continue;
      }
    }

    if (targetFishes.empty()) {
      if (fishFailNum == MaxThrowFailNum) {
        cancelThrowRod(false);
        printf("\n");
        throw fishingException(
            "throw rod error: cannot find target fish within acceptable "
            "tries!");
      } else {
        printf("        find no target fish!\n");
#ifdef DATA_COLLECT
        imgLog("targetFish", true);
#endif
        fishFailNum++;
        // random move to try to retrieve the fish
        double ang = angDistrib(random_engine);
        mouseEvent(MOUSEEVENTF_MOVE, dx / 2, dy / 2);
        mouseEvent(MOUSEEVENTF_MOVE, 160 * cos(ang), 90 * sin(ang));
        continue;
      }
    }
    fishFailNum = 0;  // find the fish and clear fishFailNum

    rod = *std::max_element(rods.begin(), rods.end(),
                            [](BoxInfo bbox1, BoxInfo bbox2) {
                              return bbox1.score < bbox2.score;
                            });  // find the most confident rod;

    targetFish = *std::min_element(
        targetFishes.begin(), targetFishes.end(),
        [this, lastTargetFish](BoxInfo bbox1, BoxInfo bbox2) {
          return bboxDist(rod, lastTargetFish, bbox1) <
                 bboxDist(rod, lastTargetFish, bbox2);
        });  // find the fish closest to where the target fish last appears

    dx = (targetFish.x1 + targetFish.x2 - rod.x1 - rod.x2) / 2;
    dy = (targetFish.y1 + targetFish.y2 - rod.y1 - rod.y2) / 2;

    checkWorking();

    switch (getRodState(rod, targetFish)) {
      case 0:  // the rod is at a proper position
        printf(
            "\n    throw rod: the rod move to a proper position after %d "
            "steps!\n",
            i + 1);
        mouseEvent(MOUSEEVENTF_LEFTUP, 0, 0);
        return;
      case 1:  // too close
        printf("        too close!\n");
        dl = sqrt(dx * dx + dy * dy);
        if (dl < 30) {  // set a minimum step
          dx = dx / dl * 30;
          dy = dy / dl * 30;
        }
        mouseEvent(MOUSEEVENTF_MOVE, -dx / 1.5, -dy * 1.5);
        break;
      case 2:  // too far
        printf("        too far!\n");
        mouseEvent(MOUSEEVENTF_MOVE, dx / 1.5, dy * 1.5);
        break;
    }
  }
  cancelThrowRod(false);
  throw fishingException(
      "throw rod error: cannot reach a proper position within iteration "
      "limit!");
  return;
}

void Fisher::checkBite() {
  Beep(C4, 250);
  Beep(A4, 250);

  bool throwRodSuccess = false, biteSuccess = false;

  clock_t startTime = clock();

  cv::Mat gray, resized, edge;

  while (double(clock() - startTime) / CLOCKS_PER_SEC < MaxThrowWaiting) {
    checkWorking();
    cv::Mat throwImage = screen->getScreenshot();
    cv::cvtColor(throwImage, gray, cv::COLOR_BGR2GRAY);
    cv::resize(gray, resized, cv::Size(processShape[0], processShape[1]));
    // the position of hook icon
    cv::Canny(resized(cv::Rect(839, 508, 41, 41)), edge, 200, 400);
    if (cv::PSNR(edge, hookImg) > 10) {
      throwRodSuccess = true;
      break;
    }
  }

  if (throwRodSuccess) {
    double throwTime = double(clock() - startTime) / CLOCKS_PER_SEC;
    printf(
        "    checkBite: the float splashes into the water after %lf seconds!\n",
        throwTime);
    startTime = clock();
  } else {
    screenImage = screen->getScreenshot();
    throw fishingException(
        "checkBite: the float doesn't splash within acceptable time!");
  }

  double biteTime = 0;
  while (double(clock() - startTime) / CLOCKS_PER_SEC <
         MaxBiteWaiting[label2fish.at(targetFish.label)]) {
    checkWorking();
    cv::Mat biteImage = screen->getScreenshot();
    cv::cvtColor(biteImage, gray, cv::COLOR_BGR2GRAY);
    cv::resize(gray, resized, cv::Size(processShape[0], processShape[1]));
    // the position of hook icon too
    cv::Canny(resized(cv::Rect(839, 508, 41, 41)), edge, 100, 200);
    if (cv::PSNR(edge, pullImg) > 8.5) {  // a carefully selected value :)
      biteSuccess = true;
      biteTime = double(clock() - startTime) / CLOCKS_PER_SEC;
      break;
    }
  }

  mouseEvent(MOUSEEVENTF_LEFTDOWN, 0, 0);
  mouseEvent(MOUSEEVENTF_LEFTUP, 0, 0);

#ifdef DATA_COLLECT
  if (logData) {
    int biteState = -1;
    bool record = true;  // to guarentee the recorded fish is the biting fish

    for (auto i = bboxes.begin(); i < bboxes.end(); i++) {
      if (i->label >= NON_FISH_CLASS_NUM && !bboxEqual(targetFish, *i) &&
          baitList[label2fish.at(targetFish.label)] ==
              baitList[label2fish.at(i->label)]) {
        if (i->label == 5 && targetFish.label == 6) {  // koi and koi_head
          continue;
        } else {
          record = false;
          break;
        }
      }
    }

    if (record) {
      if (biteSuccess) {
        biteState = 0;
      } else {
        Beep(C4, 250);
        Beep(A3, 250);
        printf(
            "enter fail reason: 0-succeed, 1-too close, 2-too far, other-don't "
            "save\n");
        std::cin >> biteState;
      }
      if (biteState == 0 || biteState == 1 || biteState == 2) {
        int logTime = int(time(0));

        std::ofstream data;
        data.open(logPath + "\\data.csv", std::ios::app);
        char output[1024];
        sprintf_s(output, "%d, %f, %d, %d, %f, %f, %f, %f, %f, %f, %f, %f",
                  logTime, biteTime, label2fish.at(targetFish.label), biteState,
                  targetFish.x1, targetFish.x2, targetFish.y1, targetFish.y2,
                  rod.x1, rod.x2, rod.y1, rod.y2);
        data << output << std::endl;
        data.close();

        cv::Mat resized;
        cv::resize(screenImage, resized,
                   cv::Size(processShape[0], processShape[1]));
        cv::Mat roiImage(processShape[1], processShape[0], resized.type(),
                         cv::Scalar(0));
        resized(bbox2ROI(rod, processShape))
            .copyTo(roiImage(bbox2ROI(rod, processShape)));
        resized(bbox2ROI(targetFish, processShape))
            .copyTo(roiImage(bbox2ROI(targetFish, processShape)));

        char filename[256];
        sprintf_s(filename, "%s\\images\\%d_bite.png", logPath.c_str(),
                  logTime);

        writer.addImage(filename, roiImage);

        imgLog("bite", true, logTime);
      }
    }
  }
#endif

  if (biteSuccess) {
    printf("    checkBite: the fish gets hooked after %lf seconds!\n",
           biteTime);
  } else {
    throw fishingException(
        "checkBite: the fish didn't get hooked within an acceptable time!");
  }
  return;
}

void Fisher::control() {
  Beep(C4, 250);
  Beep(B4, 250);

  int yBase;
  cv::Mat gray, resized, cutted, score;
  clock_t startTime = clock();
  double minScore;
  cv::Point maxIdx, minIdx;

  while (double(clock() - startTime) / CLOCKS_PER_SEC < MaxControlWaiting) {
    checkWorking();
    screenImage = screen->getScreenshot();
    cv::cvtColor(screenImage, gray, cv::COLOR_BGR2GRAY);
    cv::resize(gray, resized, cv::Size(processShape[0], processShape[1]));
    //(504,0,16,216) is the possible position of progress ring
    cv::matchTemplate(resized(cv::Rect(504, 0, 16, 216)), centralBarImg, score,
                      cv::TM_SQDIFF, centralBarMask);
    cv::minMaxLoc(score, &minScore, nullptr, &minIdx, nullptr);

    if (minScore < 5e3) {
      yBase = minIdx.y;
      std::cout << "control bar match score: " << minScore
                << "/5000, yBase: " << yBase << std::endl;
      break;
    }
  }

#ifdef DATA_COLLECT
  time_t logTime = time(0);

  char filename[256];
  sprintf_s(filename, "%s/images/%d_%s_yBase=%d.png", logPath.c_str(),
            int(logTime), "match_controlbar", yBase);
  writer.addImage(filename, resized(cv::Rect(504, 0, 16, 216)));
#endif

  if (minScore >= 5e3) {
    throw fishingException("control error: control box didn't appear!");
  }

  int cursorPos, leftEdgePos, rightEdgePos;
  int progress = 0, lastProgress = 0;
  bool start = false, end = false;
  double cursorScore, leftScore, rightScore;

  int lastLeftEdgePos = -1, lastRightEdgePos = -1, lastCursorPos = -1;
  bool first = true, matching_err = false;

  bool holding = false;

  cv::Mat controlBox, progressRing;

  while (true) {
    checkWorking();
    screenImage = screen->getScreenshot();

    cv::resize(screenImage, resized,
               cv::Size(processShape[0], processShape[1]));
    cv::cvtColor(resized, gray, cv::COLOR_BGR2GRAY);

    controlBox = gray(cv::Rect(375, yBase, 273, 16));
    checkWorking();

    // find cursor
    cv::matchTemplate(controlBox, cursorImg, score, cv::TM_CCOEFF_NORMED);
    cv::minMaxLoc(score, nullptr, &cursorScore, nullptr, &maxIdx);
    cursorPos = maxIdx.x;

    // check exit condition
    if (cursorScore < 0.55) {
      if (start) {
        if (end) {
          printf("    control: succeed!\n");
        } else {
          time_t logTime = time(0);
          printf(
              "time: %d pos: left: %d-%d %lf cursor: %d-%d %lf right: %d-%d "
              "%lf\n",
              int(logTime), leftEdgePos, lastLeftEdgePos, leftScore, cursorPos,
              lastCursorPos, cursorScore, rightEdgePos, lastRightEdgePos,
              rightScore);

          char filename[256];
          sprintf_s(filename, "%s/images/%d_%s_l%d_c%d_r%d.png",
                    logPath.c_str(), int(logTime), "control_fail", leftEdgePos,
                    cursorPos, rightEdgePos);

          // save colorful controlbox
          writer.addImage(filename, resized(cv::Rect(375, yBase, 273, 16)));
          throw fishingException("control error: control fail!");
        }
        mouseEvent(MOUSEEVENTF_LEFTUP, 0, 0);
        break;
      } else {
        continue;
      }
    }

    start = true;
    matching_err = false;

    // find left edge
    cv::matchTemplate(controlBox, leftEdgeImg, score, cv::TM_SQDIFF,
                      leftEdgeMask);
    cv::minMaxLoc(score, &leftScore, nullptr, &minIdx, nullptr);
    if (leftScore < 1e3) {
      leftEdgePos = minIdx.x;
    } else {
      leftEdgePos = cursorPos;
      matching_err = true;
    }

    // find right edge
    cv::matchTemplate(controlBox, rightEdgeImg, score, cv::TM_SQDIFF,
                      rightEdgeMask);
    cv::minMaxLoc(score, &rightScore, nullptr, &minIdx, nullptr);
    if (rightScore < 1e3) {
      rightEdgePos = minIdx.x;
    } else {
      rightEdgePos = cursorPos;
      matching_err = true;
    }

#ifdef DATA_COLLECT
    if (first) {
      lastLeftEdgePos = leftEdgePos;
      lastRightEdgePos = rightEdgePos;
      lastCursorPos = cursorPos;
      first = false;
    }

    if (matching_err) {
      time_t logTime = time(0);
      printf(
          "time: %d pos: left: %d-%d %lf cursor: %d-%d %lf right: %d-%d %lf\n",
          int(logTime), leftEdgePos, lastLeftEdgePos, leftScore, cursorPos,
          lastCursorPos, cursorScore, rightEdgePos, lastRightEdgePos,
          rightScore);
      std::cout << "warning: recognize control element matching error!"
                << std::endl;
      char filename[256];
      sprintf_s(filename, "%s/images/%d_%s_l%d_c%d_r%d.png", logPath.c_str(),
                int(logTime), "control_matching", leftEdgePos, cursorPos,
                rightEdgePos);

      // save colorful controlbox
      writer.addImage(filename, resized(cv::Rect(375, yBase, 273, 16)));
    } else if ((leftEdgePos - lastLeftEdgePos) > 30 ||
               abs(rightEdgePos - lastRightEdgePos) > 30 ||
               abs(cursorPos - lastCursorPos) > 30) {
      // experience tell me if this happen there is likely a recognition error
      time_t logTime = time(0);
      printf(
          "time: %d pos: left: %d-%d %lf cursor: %d-%d %lf right: %d-%d %lf\n",
          int(logTime), leftEdgePos, lastLeftEdgePos, leftScore, cursorPos,
          lastCursorPos, cursorScore, rightEdgePos, lastRightEdgePos,
          rightScore);
      std::cout << "warning: recognize control element discontinious movement!"
                << std::endl;

      char filename[256];
      sprintf_s(filename, "%s/images/%d_%s_l%d_c%d_r%d.png", logPath.c_str(),
                int(logTime), "control_moving", leftEdgePos, cursorPos,
                rightEdgePos);

      // save colorful controlbox
      writer.addImage(filename, resized(cv::Rect(375, yBase, 273, 16)));
    }

    lastLeftEdgePos = leftEdgePos;
    lastRightEdgePos = rightEdgePos;
    lastCursorPos = cursorPos;
#endif

    checkWorking();

    // check progress

    progressRing = gray(cv::Rect(498, yBase + 23, 28, 29));
    // 20 checkpoints on the ring, but 2 is blocked by stablizer buff
    for (progress = 0; progress < 17; progress++) {
      if (abs(controlColor -
              progressRing.at<uchar>(progressRingPx[progress][1],
                                     progressRingPx[progress][0])) > 20) {
        break;
      }
    }
    if (progress > 5) {
      progress += 3;
    }
    end = (progress >= 18) || end;
    if (progress != lastProgress) {
      printf("        control: progress %d %%\n", progress * 5);
      lastProgress = progress;
    }
    checkWorking();
    // a super simple strategy
    if (double(cursorPos - leftEdgePos) /
            double(rightEdgePos - leftEdgePos + 1) <
        0.4) {
      if (!holding) {
        holding = true;
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
      }
    } else {
      if (holding) {
        holding = false;
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
      }
    }
  }

  Beep(C4, 250);
  Beep(C5, 250);
  return;
}

void Fisher::imgLog(char name[], bool bbox, int logTime) {
  if (logTime == NULL) {
    logTime = int(time(0));
  }

  cv::rectangle(  // cover uid for privacy
      screenImage,
      cv::Rect(int(0.878125 * screen->m_width), int(0.975 * screen->m_height),
               int(0.09375 * screen->m_width), int(0.025 * screen->m_height)),
      cv::Scalar(255, 255, 255), -1);

  char filename[256];
  sprintf_s(filename, "%s\\images\\%d_%s_orig.png", logPath.c_str(), logTime,
            name);
  writer.addImage(filename, screenImage);

  if (bbox) {
    object_rect effect_roi;
    effect_roi.x = 0;
    effect_roi.y = 0;
    effect_roi.width = processShape[0];
    effect_roi.height = processShape[1];
    cv::Mat bboxed_img = draw_bboxes(screenImage, bboxes, effect_roi);
    char bbox_filename[256];
    double ratio = double(screen->m_width) / double(processShape[0]);
    if (targetFish.label != -1) {
      cv::putText(bboxed_img, "target: " + typeNames[targetFish.label],
                  cv::Point(int(50 * ratio), int(50 * ratio)),
                  cv::FONT_HERSHEY_SIMPLEX, ratio, cv::Scalar(0, 0, 255),
                  int(2 * ratio));
    }
    sprintf_s(bbox_filename, "%s\\images\\%d_%s_bbox.jpg", logPath.c_str(),
              logTime, name);
    writer.addImage(bbox_filename, bboxed_img);
  }

  return;
}

void Fisher::errLog() {
  Beep(A3, 500);
  imgLog("err", true);
  Sleep(1000);
  return;
}

void Fisher::fishing() {
  int fishingFailCnt;
  while (true) {
    fishingFailCnt = 0;
    try {
      while (!working) {
        Sleep(100);  // wait for fisher launch
      }
      screen->init();
      while (working && (fishingFailCnt < MaxFailNum) && scanFish()) {
        printf("Fisher: Begin to try to catch a fish!\n");
        try {
          selectFish();
          chooseBait();
          throwRod();
          checkBite();
          control();
          printf("Fisher: A fish has been successfully caught!\n");
          fishingFailCnt = 0;
          /* wait for the icon of last caught fish to disappear, otherwise it
        will disturb the fish recognition in this fishing cycle */
          Sleep(1500);
        } catch (const fishingException &e) {
          fishingFailCnt++;
          bait = -1;
          std::cerr << "    Error occured while fishing: " << e.what() << '\n';
          errLog();
        }
      }
      if (working) {
        if (fishingFailCnt >= MaxFailNum) {
          Sleep(250);
          printf(
              "Fisher: Warning! Fisher failed too many times and has "
              "automatically stopped. Please check.\n");
          Beep(A3, 250);
          Beep(A3, 250);
          Beep(A3, 250);
        } else {
          printf("Fisher: All the fishes in this pool have been caught!\n");
          bait = -1;
          Beep(C5, 250);
          Beep(G4, 250);
          Beep(E4, 250);
          Beep(C4, 250);
        }
        working = false;
      }
    } catch (const screenshotException &e) {
      Sleep(250);
      bait = -1;
      std::cerr << "    Fisher: Fatal screenshot error: " << e.what()
                << " The fisher has automatically stopped.\n";
      working = false;
      Beep(A3, 250);
      Beep(A3, 250);
      Beep(A3, 250);
    } catch (const fisherShutdown) {
      bait = -1;
      std::cerr << "    Fisher: Fishing process was manually aborted!\n";
      continue;
    } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
      system("pause");
      throw(e);
    }
  }

  return;
}
