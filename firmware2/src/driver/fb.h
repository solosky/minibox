#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include "io_base.h"

#define uint32 unsigned long

//frame buffer defines
typedef struct __fb__{
    uint8_t w;  // width pixels
    uint8_t h; // height pixels
    uint8_t volatile gcnt; // gray sclk count..
    uint8_t fcnt; // frame count..

    uint32_t mem[8][32]; //frame mem buffer, in argb format
    uint8_t  spec[6][2][2]; //driver specs, meta info for rgb display
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
void _fb_wait_gtimer(fb_t* fb);
void _fb_flush_gray(fb_t* fb, uint8_t row);
void _fb_flush_dot(fb_t* fb, uint8_t row);

void _fb_fill_192bit_color(fb_t* fb, uint8_t row, uint8_t offset, uint8_t clr, uint8_t buff[]);
void _fb_fill_96bit_dot(fb_t* fb, uint8_t row, uint8_t offset, uint8_t buff[]);



#endif
