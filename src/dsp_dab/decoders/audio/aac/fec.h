/* Main header for reduced libfec.
 *
 * The FEC code in this folder is
 * Copyright 2003 Phil Karn, KA9Q
 * May be used under the terms of the GNU Lesser General Public License (LGPL)
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

/* Reed-Solomon codec control block */
struct rs
{
  int mm; /* Bits per symbol */
  int nn; /* Symbols per block (= (1<<mm)-1) */
  uint8_t* alpha_to; /* log lookup table */
  uint8_t* index_of; /* Antilog lookup table */
  uint8_t* genpoly; /* Generator polynomial */
  int nroots; /* Number of generator roots = number of parity symbols */
  int fcr; /* First consecutive root, index form */
  int prim; /* Primitive element, index form */
  int iprim; /* prim-th root of 1, index form */
  int pad; /* Padding bytes in shortened block */

  uint8_t* lambda;
  uint8_t* s;
  uint8_t* b;
  uint8_t* t;
  uint8_t* omega;
  uint8_t* root;
  uint8_t* reg;
  uint8_t* loc;
};

static inline int modnn(struct rs* rs, int x)
{
  while (x >= rs->nn)
  {
    x -= rs->nn;
    x = (x >> rs->mm) + (x & rs->nn);
  }
  return x;
}

/* Initialize a Reed-Solomon codec
 * symsize = symbol size, bits
 * gfpoly = Field generator polynomial coefficients
 * fcr = first root of RS code generator polynomial, index form
 * prim = primitive element to generate polynomial roots
 * nroots = RS code generator polynomial degree (number of roots)
 * pad = padding bytes at front of shortened block
 */
void* init_rs_dab(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad);

void free_rs_dab(void* p);

int decode_rs_dab(void* p, uint8_t* data, int* eras_pos, int no_eras);
