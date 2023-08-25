#include "rodnet.h"

const double alpha =
    1734.34 / 2.5;  // tangent per pixel of nanodet input screen

const double
    dz[FISH_CLASS_NUM] = {0.579555011272, 0.607075089722, 0.655828120094,
                          1.053958792001, 1.027940522453, 1.176038938876,
                          1.363470417831, 1.614591088284, 0.900931053376,
                          1.008025303741, 1.218730669890, 1.235426623787},
    theta[3][1 + FISH_CLASS_NUM] =
        {{-0.091167296647, 0.283792064674, -0.559088023013, 0.156341374661,
          -0.641171856583, -0.111692867402, -0.025891345803, -0.118130345896,
          -0.259355645208, 0.320409134326, 0.286284584755, 0.483840808036,
          0.714874029724},
         {-0.790103981444, -0.627952580626, -0.455042631703, -0.052872883554,
          0.581543746670, -0.295458217380, 0.957901703928, 0.867878454027,
          1.606664221939, 0.150593652979, 0.089977303165, 0.191943455061,
          0.147422923314},
         {0.557450344391, 0.245339501786, 0.513697641873, -0.105311153800,
          0.650557379703, 0.264734547974, -0.964331736713, -1.004471478207,
          -0.891380817322, -0.764475807501, -0.564087761414, -0.849276307308,
          -0.695472850636}},
    B[3] = {1.168009387132, 2.766292871500,
            -3.708058852097};  // fitted parameters

#ifdef RELEASE
const double offset[FISH_CLASS_NUM] = {0.3, 0.1,  0.25, 0,    0.2, 0.2,
                                       0.2, -0.1, 0.3,  0.25, 0.2, 0.2};
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