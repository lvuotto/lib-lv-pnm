
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lv-pnm.h"


/* #  ~ .lV   ! */


static void            pnm_read_bin              (pnm_context_t *ctx, FILE *fp);
static void            pnm_read_ascii            (pnm_context_t *ctx, FILE *fp);
static pnm_bool_t      pnm_write_bin             (pnm_context_t *ctx,
                                                  const char *filename);
static pnm_bool_t      pnm_write_ascii           (pnm_context_t *ctx,
                                                  const char *filename);


/* Functions */


/* ==========================================================================
 * PNM MATH MACROS & FUNCTIONS
 * ========================================================================== */

#define pnm_max(x, y, z) ((x) > (y) ? (x) : (y) > (z) ? (y) : (z))
#define pnm_min(x, y, z) ((x) < (y) ? (x) : (y) < (z) ? (y) : (z))
#define pnm_fabs(x)      ((x) > 0 ? (x) : (-x))
#define pnm_in_range(x, i, s) ((int) (x) < (int) (i) ? PNM_FALSE : \
                               (int) (x) > (int) (s) ? PNM_FALSE : \
                               PNM_TRUE)

static inline double pnm_fmod (double x, int q) {
  while (x < 0) x += q;
  while (x > q) x -= q;
  return x;
}


/* ==========================================================================
 * ERROR MANIPULATION
 * ========================================================================== */


void pnm_error_exit (pnm_context_t *ctx, int code) {
  pnm_destroy(ctx);
  exit(code);
}


/* ==========================================================================
   PNM MANIPULATION
   ========================================================================== */


pnm_context_t * pnm_create (pnm_image_t type,
                            unsigned int width,
                            unsigned int height,
                            unsigned int maxval)
{
  pnm_context_t *ctx;
  
  ctx = (pnm_context_t *) malloc(sizeof(pnm_context_t));
  ctx->__comm_q = 0;
  ctx->comments = NULL;
  ctx->canvas = NULL;
  
  pnm_set_type(ctx, type);
  pnm_set_width(ctx, width);
  pnm_set_height(ctx, height);
  pnm_set_maxval(ctx, maxval);
  pnm_initialize(ctx);
  
  return ctx;
}


void pnm_initialize (pnm_context_t *ctx) {
  ctx->canvas = (pnm_pixel_t *) calloc(ctx->width * ctx->height,
                                       sizeof(pnm_pixel_t));
  if (ctx->canvas == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
    pnm_error_exit(ctx, PNMERR_MEMORY_FAILURE);
  }
}


void pnm_destroy (pnm_context_t *ctx) {
  unsigned int i;
  
  i = 0;
  while (i < ctx->__comm_q) {
    free(*(ctx->comments + i));
    *(ctx->comments + i) = NULL;
    i++;
  }
  
  free(ctx->comments);
  ctx->comments = NULL;
  
  free(ctx->canvas);
  ctx->canvas = NULL;
  
  free(ctx);
  ctx = NULL;
}


void pnm_set_type (pnm_context_t *ctx, pnm_image_t t) {
  ctx->type = t;
}


void pnm_set_width (pnm_context_t *ctx, unsigned int w) {
  if (!pnm_in_range(w, 1, PNM_WIDTH_MAX_VALUE)) {
    fprintf(stderr,
            "Width value `%u' must be between 1 and "
            "PNM_WIDTH_MAX_VALUE (%d).\n",
            w, PNM_WIDTH_MAX_VALUE);
    pnm_error_exit(ctx, PNMERR_LIMIT_OVERFLOW);
  }
  
  ctx->width = w;
}


void pnm_set_height (pnm_context_t *ctx, unsigned int h) {
  if (!pnm_in_range(h, 1, PNM_WIDTH_MAX_VALUE)) {
    fprintf(stderr,
            "Height value `%u' must be between 1 and "
            "PNM_WIDTH_MAX_VALUE (%d).\n",
            h, PNM_WIDTH_MAX_VALUE);
    pnm_error_exit(ctx, PNMERR_LIMIT_OVERFLOW);
  }
  
  ctx->height = h;
}


void pnm_set_maxval (pnm_context_t *ctx, unsigned int m) {
  if (!pnm_in_range(m, 1, PNM_WIDTH_MAX_VALUE)) {
    fprintf(stderr,
            "Maxval value `%u' must be between 1 and "
            "PNM_WIDTH_MAX_VALUE (%d).\n",
            m, PNM_WIDTH_MAX_VALUE);
    pnm_error_exit(ctx, PNMERR_LIMIT_OVERFLOW);
  }
  
  ctx->maxval = m;
}


