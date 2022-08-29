#include "rodnet.h"

const double alpha =
    1734.34 / 2.5;  // tangent per pixel of nanodet input screen

const double dz[FISH_CLASS_NUM] = {0.694767486335, 0.705337270051,
                                   0.686332216768, 1.220813102738,
                                   1.094953763608, 1.075469787444,
                                   0.816349156903, 0.805647715939,
                                   0.580136160296, 0.717789275813},
             theta[3][1 + FISH_CLASS_NUM] =
                 {{-0.063321387830, 0.271287522041, -0.289622288023,
                   0.369747102726, -0.382151700443, -0.209881378546,
                   0.216269445203, 0.047319592279, -0.336408813103,
                   0.472667978506, 0.063048934754},
                  {-0.409084259711, -0.435680067346, -0.021881190130,
                   -0.029230749790, 0.452849262476, -0.152770124847,
                   0.483840163995, 0.375548149249, 0.430277401509,
                   -0.083056601200, 0.402188636327},
                  {0.432541882301, 0.189117922865, 0.348296203478,
                   -0.046222606584, 0.068145706124, 0.057344159790,
                   -0.504384913277, -0.510813307110, -0.440694262582,
                   -0.300708292908, -0.200549034557}},
             B[3] = {0.864525353743, 1.243253422151,
                     -2.503097924792};  // fitted parameters

const double offset[FISH_CLASS_NUM] = {0.25, 0.05, 0.25, 0,   0.15,
                                       0.2,  0.2,  0.1,  0.3, 0.15};

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