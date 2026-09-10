#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "../Common/spatial_inrin.h"
#include "../Common/test.h"
#include "../Common/timing.h"
#include "../Specs/solver.h"


#ifndef N
#define N 48
#endif

void solver(TYPE *a, TYPE *v) {
  #pragma ss config
  {
    arrayhint(a, N * N * sizeof(TYPE), 0);
    arrayhint(v, N * sizeof(TYPE), 1.0 - (double) N / ((1 + N) * N / 2.0));
    for (int i = 0; i < N - 1; ++i) {
      TYPE vv = 0;
      TYPE v0 = v[i];
      TYPE a0 = a[i * N + i];
      #pragma ss dfg temporal
      {
        vv = v0 / a0;
      }
      // v[i] = vv;
      #pragma ss stream
      #pragma ss dfg dedicated unroll(4)
      for (int j = i + 1; j < N; ++j) {
        v[j] -= a[i * N + j] * vv;
      }
    }
  }
}

struct Arguments {
  TYPE a[N * N], v[N];
  TYPE a_[N * N], v_[N];
  TYPE ref[N];
} args_;

// Diagonally dominant system keeps the values bounded.
struct Arguments *init_data() {
#ifdef ZERO_DATA
  return &args_;  // all-zero inputs (hang bisection only)
#endif
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      args_.a[i * N + j] = args_.a_[i * N + j] = (i == j) ? 16.0 : (TYPE) (((i * 7 + j * 3) % 5) - 2);
  for (int i = 0; i < N; ++i) args_.v[i] = args_.v_[i] = args_.ref[i] = (i % 5) + 1;
  return &args_;
}

// Mirrors solver(): the pivot is not stored back, no FMA contraction.
void run_reference(struct Arguments *args) {
  #pragma clang fp contract(off)
  TYPE *a = args->a, *v = args->ref;
  for (int i = 0; i < N - 1; ++i) {
    TYPE vv = v[i] / a[i * N + i];
    for (int j = i + 1; j < N; ++j) {
      TYPE prod = a[i * N + j] * vv;
      v[j] = v[j] - prod;
    }
  }
}

void run_accelerator(struct Arguments *args, int _) {
  if (_) {
    solver(args->a, args->v);
  } else {
    solver(args->a_, args->v_);
  }
}

int sanity_check(struct Arguments *args) {
  compare_dbl(args->v_, args->ref, N);
  return 1;
}