void pnm_add_comment (pnm_context_t *ctx, const char *comment) {
  char *token, *cp;
  cp = (char *) malloc(sizeof(char) * (strlen(comment) + 1));
  strcpy(cp, comment);
  token = strtok(cp, "\n");
  
  while (token != NULL) {
    ctx->comments = (char **) realloc(ctx->comments,
                                      sizeof(char *) * (ctx->__comm_q + 1));
    if (ctx->comments == NULL) {
      fprintf(stderr, "Memory allocation failed.\n");
      pnm_error_exit(ctx, PNMERR_MEMORY_FAILURE);
    }
    *(ctx->comments + ctx->__comm_q) = (char *) malloc(sizeof(char) *
                                                       (strlen(token) + 1));
    strcpy(*(ctx->comments + ctx->__comm_q), token);
    ++ctx->__comm_q;
    token = strtok(NULL, "\n");
  }
  
  free(token);
  free(cp);
  token = NULL;
  cp = NULL;
}


void pnm_replace_comment (pnm_context_t *ctx,
                          unsigned int pos,
                          const char *comment)
{
  if (pos >= ctx->__comm_q) {
    fprintf(stderr, "Trying to replace inexistant comment.\n");
    pnm_error_exit(ctx, PNMERR_VALUE_OUT_OF_RANGE);
  }
  *(ctx->comments + pos) = (char *) realloc(*(ctx->comments + pos),
                                            strlen(comment) + 1);
  strcpy(*(ctx->comments + pos), comment);
}


void pnm_delete_comment (pnm_context_t *ctx, unsigned int pos) {
  char *cp;
  cp = *(ctx->comments + pos);
  
  if (pos >= ctx->__comm_q) {
    fprintf(stderr, "Trying to delete inexistant comment.\n");
    pnm_error_exit(ctx, PNMERR_VALUE_OUT_OF_RANGE);
  }
  
  if (pos < ctx->__comm_q - 1)
    memmove(ctx->comments + pos,
            ctx->comments + pos + 1,
            sizeof(char *) * (ctx->__comm_q - pos));
  
  *(ctx->comments + ctx->__comm_q - 1) = cp;
  free(*(ctx->comments + ctx->__comm_q - 1));
  *(ctx->comments + ctx->__comm_q - 1) = NULL;
  --ctx->__comm_q;
  ctx->comments = (char **) realloc(ctx->comments,
                                    sizeof(char *) * ctx->__comm_q);
}


/* ==========================================================================
   PNM PIXEL MANIPULATION
   ========================================================================== */


pnm_pixel_t pnm_pixel_get (pnm_context_t *ctx,
                           unsigned int x,
                           unsigned int y)
{
  if (x >= ctx->width) {
    fprintf(stderr,
            "`x' pixel coordinate is out of range "
            "(must be between 0 and %u, current value is `%u').\n",
            ctx->width - 1, x);
    pnm_error_exit(ctx, PNMERR_VALUE_OUT_OF_RANGE);
  }
  if (y >= ctx->height) {
    fprintf(stderr,
            "`y' pixel coordinate is out of range "
            "(must be between 0 and %u, current value is `%u').\n",
            ctx->height - 1, y);
    pnm_error_exit(ctx, PNMERR_VALUE_OUT_OF_RANGE);
  }
  
  return *(ctx->canvas + y * ctx->width + x);
}


void pnm_pixel_set (pnm_context_t *ctx,
                    unsigned int x,
                    unsigned int y,
                    pnm_pixel_t color)
{
  if (x >= ctx->width) {
    fprintf(stderr,
            "`x' pixel coordinate is out of range "
            "(must be between 0 and %u and current value is `%u').\n",
            ctx->width - 1, x);
    pnm_error_exit(ctx, PNMERR_VALUE_OUT_OF_RANGE);
  }
  if (y >= ctx->height) {
    fprintf(stderr,
            "`y' pixel coordinate is out of range "
            "(must be between 0 and %u and current value is `%u').\n",
            ctx->height - 1, y);
    pnm_error_exit(ctx, PNMERR_VALUE_OUT_OF_RANGE);
  }
  
  *(ctx->canvas + y * ctx->width + x) = color;
}


void pnm_set_color (pnm_context_t *ctx, pnm_pixel_t p) {
  unsigned int x, y;
  
  y = 0;
  while (y < ctx->height) {
    x = 0;
    while (x < ctx->width) {
      ctx->canvas[y*ctx->width + x] = p;
      x++;
    }
    y++;
  }
}


void pnm_pixel_negative (pnm_pixel_t *in) {
  in->r ^= 0xff;
  in->g ^= 0xff;
  in->b ^= 0xff;
}

