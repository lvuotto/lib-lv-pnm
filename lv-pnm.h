
/* pnm.h */


#ifndef __LV_PNM_LIB_INCLUDED__

#define __LV_PNM_LIB_INCLUDED__


#include "lv-pnm-types.h"


/* #  ~ .lV   ! */

/* Prototypes */


/* ==========================================================================
   ERROR MANIPULATION
   ========================================================================== */

void            pnm_error_exit            (pnm_context_t *ctx, int code);


/* ==========================================================================
 * PNM MANIPULATION
 * ========================================================================== */

pnm_context_t * pnm_create                (pnm_image_t  type,
                                           unsigned int width,
                                           unsigned int height,
                                           unsigned int maxval);
void            pnm_initialize            (pnm_context_t *ctx);
void            pnm_destroy               (pnm_context_t *ctx);
void            pnm_set_type              (pnm_context_t *ctx, pnm_image_t t);
void            pnm_set_width             (pnm_context_t *ctx, unsigned int w);
void            pnm_set_height            (pnm_context_t *ctx, unsigned int h);
void            pnm_set_maxval            (pnm_context_t *ctx, unsigned int m);
void            pnm_add_comment           (pnm_context_t *ctx,
                                           const char *comment);
void            pnm_replace_comment       (pnm_context_t *ctx,
                                           unsigned int pos,
                                           const char *comment);
void            pnm_delete_comment        (pnm_context_t *ctx,
                                           unsigned int pos);


/* ==========================================================================
 * PNM PIXEL MANIPULATION
 * ========================================================================== */

pnm_pixel_t     pnm_pixel_get             (pnm_context_t *ctx,
                                           unsigned int x,
                                           unsigned int y);
void            pnm_pixel_set             (pnm_context_t *ctx,
                                           unsigned int x,
                                           unsigned int y,
                                           pnm_pixel_t color);
void            pnm_set_color             (pnm_context_t *ctx, pnm_pixel_t p);
void            pnm_pixel_negative        (pnm_pixel_t *in);
void            pnm_rgb2hsl               (const pnm_pixel_t *in,
                                           pnm_hsl_pixel_t *out);
void            pnm_hsl2rgb               (const pnm_hsl_pixel_t *in,
                                           pnm_pixel_t *out);


/* ==========================================================================
 * PNM INPUT/OUTPUT
 * ========================================================================== */

pnm_context_t * pnm_create_from_file      (const char *filename);
void            pnm_write_image           (pnm_context_t *ctx,
                                           const char *filename);

#endif      /*  __LV_PNM_LIB_INCLUDED__  */
