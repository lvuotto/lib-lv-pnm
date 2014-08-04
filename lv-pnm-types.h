
/* pnm_types.h */


#ifndef __LV_PNM_TYPES_H__

#define __PNM_TYPES_H__

#define PNM_MAXVAL_MAX_VALUE 255 /* 65535 */
#define PNM_WIDTH_MAX_VALUE 65535
#define PNM_HEIGHT_MAX_VALUE 65535


typedef struct __PNM_COLOR pnm_pixel_t;
typedef struct __PNM_HSL_PIXEL pnm_hsl_pixel_t;
typedef struct __PNM_CONTEXT pnm_context_t;

enum __PNM_BOOL_T {
  PNM_UNDEFINED              = -1,
  PNM_FALSE                  =  0,
  PNM_TRUE                   =  1
};

enum __PNM_IMAGE_TYPE {
  PNM_ASCII_BIT              = 1,
  PNM_ASCII_GREY             = 2,
  PNM_ASCII_RGB              = 3,
  PNM_BIN_BIT                = 4,
  PNM_BIN_GREY               = 5,
  PNM_BIN_RGB                = 6
};

typedef enum __PNM_BOOL_T pnm_bool_t;
typedef enum __PNM_IMAGE_TYPE pnm_image_t;

enum __PNM_ERROR {
  PNMERR_LIMIT_OVERFLOW      = 1,
  PNMERR_VALUE_OUT_OF_RANGE  = 2,
  PNMERR_MEMORY_FAILURE      = 3,
  PNMERR_OUTPUT_FILE_FAILURE = 4,
  PNMERR_WRONG_HEADER        = 5,
  PNMERR_READ_FAILURE        = 6
};

struct __PNM_COLOR {
  unsigned char  r;
  unsigned char  g;
  unsigned char  b;
};

struct __PNM_HSL_PIXEL {
  double         h;
  double         s;
  double         l;
};

struct __PNM_CONTEXT {
  pnm_image_t    type;

  unsigned int   width;
  unsigned int   height;
  unsigned int   maxval;

  pnm_pixel_t   *canvas;

  char         **comments;
  unsigned int   __comm_q; /* "private" */
};


#endif  /*  __LV_PNM_TYPES_H__  */
