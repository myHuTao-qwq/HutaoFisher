#include "rodnet.h"

const double alpha =
    1734.34 / 2.5;  // tangent per pixel of nanodet input screen

// fitted parameters
const double dz[FISH_CLASS_NUM] = {0.9851715, 1.4054390,  1.8393939, 0.6135482,
                                   2.1581705, -0.1897278, 1.6540480, 1.0343770,
                                   1.7012502, 1.9880142,  1.0431960},
             h_coeff[FISH_CLASS_NUM] = {0.4750018,  0.7015906,  0.6987048,
                                        -0.3205624, 0.9526339,  -0.6445330,
                                        0.2402014,  -0.0360333, 0.5685504,
                                        0.8629405,  0.1504580},
             weight[FISH_CLASS_NUM][3] =
                 {{3.5397220e-02, -3.4287031e+00, 1.6048468e+00},
                  {8.2981072e-02, -1.7058493e+00, 2.3874671e+00},
                  {1.2407478e-01, 1.6143405e-01, 7.7104688e-01},
                  {6.2074163e-03, -1.0359080e+00, 1.1525167e+00},
                  {-7.1919703e-01, -4.8256955e+00, 1.9032588e+00},
                  {9.2586949e-02, -8.1881726e-01, 1.4554611e+00},
                  {-6.5701487e-03, -9.8798805e-01, 8.2569987e-01},
                  {-2.2301233e-01, -8.4512320e+00, 2.2345786e+00},
                  {-4.7096670e-01, -3.0073380e+00, 1.5616963e+00},
                  {-2.5770429e-01, -2.2239101e+00, 2.5113721e+00},
                  {5.0787885e-02, -1.2928698e+00, 1.3068954e+00}},
             bias[FISH_CLASS_NUM][3] = {{1.2142940, 10.4697227, -9.8416061},
                                        {2.3252437, 8.4531937, -10.9660082},
                                        {2.6797915, 1.2335930, -2.0881109},
                                        {2.4344676, 7.0287461, -6.1795411},
                                        {2.8694069, 16.4680099, -15.2516127},
                                        {3.8714075, 7.6705694, -7.0723877},
                                        {1.8523616, 5.5640178, -4.5101700},
                                        {-4.6569219, 25.0882187, -24.7824783},
                                        {3.7018719, 11.4508324, -12.0481672},
                                        {6.4655323, 13.6047897, -14.9730387},
                                        {3.0985067, 7.9947314, -7.9432259}};

#ifdef DATA_COLLECT
const double offset[FISH_CLASS_NUM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
const double offset[FISH_CLASS_NUM] = {0.8, 0.4, 0.4, 0.4, 0.8, 0.5,
                                       0.4, 0.9, 0.9, 0.8, 0.7};
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