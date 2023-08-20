#include "rodnet.h"

const double alpha =
    1734.34 / 2.5;  // tangent per pixel of nanodet input screen

const double
    dz[FISH_CLASS_NUM] = {0.590693621702, 0.623981575078, 0.690914073175,
                          1.103541640724, 1.041895085592, 1.159647545016,
                          1.097916312483, 1.579553747230, 0.860559685618,
                          1.116212242433, 0.642676985582, 0.380209293033},
    theta[3][1 + FISH_CLASS_NUM] =
        {{-0.082130245681, 0.308632340370, -0.289723731453, 0.399834503877,
          -0.474520758962, 0.082554806311, 0.228423298025, 0.330206708792,
          -0.147862550888, 0.621553918786, 0.188201218554, 0.109618028215,
          0.500233913317},
         {-0.659257594690, -0.729069405909, -0.366819523341, -0.064630728161,
          0.487852889871, -0.256368996733, 0.733798536021, 0.687784525514,
          1.393945876211, 0.104497256524, 0.124428693453, -0.181852027243,
          -0.143343215424},
         {0.483615841727, 0.061305401502, 0.414839327278, -0.085898378444,
          0.426064250469, 0.120214817916, -0.706133189779, -0.500624616388,
          -0.792014068541, -0.503643357470, -0.775869671214, -0.561323037490,
          -0.524580542859}},
    B[3] = {0.900309749294,  2.232998206393, -3.197501696174};  // fitted parameters

const double offset[FISH_CLASS_NUM] = {0.3, 0.1,  0.25, 0,    0.2, 0.2,
                                       0.2, -0.1, 0.3,  0.25, 0,   0};

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