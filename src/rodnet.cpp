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

#ifdef RELEASE
const double offset[FISH_CLASS_NUM] = {0.4, 0.2,  0.4, 0,   0.3, 0.3,
                                       0.3, 0.15, 0.5, 0.5, 0.5, 0.5};
#else
const double offset[FISH_CLASS_NUM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

// dst = {a,b,v}
void f(double* dst, double* x, double* y) {
  double y0 = x[0], z0 = x[1], t = x[2];
  double tmp = (y0 + t * z0) * (y0 + t * z0) - 1;
  dst[0] = sqrt((1 + t * t) / tmp) - y[0];
  dst[1] = (1 + t * t) * z0 / tmp - y[1];
  dst[2] = ((t * t - 1) * y0 * z0 + t * (y0 * y0 - z0 * z0 - 1)) / tmp - y[2];
  return;
}

void dfInv(double* dst, double* x) {
  double y0 = x[0], z0 = x[1], t = x[2];
  double tmp1 = (y0 + t * z0) * (y0 + t * z0) - 1;
  double tmp2 = 1 + t * t;
  dst[0] = (1 - y0 * y0 + z0 * z0) / y0 * sqrt(tmp1 / tmp2);
  dst[1] = -z0 * (y0 * y0 + t * (t * (1 + z0 * z0) + 2 * y0 * z0)) / tmp2 / y0;
  dst[2] = ((t * t - 1) * y0 * z0 + t * (y0 * y0 - z0 * z0 - 1)) / y0 / tmp2;
  dst[3] = -2 * z0 * sqrt(tmp1 / tmp2);
  dst[4] = tmp1 / tmp2;
  dst[5] = 0;
  dst[6] = -z0 / y0 * sqrt(tmp1 / tmp2);
  dst[7] = (y0 + t * z0) * (y0 + t * z0) / y0;
  dst[8] = 1 + t * z0 / y0;
  return;
}

bool NewtonRaphson(void f(double* dst, double* x, double* y),
                   void dfInv(double* dst, double* x), double* dst, double* y,
                   double* init, int n, int maxIter, double eps) {
  double* f_est = new double[n];
  double* df_inv = new double[n * n];
  double* x = new double[n];
  double err;

  memcpy(x, init, n * sizeof(double));

  for (int iter = 0; iter < maxIter; iter++) {
    err = 0;
    f(f_est, x, y);
    for (int i = 0; i < n; i++) {
      err += abs(f_est[i]);
    }

    if (err < eps) {
      memcpy(dst, x, n * sizeof(double));
      delete f_est;
      delete df_inv;
      delete x;
      // printf("Newton-Raphson solver converge after %d steps, err: %lf !\n",
      //        iter, err);
      return true;
    }

    dfInv(df_inv, x);

    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        x[i] -= df_inv[n * i + j] * f_est[j];
      }
    }
  }
  delete f_est;
  delete df_inv;
  delete x;
  return false;
}

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
    std::swap(a, b);  // when vision is almost vertical this can cause error
  }

  v0 = (288 - (input.rod_y1 + input.rod_y2) / 2) / alpha;

  u = (input.fish_x1 + input.fish_x2 - input.rod_x1 - input.rod_x2) / 2 / alpha;
  v = (288 - (input.fish_y1 + input.fish_y2) / 2) / alpha;

  double y0z0t[3], abv0[3] = {a, b, v0},
                   init[3] = {30, 15, 1};  // empirical value

  double solveSuccess =
      NewtonRaphson(f, dfInv, y0z0t, abv0, init, 3, 1000, 1e-6);

  if (!solveSuccess) {
    return -1;
  }

  double y0 = y0z0t[0], z0 = y0z0t[1], t = y0z0t[2];
  double x, y, dist;
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