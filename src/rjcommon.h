/* R-related tools (mapping of jpeg error handling to R)
   common to all tasks */

#ifndef R_J_COMMON_H
#define R_J_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>

#include "jcompat.h"

#if (BITS_IN_JSAMPLE != 8)
#error "Sorry, only 8-bit libjpeg is supported"
#endif

#define USE_RINTERNALS 1
#include <Rinternals.h>
/* for R_RGB / R_RGBA */
#include <R_ext/GraphicsEngine.h>

METHODDEF(void)
Rjpeg_error_exit(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    
    (*cinfo->err->format_message) (cinfo, buffer);
    Rf_error("JPEG decompression error: %s", buffer);
}

METHODDEF(void)
Rjpeg_output_message (j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    
    (*cinfo->err->format_message) (cinfo, buffer);
    REprintf("JPEG decompression: %s", buffer);
}

struct Rjpeg_error_mgr {
    struct jpeg_error_mgr api;
    void *mem; /* additional memory that will be free()d eventually */
    unsigned long size;  /* arbitrary value that is usually used as buffer size */
};

#define Rjpeg_mem_ptr(CINFO) (((struct Rjpeg_error_mgr*)(CINFO->err))->mem)
#define Rjpeg_mem_size(CINFO) (((struct Rjpeg_error_mgr*)(CINFO->err))->size)

static void Rjpeg_fin(SEXP dco) {
    struct jpeg_common_struct *cinfo = (struct jpeg_common_struct*) R_ExternalPtrAddr(dco);
    if (cinfo) {
	struct Rjpeg_error_mgr *jerr;
        jpeg_destroy(cinfo);
	if ((jerr = (struct Rjpeg_error_mgr *) cinfo->err)) {
	    if (jerr->mem)
		free(jerr->mem);
	    free(jerr);
	}
        free(cinfo);
    }
    /* make it a NULL ptr in case this was not a finalizer call */
    CAR(dco) = 0;
}

static struct jpeg_error_mgr *Rjpeg_new_err() {
    struct jpeg_error_mgr *jerr = (struct jpeg_error_mgr*) calloc(sizeof(struct Rjpeg_error_mgr), 1);
    if (!jerr) Rf_error("Unable to allocate jpeg error management structure");
    jpeg_std_error(jerr);
    jerr->error_exit = Rjpeg_error_exit;
    jerr->output_message = Rjpeg_output_message;
    return jerr;
}

#endif