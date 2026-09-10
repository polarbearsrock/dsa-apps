#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define init_linear(a, n_)            \
  do {                                \
    int64_t _n_ = n_;                 \
    for (int64_t i = 0; i < _n_; ++i) \
      a[i] = (i + 1);                 \
  } while (0)

#define init_odd(a, n_)               \
  do {                                \
    int64_t _n_ = n_;                 \
    for (int64_t i = 0; i < _n_; ++i) \
      a[i] = (i * 2 + 1);             \
  } while (0)

#define init_even(a, n_)              \
  do {                                \
    int64_t _n_ = n_;                 \
    for (int64_t i = 0; i < _n_; ++i) \
      a[i] = (i + 1) * 2;             \
  } while (0)

#define init_rand(a, n_)              \
  do {                                \
    int64_t _n_ = n_;                 \
    for (int64_t i = 0; i < _n_; ++i) \
      a[i] = rand() % _n_;            \
  } while (0)

#define compare(a, b, n_, fmt)            \
  do {                                    \
    int64_t _n_ = n_;                     \
    for (int64_t i = 0; i < _n_; ++i) {   \
      if (1e-5 < (a[i] - b[i]) ||         \
          -1e-5 > (a[i] - b[i])) {        \
        printf("Mismatch @ Iter %ld: calculated:" fmt " != expected:" fmt "\n", \
               i, a[i], b[i]);            \
        exit(1);                          \
      }                                   \
    }                                     \
  } while (0)

#endif // __TEST_H__

// Like compare(), but reports doubles in a form the bare-metal printf can show:
// the raw IEEE bits and the value in 1/1000 units.
#define compare_dbl(a, b, n_)                                           \
  do {                                                                  \
    int64_t _n_ = n_; int _bad_ = 0;                                    \
    for (int64_t i = 0; i < _n_; ++i) {                                 \
      double _d_ = (double) (a[i]) - (double) (b[i]);                   \
      if (1e-5 < _d_ || -1e-5 > _d_) {                                  \
        union { double d; unsigned long u; } _x_, _y_;                  \
        _x_.d = a[i]; _y_.d = b[i];                                      \
        printf("Mismatch @ Iter %ld: calculated:%lx (%ld/1000) != expected:%lx (%ld/1000)\n", \
               i, _x_.u, (long) (_x_.d * 1000), _y_.u, (long) (_y_.d * 1000)); \
        if (++_bad_ >= 12) exit(1);                                     \
      }                                                                 \
    }                                                                   \
    if (_bad_) exit(1);                                                 \
  } while (0)
