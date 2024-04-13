/* Main source for reduced libfec.
 *
 * The FEC code in this folder is
 * Copyright 2003 Phil Karn, KA9Q
 * May be used under the terms of the GNU Lesser General Public License (LGPL)
 */

#include "fec.h"

#include <string.h>

#ifndef min
#define min(a, b) \
  ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b; \
  })
#endif

/* Initialize a Reed-Solomon codec
 * symsize = Symbol size, bits
 * gfpoly = Field generator polynomial coefficients
 * fcr = first root of RS code generator polynomial, index form
 * prim = primitive element to generate polynomial roots
 * nroots = RS code generator polynomial degree (number of roots)
 * pad = padding bytes at front of shortened block
 */
void* init_rs_dab(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad)
{
  struct rs* rs;

  int i, j, sr, root, iprim;

  /* Check parameter ranges */
  if (symsize < 0 || (size_t)symsize > 8 * sizeof(uint8_t))
  {
    return NULL;
  }

  if (fcr < 0 || fcr >= (1 << symsize))
    return NULL;
  if (prim <= 0 || prim >= (1 << symsize))
    return NULL;
  if (nroots < 0 || nroots >= (1 << symsize))
    return NULL; /* Can't have more roots than symbol values! */
  if (pad < 0 || pad >= ((1 << symsize) - 1 - nroots))
    return NULL; /* Too much padding */

  rs = (struct rs*)calloc(1, sizeof(struct rs));
  if (rs == NULL)
    return NULL;

  rs->mm = symsize;
  rs->nn = (1 << symsize) - 1;
  rs->pad = pad;

  rs->alpha_to = (uint8_t*)malloc(sizeof(uint8_t) * (rs->nn + 1));
  if (rs->alpha_to == NULL)
  {
    free(rs);
    return NULL;
  }
  rs->index_of = (uint8_t*)malloc(sizeof(uint8_t) * (rs->nn + 1));
  if (rs->index_of == NULL)
  {
    free(rs->alpha_to);
    free(rs);
    return NULL;
  }

  /* Generate Galois field lookup tables */
  rs->index_of[0] = rs->nn; /* log(zero) = -inf */
  rs->alpha_to[rs->nn] = 0; /* alpha**-inf = 0 */
  sr = 1;
  for (i = 0; i < rs->nn; i++)
  {
    rs->index_of[sr] = i;
    rs->alpha_to[i] = sr;
    sr <<= 1;
    if (sr & (1 << symsize))
      sr ^= gfpoly;
    sr &= rs->nn;
  }
  if (sr != 1)
  {
    /* field generator polynomial is not primitive! */
    free(rs->alpha_to);
    free(rs->index_of);
    free(rs);
    return NULL;
  }

  /* Form RS code generator polynomial from its roots */
  rs->genpoly = (uint8_t*)malloc(sizeof(uint8_t) * (nroots + 1));
  if (rs->genpoly == NULL)
  {
    free(rs->alpha_to);
    free(rs->index_of);
    free(rs);
    return NULL;
  }
  rs->fcr = fcr;
  rs->prim = prim;
  rs->nroots = nroots;

  /* Find prim-th root of 1, used in decoding */
  for (iprim = 1; (iprim % prim) != 0; iprim += rs->nn)
    ;
  rs->iprim = iprim / prim;

  rs->genpoly[0] = 1;
  for (i = 0, root = fcr * prim; i < nroots; i++, root += prim)
  {
    rs->genpoly[i + 1] = 1;

    /* Multiply rs->genpoly[] by  @**(root + x) */
    for (j = i; j > 0; j--)
    {
      if (rs->genpoly[j] != 0)
        rs->genpoly[j] =
            rs->genpoly[j - 1] ^ rs->alpha_to[modnn(rs, rs->index_of[rs->genpoly[j]] + root)];
      else
        rs->genpoly[j] = rs->genpoly[j - 1];
    }
    /* rs->genpoly[0] can never be zero */
    rs->genpoly[0] = rs->alpha_to[modnn(rs, rs->index_of[rs->genpoly[0]] + root)];
  }
  /* convert rs->genpoly[] to index form for quicker encoding */
  for (i = 0; i <= nroots; i++)
    rs->genpoly[i] = rs->index_of[rs->genpoly[i]];

  rs->lambda = malloc(sizeof(uint8_t) * (rs->nroots + 1));
  rs->s = malloc(sizeof(uint8_t) * rs->nroots);
  rs->b = malloc(sizeof(uint8_t) * (rs->nroots + 1));
  rs->t = malloc(sizeof(uint8_t) * (rs->nroots + 1));
  rs->omega = malloc(sizeof(uint8_t) * (rs->nroots + 1));
  rs->root = malloc(sizeof(uint8_t) * rs->nroots);
  rs->reg = malloc(sizeof(uint8_t) * (rs->nroots + 1));
  rs->loc = malloc(sizeof(uint8_t) * rs->nroots);

  return rs;
}

