#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "../Common/test.h"
#include "../Common/timing.h"
#include "../Common/spatial_inrin.h"
#include "../Common/interface.h"
#include "../Specs/cholesky.h"

struct Arguments {
  TYPE a[N * N], L[N * N];
  TYPE a_[N * N], L_[N * N];
  TYPE ref_a[N * N], ref_L[N * N];
} args_;

// Large diagonal keeps every pivot positive for the (shifted) updates below.
struct Arguments *init_data() {
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      args_.a[i * N + j] = args_.a_[i * N + j] = args_.ref_a[i * N + j] =
        (i == j) ? 1000.0 + i : 0.5 * ((i * 3 + j * 5) % 7);
  return &args_;
}

void cholesky(TYPE *a, TYPE *L) {
  #pragma ss config
  {
    arrayhint(a, N * N * sizeof(TYPE), 1.0 - (double) (N * N) / ((N * (N + 1) * (2 * N + 1)) / 6.0));
    arrayhint(L, N * N * sizeof(TYPE), 0);
    for (int i = 0; i < N - 2; ++i) {

      TYPE sqrt_inv, inv, aii = a[i * (N + 1)];
      #pragma ss dfg temporal
      {
        sqrt_inv = 1.0 / fsqrt(aii);
        inv = 1.0 / aii;
      }

      #pragma ss stream nonblock
      #pragma ss dfg dedicated
      for (int j = i; j < N; ++j)
        L[j * N + i] = a[i * N + j + 1] * sqrt_inv;

      #pragma ss stream
      for (int j = i + 1; j < N; ++j) {
        #pragma ss dfg dedicated unroll(2)
        for (int k = j; k < N; ++k) {
          a[j * N + k] -= a[i * N + j] * a[i * N + k + 1] * inv;
        }
      }

    }
  }
}

// IEEE square root without libm (the bare-metal link has no errno).
static TYPE ref_sqrt(TYPE x) {
#if defined(__riscv)
  TYPE r;
  asm("fsqrt.d %0, %1" : "=f"(r) : "f"(x));
  return r;
#else
  return sqrt(x);
#endif
}

// Mirrors cholesky() operation for operation (same association, no FMA).
void run_reference(struct Arguments *args) {
  #pragma clang fp contract(off)
  TYPE *a = args->ref_a, *L = args->ref_L;
  for (int i = 0; i < N - 2; ++i) {
    TYPE aii = a[i * (N + 1)];
    TYPE sqrt_inv = 1.0 / ref_sqrt(aii);
    TYPE inv = 1.0 / aii;
    for (int j = i; j < N; ++j)
      L[j * N + i] = a[i * N + j + 1] * sqrt_inv;
    for (int j = i + 1; j < N; ++j)
      for (int k = j; k < N; ++k) {
        TYPE prod = a[i * N + j] * a[i * N + k + 1];
        prod = prod * inv;
        a[j * N + k] = a[j * N + k] - prod;
      }
  }
}

void run_accelerator(struct Arguments *args, int iswarmup) {
  if (iswarmup) {
    cholesky(args->a, args->L);
  } else {
    cholesky(args->a_, args->L_);
  }
}

int sanity_check(struct Arguments *args) {
  compare_dbl(args->a_, args->ref_a, N * N);
  compare_dbl(args->L_, args->ref_L, N * N);
  return 1;
}
