#include <Windows.h>

#include <ctime>
#include <iostream>
#include <thread>

#include "fishing.h"
#include "fishnet/nanodet.h"
#include "utils/screenshot.h"

const std::string imgPath = "resource/imgs";

const std::string modelPath = "resource/model";

bool useGPU = true;

int main() {
  printf("Hutao Fisher guide: Alt+V launch fisher; Alt+X stop fisher\n");

  // init
  if (RegisterHotKey(NULL, 0, MOD_ALT,
                     0x56)) {  // 0x56: V do fishing
    printf("Register hotkey Alt+V success!\n");
  } else {
    printf("Register hotkey Alt+V fail\n");
  }
  if (RegisterHotKey(NULL, 1, MOD_ALT,
                     0x58)) {  // 0x58: X, exit fishing
    printf("Register hotkey Alt+X success!\n");
  } else {
    printf("Register hotkey Alt+X fail\n");
  }
  // // run tests
  // if (RegisterHotKey(NULL, 2, MOD_ALT,
  //                    0x41)) {  // 0x41: A, test
  //   printf("Register hotkey Alt+A success!\n");
  // } else {
  //   printf("Register hotkey Alt+A fail\n");
  // }

  printf("please choose whether to use GPU to infer: 1-Y 0:N          ");
  std::cin >> useGPU;

  NanoDet fishnet((modelPath + "/nanodet-fish_mod.param").c_str(),
                  (modelPath + "/nanodet-fish_mod.bin").c_str(), useGPU);

  Screen screen;
  Fisher fisher(&fishnet, &screen, imgPath);

  // fishing process

  std::thread fishThread(&Fisher::fishing, std::ref(fisher));
  fishThread.detach();

  // // collect data

  // std::thread fishThread(&Fisher::getRodData, std::ref(fisher));
  // fishThread.detach();

  // check log
  bool logAllImgs, logData;
  printf(
      "please enter whether to enable log all useful images for this fisher "
      "instance: 1-Y 0-N          ");
  std::cin >> logAllImgs;
  printf(
      "please enter whether to log all useful images for this fisher "
      "instance: 1-Y 0-N          ");
  std::cin >> logData;

  fisher.logAllImgs = logAllImgs;
  fisher.logData = logData;

  printf("Hutao Fisher configuration done! Now you can start fishing.\n");

  MSG msg;

  while (GetMessage(&msg, NULL, WM_HOTKEY, WM_HOTKEY)) {
    switch (msg.wParam) {
      case 0:
        printf("Hotkey Alt+V is pressed, fisher launch!\n");
        Beep(C4, 250);
        Beep(E4, 250);
        Beep(G4, 250);
        Beep(C5, 250);
        fisher.working = true;
        break;
      case 1:
        fisher.working = false;
        printf("Hotkey Alt+X is pressed, fisher stop!\n");
        Beep(C5, 250);
        Beep(G4, 250);
        Beep(E4, 250);
        Beep(C4, 250);
        break;
      case 2:
        continue;  // annotate this line to run fisher test
        if (fisher.working) {
          printf("Hotkey Alt+A is pressed, run fisher test!\n");
          // fisher.testing = false;
          // fisher.imgCollect();
          fisher.test();
        } else {
          printf(
              "Hotkey Alt+A is pressed, but fisher isn't "
              "running!\n");
        }
        break;
    }
  }

  return 0;
}