void pnm_rgb2hsl (const pnm_pixel_t *in,
          pnm_hsl_pixel_t *out)
{
  double M, m, C;
  
  M = pnm_max(in->r, in->g, in->b);
  m = pnm_min(in->r, in->g, in->b);
  C = M - m;
  
  if (C == 0) {
    out->h = PNM_UNDEFINED;
  } else if (M == in->r) {
    out->h = pnm_fmod((in->g - in->b) / C, 6);
  } else if (M == in->b) {
    out->h = ((in->b - in->r) / C + 2);
  } else /* M == in->b */ {
    out->h = ((in->r - in->g) / C + 4);
  }
  
  /* Hue normalization */
  out->h /= 6.;
  
  out->l = (M + m) / 510.;
  if (C == 0) {
    out->s = 0;
  } else {
    out->s = C / (1. - pnm_fabs(2.*out->l - 1.)) / 255.;
  }
}


void pnm_hsl2rgb (const pnm_hsl_pixel_t *in,
          pnm_pixel_t *out)
{
  double C, H, X, h, m, r, g, b;
  
  C = (1. - pnm_fabs(2.*in->l - 1)) * in->s;
  if (in->h == PNM_UNDEFINED) {
    r = 0;
    g = 0;
    b = 0;
  } else {
    H = in->h == 1 ? 0 : in->h * 6.;
    h = H > 4 ? H - 4 : H > 2 ? H - 2 : H;
    X = C * (1. - pnm_fabs(h - 1.));
    if (0 <= H && H < 1) {
      r = C;
      g = X;
      b = 0;
    } else if (1 <= H && H < 2) {
      r = X;
      g = C;
      b = 0;
    } else if (2 <= H && H < 3) {
      r = 0;
      g = C;
      b = X;
    } else if (3 <= H && H < 4) {
      r = 0;
      g = X;
      b = C;
    } else if (4 <= H && H < 5) {
      r = X;
      g = 0;
      b = C;
    } else /* 5 <= H && H < 6 */ {
      r = C;
      g = 0;
      b = X;
    }
  }
  
  m = in->l - .5*C;
  out->r = 255. * (r + m);
  out->g = 255. * (g + m);
  out->b = 255. * (b + m);
}


/* ==========================================================================
   PNM INPUT/OUTPUT
   ========================================================================== */


pnm_context_t * pnm_create_from_file (const char *filename) {
  FILE *fp;
  pnm_context_t *ctx;
  char c, *buffer;
  unsigned int w, h, m, ws;
  unsigned long bPos, ePos, dPos;
  
  ctx = (pnm_context_t *) malloc(sizeof(pnm_context_t));
  ctx->__comm_q = 0;
  ctx->comments = NULL;
  ctx->canvas = NULL;
  
  buffer = NULL;
  ws = 0;
  fp = fopen(filename, "r");
  
  c = getc(fp);
  if (c != 'P') {
    pnm_error_exit(ctx, PNMERR_WRONG_HEADER);
  }
  c = getc(fp);
  switch (c) {
    case '1': pnm_set_type(ctx, PNM_ASCII_BIT); break;
    case '2': pnm_set_type(ctx, PNM_ASCII_GREY); break;
    case '3': pnm_set_type(ctx, PNM_ASCII_RGB); break;
    case '4': pnm_set_type(ctx, PNM_BIN_BIT); break;
    case '5': pnm_set_type(ctx, PNM_BIN_GREY); break;
    case '6': pnm_set_type(ctx, PNM_BIN_RGB); break;
    default: pnm_error_exit(ctx, PNMERR_WRONG_HEADER); break;
  }
  
  while (ws < 4) {
    c = getc(fp);
    if (isspace(c)) {
      ws++;
      do {
        c = getc(fp);
      } while (isspace(c));
      ungetc(c, fp);
    } else if (c != EOF) {
      if (c == '#') {
        bPos = ftell(fp);
        while (c != '\n' && c != EOF) {
          c = getc(fp);
        }
        ePos = ftell(fp);
        dPos = ePos - bPos;
        buffer = (char *) realloc(buffer, sizeof(char) * (dPos));
        fseek(fp, bPos, SEEK_SET);
        if (fread(buffer, sizeof(char), dPos - 1, fp) != dPos - 1) {
          free(buffer);
          buffer = NULL;
          pnm_error_exit(ctx, PNMERR_READ_FAILURE);
        }
        buffer[dPos - 1] = '\0';
        pnm_add_comment(ctx, buffer);
        c = getc(fp);
      } else {
        ungetc(c, fp);
        switch (ws) {
          case 1: fscanf(fp, "%u", &w); break;
          case 2: fscanf(fp, "%u", &h); break;
          case 3: fscanf(fp, "%u", &m); break;
        }
      }
    } else {
      break;
    }
  }
  
  free(buffer);
  buffer = NULL;
  
  pnm_set_width(ctx, w);
  pnm_set_height(ctx, h);
  pnm_set_maxval(ctx, m);
  pnm_initialize(ctx);
  
  if (ctx->type <= 3)
    pnm_read_ascii(ctx, fp);
  else
    pnm_read_bin(ctx, fp);
  
  fclose(fp);
  
  return ctx;
}