void free_rs_dab(void* p)
{
  if (p == NULL)
    return;

  struct rs* rs = (struct rs*)p;

  free(rs->loc);
  free(rs->reg);
  free(rs->root);
  free(rs->omega);
  free(rs->t);
  free(rs->b);
  free(rs->s);
  free(rs->lambda);

  free(rs->alpha_to);
  free(rs->index_of);
  free(rs->genpoly);
  free(rs);
}

int decode_rs_dab(void* p, uint8_t* data, int* eras_pos, int no_eras)
{
  struct rs* rs = (struct rs*)p;

  int deg_lambda, el, deg_omega;
  int i, j, r, k;
  uint8_t u, q, tmp, num1, num2, den, discr_r;
  int syn_error, count;

  /* form the syndromes; i.e., evaluate data(x) at roots of g(x) */
  for (i = 0; i < rs->nroots; i++)
    rs->s[i] = data[0];

  for (j = 1; j < rs->nn - rs->pad; j++)
  {
    for (i = 0; i < rs->nroots; i++)
    {
      if (rs->s[i] == 0)
      {
        rs->s[i] = data[j];
      }
      else
      {
        rs->s[i] =
            data[j] ^ rs->alpha_to[modnn(rs, rs->index_of[rs->s[i]] + (rs->fcr + i) * rs->prim)];
      }
    }
  }

  /* Convert syndromes to index form, checking for nonzero condition */
  syn_error = 0;
  for (i = 0; i < rs->nroots; i++)
  {
    syn_error |= rs->s[i];
    rs->s[i] = rs->index_of[rs->s[i]];
  }

  if (!syn_error)
  {
    /* if syndrome is zero, data[] is a codeword and there are no
     * errors to correct. So return data[] unmodified
     */
    return 0;
  }
  memset(&rs->lambda[1], 0, rs->nroots * sizeof(rs->lambda[0]));
  rs->lambda[0] = 1;

  if (no_eras > 0)
  {
    /* Init lambda to be the erasure locator polynomial */
    rs->lambda[1] = rs->alpha_to[modnn(rs, rs->prim * (rs->nn - 1 - eras_pos[0]))];
    for (i = 1; i < no_eras; i++)
    {
      u = modnn(rs, rs->prim * (rs->nn - 1 - eras_pos[i]));
      for (j = i + 1; j > 0; j--)
      {
        tmp = rs->index_of[rs->lambda[j - 1]];
        if (tmp != rs->nn)
          rs->lambda[j] ^= rs->alpha_to[modnn(rs, u + tmp)];
      }
    }

#if DEBUG >= 1
    /* Test code that verifies the erasure locator polynomial just constructed
      Needed only for decoder debugging. */

    /* find roots of the erasure location polynomial */
    for (i = 1; i <= no_eras; i++)
      rs->reg[i] = rs->index_of[rs->lambda[i]];

    count = 0;
    for (i = 1, k = rs->iprim - 1; i <= rs->nn; i++, k = modnn(rs, k + rs->iprim))
    {
      q = 1;
      for (j = 1; j <= no_eras; j++)
        if (rs->reg[j] != rs->nn)
        {
          rs->reg[j] = modnn(rs, rs->reg[j] + j);
          q ^= rs->alpha_to[rs->reg[j]];
        }
      if (q != 0)
        continue;
      /* store root and error location number indices */
      rs->root[count] = i;
      rs->loc[count] = k;
      count++;
    }
    if (count != no_eras)
    {
      fprintf(stderr, "count = %d no_eras = %d\n lambda(x) is WRONG\n", count, no_eras);
      return -1;
    }
#if DEBUG >= 2
    fprintf(stderr, "\n Erasure positions as determined by roots of Eras Loc Poly:\n");
    for (i = 0; i < count; i++)
      fprintf(stderr, "%d ", rs->loc[i]);
    fprintf(stderr, "\n");
#endif
#endif
  }
  for (i = 0; i < rs->nroots + 1; i++)
    rs->b[i] = rs->index_of[rs->lambda[i]];

  /*
   * Begin Berlekamp-Massey algorithm to determine error+erasure
   * locator polynomial
   */
  r = no_eras;
  el = no_eras;
  while (++r <= rs->nroots)
  { /* r is the step number */
    /* Compute discrepancy at the r-th step in poly-form */
    discr_r = 0;
    for (i = 0; i < r; i++)
    {
      if ((rs->lambda[i] != 0) && (rs->s[r - i - 1] != rs->nn))
      {
        discr_r ^= rs->alpha_to[modnn(rs, rs->index_of[rs->lambda[i]] + rs->s[r - i - 1])];
      }
    }
    discr_r = rs->index_of[discr_r]; /* Index form */
    if (discr_r == rs->nn)
    {
      /* 2 lines below: B(x) <-- x*B(x) */
      memmove(&rs->b[1], rs->b, rs->nroots * sizeof(rs->b[0]));
      rs->b[0] = rs->nn;
    }
    else
    {
      /* 7 lines below: T(x) <-- rs->lambda(x) - discr_r*x*rs->b(x) */
      rs->t[0] = rs->lambda[0];
      for (i = 0; i < rs->nroots; i++)
      {
        if (rs->b[i] != rs->nn)
          rs->t[i + 1] = rs->lambda[i + 1] ^ rs->alpha_to[modnn(rs, discr_r + rs->b[i])];
        else
          rs->t[i + 1] = rs->lambda[i + 1];
      }
      if (2 * el <= r + no_eras - 1)
      {
        el = r + no_eras - el;
        /*
	       * 2 lines below: B(x) <-- inv(discr_r) *
	       * rs->lambda(x)
	       */
        for (i = 0; i <= rs->nroots; i++)
          rs->b[i] = (rs->lambda[i] == 0)
                         ? rs->nn
                         : modnn(rs, rs->index_of[rs->lambda[i]] - discr_r + rs->nn);
      }
      else
      {
        /* 2 lines below: B(x) <-- x*B(x) */
        memmove(&rs->b[1], rs->b, rs->nroots * sizeof(rs->b[0]));
        rs->b[0] = rs->nn;
      }
      memcpy(rs->lambda, rs->t, (rs->nroots + 1) * sizeof(rs->t[0]));
    }
  }

  /* Convert lambda to index form and compute deg(rs->lambda(x)) */
  deg_lambda = 0;
  for (i = 0; i < rs->nroots + 1; i++)
  {
    rs->lambda[i] = rs->index_of[rs->lambda[i]];
    if (rs->lambda[i] != rs->nn)
      deg_lambda = i;
  }
  /* Find roots of the error+erasure locator polynomial by Chien search */
  memcpy(&rs->reg[1], &rs->lambda[1], rs->nroots * sizeof(rs->reg[0]));
  count = 0; /* Number of roots of rs->lambda(x) */
  for (i = 1, k = rs->iprim - 1; i <= rs->nn; i++, k = modnn(rs, k + rs->iprim))
  {
    q = 1; /* rs->lambda[0] is always 0 */
    for (j = deg_lambda; j > 0; j--)
    {
      if (rs->reg[j] != rs->nn)
      {
        rs->reg[j] = modnn(rs, rs->reg[j] + j);
        q ^= rs->alpha_to[rs->reg[j]];
      }
    }
    if (q != 0)
      continue; /* Not a root */
      /* store rs->root (index-form) and error location number */
#if DEBUG >= 2
    fprintf(stderr, "count %d root %d loc %d\n", count, i, k);
#endif
    rs->root[count] = i;
    rs->loc[count] = k;
    /* If we've already found max possible roots,
     * abort the search to save time
     */
    if (++count == deg_lambda)
      break;
  }
  if (deg_lambda != count)
  {
    /*
     * deg(rs->lambda) unequal to number of roots => uncorrectable
     * error detected
     */
    return -1;
  }
  /*
   * Compute err+eras evaluator poly rs->omega(x) = rs->s(x)*rs->lambda(x) (modulo
   * x**rs->nroots). in index form. Also find deg(rs->omega).
   */
  deg_omega = deg_lambda - 1;
  for (i = 0; i <= deg_omega; i++)
  {
    tmp = 0;
    for (j = i; j >= 0; j--)
    {
      if ((rs->s[i - j] != rs->nn) && (rs->lambda[j] != rs->nn))
        tmp ^= rs->alpha_to[modnn(rs, rs->s[i - j] + rs->lambda[j])];
    }
    rs->omega[i] = rs->index_of[tmp];
  }

  /*
   * Compute error values in poly-form. num1 = rs->omega(inv(X(l))), num2 =
   * inv(X(l))**((rs->fcr-1) and den = lambda_pr(inv(X(l))) all in poly-form
   */
  for (j = count - 1; j >= 0; j--)
  {
    num1 = 0;
    for (i = deg_omega; i >= 0; i--)
    {
      if (rs->omega[i] != rs->nn)
        num1 ^= rs->alpha_to[modnn(rs, rs->omega[i] + i * rs->root[j])];
    }
    num2 = rs->alpha_to[modnn(rs, rs->root[j] * (rs->fcr - 1) + rs->nn)];
    den = 0;

    /* rs->lambda[i+1] for i even is the formal derivative lambda_pr of rs->lambda[i] */
    for (i = min(deg_lambda, rs->nroots - 1) & ~1; i >= 0; i -= 2)
    {
      if (rs->lambda[i + 1] != rs->nn)
        den ^= rs->alpha_to[modnn(rs, rs->lambda[i + 1] + i * rs->root[j])];
    }
#if DEBUG >= 1
    if (den == 0)
    {
      fprintf(stderr, "\n ERROR: denominator = 0\n");
      return -1;
    }
#endif
    /* Apply error to data */
    if (num1 != 0 && rs->loc[j] >= rs->pad)
    {
      data[rs->loc[j] - rs->pad] ^= rs->alpha_to[modnn(rs, rs->index_of[num1] + rs->index_of[num2] +
                                                               rs->nn - rs->index_of[den])];
    }
  }

  if (eras_pos != NULL)
  {
    for (i = 0; i < count; i++)
      eras_pos[i] = rs->loc[i];
  }

  return count;
}
