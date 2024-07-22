#include "rodnet.h"

const double alpha =
    1734.34 / 2.5;  // tangent per pixel of nanodet input screen

const double
    dz[FISH_CLASS_NUM] = {0.561117562965, 0.637026851288, 0.705579317577,
                          1.062734463845, 0.949307580751, 1.015620474332,
                          1.797904203405, 1.513476738412, 1.013873007495,
                          1.159949954831, 1.353650974146, 1.302893195071},
    theta[3][1 + FISH_CLASS_NUM] =
        {{-0.262674397633, 0.317025388945, -0.457150765450, 0.174522158281,
          -0.957110676932, -0.095339800558, -0.119519564026, -0.139914755291,
          -0.580893838475, 0.702302245305, 0.271575851220, 0.708473199472,
          0.699108382380},
         {-1.062702043060, -0.280779165943, -0.289891597384, 0.220173840594,
          0.493463877037, -0.326492366566, 1.215859141832, 1.607133159643,
          1.619199133672, 0.356402262447, 0.365385941958, 0.411869019381,
          0.224962055122},
         {0.460481782256, 0.048180392806, 0.475529271293, -0.150186412126,
          0.135512307120, 0.087365984352, -1.317661146364, -1.882438208662,
          -1.502483859283, -0.580228373556, -1.005821958682, -1.184199131739,
          -1.285988918494}},
    B[3] = {1.241950004386, 3.051113640564,
            -3.848898190087};  // fitted parameters

#ifdef DATA_COLLECT
const double offset[FISH_CLASS_NUM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
const double offset[FISH_CLASS_NUM] = {0.4, 0.2,  0.4, 0,   0.3, 0.3,
                                       0.3, 0.15, 0.5, 0.5, 0.5, 0.5};
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
  double a, b, v0, u, v;

  a = (input.rod_x2 - input.rod_x1) / 2 / alpha;
  b = (input.rod_y2 - input.rod_y1) / 2 / alpha;

  if (a < b) {
    b = sqrt(a * b);
    a = b + 1e-6;
  }

  v0 = (288 - (input.rod_y1 + input.rod_y2) / 2) / alpha;

  u = (input.fish_x1 + input.fish_x2 - input.rod_x1 - input.rod_x2) / 2 / alpha;
  v = (288 - (input.fish_y1 + input.fish_y2) / 2) / alpha;

  double y0, z0, t;
  double x, y, dist;

  y0 = sqrt(pow(a, 4) - b * b + a * a * (1 - b * b - v0 * v0)) / (a * a);
  z0 = b / (a * a);
  t = a * a * (y0 * b + v0) / (a * a - b * b);

  x = u * (z0 + dz[input.fish_label]) * sqrt(1 + t * t) / (t - v);
  y = (z0 + dz[input.fish_label]) * (1 + t * v) / (t - v);
  dist = sqrt(x * x + (y - y0) * (y - y0));

  double logits[3];
  for (int i = 0; i < 3; i++) {
    logits[i] = theta[i][0] * dist + theta[i][1 + input.fish_label] + B[i];
  }

  double pred[3];
  softmax(pred, logits, 3);
  pred[0] -= offset[input.fish_label];  // to make the prediction more precise
                                        // when deployed

  return int(std::max_element(pred, pred + 3) - pred);
}