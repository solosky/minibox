#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include "io_base.h"

//frame buffer defines
typedef struct __fb__{
    uint8_t* mem; // display memory mapping
    uint8_t w;  // width pixels
    uint8_t h; // height pixels
    uint8_t br; // bright level
    uint8_t volatile gcnt; // gray sclk count..
    uint8_t fcnt; // frame count..
} fb_t;


///////////////////////////////////////////////////////////////////////////
/// public functions
///////////////////////////////////////////////////////////////////////////
void fb_init(fb_t* fb);
void fb_display(fb_t* fb);
void fb_tick(fb_t* fb);
fb_t* fb_default();


///////////////////////////////////////////////////////////////////////////
/// private functions
///////////////////////////////////////////////////////////////////////////
void _fb_set_gtimer(fb_t* fb);
void _fb_flush_gray(fb_t* fb, uint8_t row);
void _fb_flush_dot(fb_t* fb, uint8_t row);
void _fb_fill_gray(fb_t* fb, byte_t * buff, uint8_t row, uint8_t bucket);
void _fb_fill_dot(fb_t* fb, byte_t* buff, uint8_t row, uint8_t bucket);



#endif
