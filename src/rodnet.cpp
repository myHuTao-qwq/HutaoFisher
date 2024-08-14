#include "rodnet.h"

const double alpha =
    1734.34 / 2.5;  // tangent per pixel of nanodet input screen

// fitted parameters
const double dz[FISH_CLASS_NUM] = {1.0306686, 1.6143661,  1.3931013, 1.2422057,
                                   2.0799911, -0.3158687, 1.9223186, 0.6864969,
                                   1.7144060, 1.9662596,  0.7410775},
             h_coeff[FISH_CLASS_NUM] = {0.5024875, 0.8368146,  0.5266340,
                                        0.4377405, 0.8123552,  -0.7146013,
                                        0.3847316, -0.3302139, 0.5946695,
                                        0.8471138, -0.1875291},
             weight[FISH_CLASS_NUM][3] = {{0.5475771, -3.5762033, 2.2496860},
                                          {-0.4133069, -2.4102299, 3.1962290},
                                          {0.1111080, -0.1539089, 0.7149971},
                                          {-0.0844608, -1.2611908, 1.1210262},
                                          {0.7631445, -2.7045443, 2.6125777},
                                          {-0.1799275, -1.2351764, 1.1879472},
                                          {-0.0618303, -1.0438598, 0.7659825},
                                          {0.2884026, -18.0199852, 2.9590788},
                                          {-0.5060638, -3.1528072, 1.4466045},
                                          {-0.6065114, -2.2915275, 2.1647522},
                                          {-0.0466425, -1.3365734, 1.1907655}},
             bias[FISH_CLASS_NUM][3] = {{2.4757149, 13.2564144, -9.6482182},
                                        {6.5577569, 13.3056774, -13.5220137},
                                        {1.9305423, 2.3804696, -2.5241368},
                                        {2.5643363, 7.8848619, -6.5391688},
                                        {1.2430325, 12.2477198, -11.0323315},
                                        {2.9600599, 7.5668612, -7.8188167},
                                        {1.7597898, 5.4774885, -4.5214224},
                                        {-20.9370270, 45.4211655, -42.8965797},
                                        {5.2681618, 13.3618031, -9.9984989},
                                        {7.3948407, 13.2555265, -14.1105747},
                                        {3.7088640, 8.3868761, -7.2634320}};

#ifdef DATA_COLLECT
const double offset[FISH_CLASS_NUM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
const double offset[FISH_CLASS_NUM] = {0.8, 0.3, 0.25, 0.3, 0.6, 0.4,
                                       0.3, 0.8, 0.8,  0.6, 0.7};
#endif

void softmax(double* dst, double* x, int n) {
  double sum = 0;
  for (int i = 0; i < n; i++) {
    dst[i] = exp(x[i]);
    sum += dst[i];
  }
  for (int i = 0; i < n; i++) {
    dst[i] /= sum;
  }
  return;
}

int _getRodState(rodInput input) {
  double a, b, v0, u, v, h;

  a = (input.rod_x2 - input.rod_x1) / 2 / alpha;
  b = (input.rod_y2 - input.rod_y1) / 2 / alpha;
  h = (input.fish_y2 - input.fish_y1) / 2 / alpha;

  if (a < b) {
    b = sqrt(a * b);
    a = b + 1e-6;
  }

  v0 = (288 - (input.rod_y1 + input.rod_y2) / 2) / alpha;

  u = (input.fish_x1 + input.fish_x2 - input.rod_x1 - input.rod_x2) / 2 / alpha;
  v = (288 - (input.fish_y1 + input.fish_y2) / 2) / alpha;

  int label = input.fish_label;

  v -= h * h_coeff[label];

  double y0, z0, t;
  double x, y, dist;

  y0 = sqrt(pow(a, 4) - b * b + a * a * (1 - b * b + v0 * v0)) / (a * a);
  z0 = b / (a * a);
  t = a * a * (y0 * b + v0) / (a * a - b * b);

  x = u * (z0 + dz[label]) * sqrt(1 + t * t) / (t - v);
  y = (z0 + dz[label]) * (1 + t * v) / (t - v);
  dist = sqrt(x * x + (y - y0) * (y - y0));

  double logits[3];
  for (int i = 0; i < 3; i++) {
    logits[i] = weight[label][i] * dist + bias[label][i];
  }

  double pred[3];
  softmax(pred, logits, 3);
  pred[0] -= offset[input.fish_label];  // to make the prediction more precise
                                        // when deployed

  return int(std::max_element(pred, pred + 3) - pred);
}