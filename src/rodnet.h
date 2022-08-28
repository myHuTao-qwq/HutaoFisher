#pragma once

#include <algorithm>
#include <cmath>
#include <string>

#include "config.h"

struct rodInput {
  double rod_x1;
  double rod_x2;
  double rod_y1;
  double rod_y2;
  double fish_x1;
  double fish_x2;
  double fish_y1;
  double fish_y2;
  int fish_label;
};

int _getRodState(rodInput input);