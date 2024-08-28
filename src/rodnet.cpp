#include "rodnet.h"

const double alpha =
    1734.34 / 2.5;  // tangent per pixel of nanodet input screen

// fitted parameters
const double dz[FISH_CLASS_NUM] = {1.0307939, 1.5887239,  1.4377865, 0.8548809,
                                   1.8640924, -0.1687729, 1.8621461, 0.7167622,
                                   1.7071064, 1.8727832,  0.5531539},
             h_coeff[FISH_CLASS_NUM] = {0.5840698,  0.8029298,  0.6090596,
                                        -0.1390072, 0.7214464,  -0.6076725,
                                        0.3286690,  -0.2991239, 0.6072225,
                                        0.7662407,  -0.3689651},
             weight[FISH_CLASS_NUM][3] = {{0.7779633, -1.7124480, 2.7366412},
                                          {-0.0381155, -1.6536976, 3.5904298},
                                          {0.1947731, -0.0445049, 0.8416666},
                                          {-0.0331017, -1.3641578, 1.2834741},
                                          {1.0268835, -1.6553984, 2.9930501},
                                          {0.0108103, -0.8515291, 1.0032536},
                                          {-0.0746362, -0.9677668, 0.7450780},
                                          {0.7382144, -9.5275803, 2.6134675},
                                          {-0.3597502, -1.7422760, 1.4354013},
                                          {-0.0578425, -2.0274212, 1.7173727},
                                          {-0.1225260, -1.0630554, 1.2958838}},
             bias[FISH_CLASS_NUM][3] = {{3.1733532, 9.3601589, -11.0612173},
                                        {6.4961057, 11.2683334, -13.7752209},
                                        {2.3662698, 2.4709859, -2.5402584},
                                        {2.4701204, 8.5112562, -7.6070199},
                                        {0.9597272, 8.9189463, -11.9037018},
                                        {2.1239815, 5.8446727, -5.7748013},
                                        {2.1403685, 5.5432696, -4.0048418},
                                        {-9.0128260, 28.4402637, -24.2205143},
                                        {5.2072763, 8.6428480, -9.2946615},
                                        {4.9253063, 11.4634714, -9.4336052},
                                        {5.2460732, 7.7711511, -7.5998945}};

#ifdef DATA_COLLECT
const double offset[FISH_CLASS_NUM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
const double offset[FISH_CLASS_NUM] = {0.8, 0.4, 0.35, 0.35, 0.6, 0.3,
                                       0.3, 0.8, 0.8,  0.8,  0.8};
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