static void pnm_read_ascii (pnm_context_t *ctx, FILE *fp) {
  unsigned int r, g, b, y, x;
  pnm_pixel_t pixel;
  
  y = 0;
  x = 0;
  
  fscanf(fp, "%u%u%u", &r, &g, &b);
  while (!feof(fp)) {
    pixel.r = r % 256;
    pixel.g = g % 256;
    pixel.b = b % 256;
    pnm_pixel_set(ctx, x, y, pixel);
    x = (x + 1) % ctx->width;
    if (x == 0) {
      y++;
    }
    fscanf(fp, "%u%u%u", &r, &g, &b);
  }
}


static void pnm_read_bin (pnm_context_t *ctx, FILE *fp) {
  unsigned int y, x, r, s;
  pnm_pixel_t *pixel_row;
  
  y = 0;
  pixel_row = (pnm_pixel_t *) malloc(sizeof(pnm_pixel_t) * ctx->width);
  
  while (ctx->height - y > 0) {
    r = fread(pixel_row, sizeof(pnm_pixel_t), ctx->width, fp);
    if (r != ctx->width) {
      s = r;
      while (ctx->width - s > 1) {
        r = fread(pixel_row,
              sizeof(pnm_pixel_t),
              ctx->width - s,
              fp);
        s += r;
      }
    }
    x = 0;
    while (x < ctx->width) {
      pnm_pixel_set(ctx, x, y, pixel_row[x]);
      x++;
    }
    y++;
  }
  if (y != ctx->height) {
    fprintf(stderr,
        "Reading error (%u of %u rows readed).\n",
        y,
        ctx->height);
    pnm_error_exit(ctx, PNMERR_READ_FAILURE);
  }
  
  free(pixel_row);
  pixel_row = NULL;
}


void pnm_write_image (pnm_context_t *ctx, const char *filename) {
  if (ctx->type <= 3) {
    if (!pnm_write_ascii(ctx, filename))
      pnm_error_exit(ctx, PNMERR_OUTPUT_FILE_FAILURE);
  } else {
    if (!pnm_write_bin(ctx, filename))
      pnm_error_exit(ctx, PNMERR_OUTPUT_FILE_FAILURE);
  }
}


static pnm_bool_t pnm_write_bin (pnm_context_t *ctx,
                                 const char *filename)
{
  FILE *fp;
  unsigned int j, bw, t;
  
  fp = fopen(filename, "wb");
  t = 0;
  
  if (fp == NULL) {
    fprintf(stderr, "An error ocurred opening file `%s'.\n", filename);
    return PNM_FALSE;
  }
  
  fprintf(fp, "P%d\n", ctx->type);
  j = 0;
  while (j < ctx->__comm_q) {
    fprintf(fp, "#%s\n", *(ctx->comments + j));
    j++;
  }
  fprintf(fp, "%u %u\n%u\n", ctx->width, ctx->height, ctx->maxval);
  
  j = 0;
  while ((bw = fwrite(ctx->canvas + j * ctx->width,
                      sizeof(pnm_pixel_t),
                      ctx->width,
                      fp)
         ) == ctx->width)
  {
    t += bw;
    j++;
    if (j == ctx->height)
      break;
  }
  
  if (t * 3 != sizeof(pnm_pixel_t) * ctx->width * ctx->height) {
    fprintf(stderr,
            "An error ocurred writing to file (%d of %lu bytes written).\n",
            t * 3,
            sizeof(pnm_pixel_t) * ctx->width * ctx->height);
    fclose(fp);
    return PNM_FALSE;
  }
  
  fclose(fp);
  
  return PNM_TRUE;
}


static pnm_bool_t pnm_write_ascii (pnm_context_t *ctx,
                                   const char *filename)
{
  FILE *fp;
  pnm_pixel_t p;
  unsigned int j;
  
  fp = fopen(filename, "w");
  
  if (fp == NULL) {
    fprintf(stderr, "An error ocurred opening file `%s'.\n", filename);
    return PNM_FALSE;
  }
  
  fprintf(fp, "P%d\n", ctx->type);
  j = 0;
  while (j < ctx->__comm_q) {
    fprintf(fp, "#%s\n", *(ctx->comments + j));
    j++;
  }
  fprintf(fp, "%u %u\n%u\n", ctx->width, ctx->height, ctx->maxval);
  
  j = 0;
  while (j < ctx->width * ctx->height) {
    p = *(ctx->canvas + j);
    fprintf(fp, "%3u %3u %3u ", p.r, p.g, p.b);
    if ((j + 1) % ctx->width == 0)
      fprintf(fp, "\n");
    j++;
  }
  
  fclose(fp);
  
  return PNM_TRUE;
}
