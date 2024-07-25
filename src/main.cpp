#include <Windows.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <thread>

#include "json.hpp"
using json = nlohmann::json;

#include "config.h"
#include "fishing.h"
#include "yolov8.h"
#include "screenshot.h"

#ifdef RELEASE
const std::string imgPath = "resource/imgs";
const std::string modelPath = "resource/model";
const std::string configPath = "config.json";
#else
const std::string imgPath = "../../resource/imgs";
const std::string modelPath = "../../resource/model";
const std::string configPath = "../../config.json";
#endif

int main() {
#ifdef DATA_COLLECT
  printf("Version: %d.%d.%d_DATA_COLLECT\n", fisher_VERSION_MAJOR,
         fisher_VERSION_MINOR, fisher_VERSION_PATCH);
#else
  printf("Version: %d.%d.%d\n", fisher_VERSION_MAJOR, fisher_VERSION_MINOR,
         fisher_VERSION_PATCH);
#endif
  printf("Hutao Fisher guide: Alt+V launch fisher; Alt+X stop fisher\n");

#ifdef DATA_COLLECT
  printf(
      "##################################################################\n"
      "#                                                                #\n"
      "#    WARNING!    WARNING!    WARNING!    WARNING!    WARNING!    #\n"
      "#                                                                #\n"
      "#    This version of HutaoFisher is compiled for data collec-    #\n"
      "#    tion and test. It cannot function correctly as the          #\n"
      "#    version ready to be released.                               #\n"
      "#                                                                #\n"
      "#    WARNING!    WARNING!    WARNING!    WARNING!    WARNING!    #\n"
      "#                                                                #\n"
      "##################################################################\n");
#endif

  // init
  bool hotkey = true;

  if (RegisterHotKey(NULL, 0, MOD_ALT,
                     0x56)) {  // 0x56: V do fishing
    printf("Register hotkey Alt+V success!\n");
  } else {
    printf("Register hotkey Alt+V fail\n");
    hotkey = false;
  }
  if (RegisterHotKey(NULL, 1, MOD_ALT,
                     0x58)) {  // 0x58: X, exit fishing
    printf("Register hotkey Alt+X success!\n");
  } else {
    printf("Register hotkey Alt+X fail\n");
    hotkey = false;
  }

#ifdef TEST
  if (RegisterHotKey(NULL, 2, MOD_ALT,
                     0x41)) {  // 0x41: A, test
    printf("Register hotkey Alt+A success!\n");
  } else {
    printf("Register hotkey Alt+A fail\n");
    hotkey = false;
  }
#endif

  if (!hotkey) {
    std::cerr << "Error: register hotkey failed! Please check hotkey settings "
                 "of other applications.\n";
    system("pause");
    return 0;
  }

  json config;
  bool useGPU, logAllImgs, logData;

  try {
    std::ifstream f(configPath);
    if (f.fail()) {
      std::cerr << "Error: load config.json failed!\n";
      system("pause");
      return 0;
    }
    config = json::parse(f);
    f.close();

    const std::vector<std::string> fishNames{
        "medaka",      "large_medaka",      "stickleback",
        "koi",         "butterflyfish",     "pufferfish",
        "formalo_ray", "divda_ray",         "angler",
        "axe_marlin",  "heartfeather_bass", "maintenance_mek"};

    std::cout << "Configurations:\n";
    std::cout << std::setw(32) << std::left
              << "  Use GPU to infer: " << config["useGPU"] << '\n';
    useGPU = config["useGPU"];

    std::cout << std::setw(32) << std::left
              << "  Log all useful images: " << config["logAllImgs"] << '\n';
    logAllImgs = config["logAllImgs"];

    std::cout << std::setw(32) << std::left
              << "  Log rod data: " << config["logData"] << '\n';
    logData = config["logData"];

    std::vector<std::string> fishYes, fishNo;
    for (int i = 0; i < FISH_CLASS_NUM; i++) {
      if (config["typeToFish"][fishNames[i]]) {
        fishYes.push_back(fishNames[i]);
      } else {
        fishNo.push_back(fishNames[i]);
      }
    }

    if (fishYes.empty()) {
      std::cerr << "Error: no fish is configured to be caught!";
      system("pause");
      return 0;
    } else {
      std::cout << "  Types of fish to be caught:\n";
      for (std::vector<std::string>::iterator i = fishYes.begin();
           i < fishYes.end(); i++) {
        std::cout << "    " + *i + "\n";
      }
    }
    if (!fishNo.empty()) {
      std::cout << "  Types of fish not to be caught:\n";
      for (std::vector<std::string>::iterator i = fishNo.begin();
           i < fishNo.end(); i++) {
        std::cout << "    " + *i + "\n";
      }
    }

  } catch (const nlohmann::json_abi_v3_11_2::detail::parse_error& e) {
    std::cerr << e.what() << '\n';
    std::cerr
        << "Error: the structure config.json cannot be parsed, please check!\n";
    system("pause");
    return 0;
  } catch (const nlohmann::json_abi_v3_11_2::detail::type_error& e) {
    std::cerr << e.what() << '\n';
    std::cerr
        << "Error: wrong key or value exists in config.json, please check!\n";
    system("pause");
    return 0;
  }

  YOLOV8 fishnet((modelPath + "/yolov8-fish.param").c_str(),
                  (modelPath + "/yolov8-fish.bin").c_str(), useGPU);

  Screen screen;
  Fisher fisher(&fishnet, &screen, imgPath, config);

  // fishing process

#ifdef TEST
  std::thread fishThread(&Fisher::getRodData, std::ref(fisher));
#else
  std::thread fishThread(&Fisher::fishing, std::ref(fisher));
#endif
  fishThread.detach();

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
#ifdef TEST
      case 2:
        if (fisher.working) {
          printf("Hotkey Alt+A has been pressed, run fisher test!\n");
          fisher.testing = false;
        } else {
          printf(
              "Hotkey Alt+A has been pressed, but fisher isn't "
              "running!\n");
        }
#endif
    }
  }

  return 0;
}
