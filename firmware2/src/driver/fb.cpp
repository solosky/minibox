#include "fb.h"
#include <SPI.h>
#include <HardwareSerial.h>



#define COE 41
#define CBIT0 44
#define CBIT1 43
#define CBIT2 42
#define BLANK 3
#define GSLCK 15
#define MODE 13
#define XLAT 12
#define DHT11 7

#define _fb_pin_blank_set() bitSet(PORTE, 3)
#define _fb_pin_blank_clear() bitClear(PORTE, 3)
#define _fb_pin_cbit0_write(v) bitWrite(PORTA, 0, (v))
#define _fb_pin_cbit1_write(v) bitWrite(PORTA, 1, (v))
#define _fb_pin_cbit2_write(v) bitWrite(PORTA, 2, (v))
#define _fb_pin_coe_set() bitSet(PORTA, 3)
#define _fb_pin_coe_clear() bitClear(PORTA, 3)
#define _fb_pin_mode_set() bitSet(PORTB, 5)
#define _fb_pin_mode_clear() bitClear(PORTB, 5)
#define _fb_pin_xlat_set() bitSet(PORTB, 4)
#define _fb_pin_xlat_clear() bitClear(PORTB, 4)

fb_t _fb = {
        64, // w
        8, // h
        0, // gcnt
        0, // fcnt
        {0}, // mem
        {
         {{0, 1} /*DR*/, {0, 0} /*DG*/},   {{8, 0} /*DB*/, {0, 2} /*CR*/},
         {{8, 2} /*CG*/, {8, 1} /*CB*/},   {{16, 1} /*BR*/, {16, 0} /*BG*/},
         {{24, 0} /*BB*/, {16, 2} /*AR*/}, {{24, 2} /*AG*/, {24, 1} /*AB*/}
}   // row, offset, color RA,GA,BA, RB, GB, BB, RC,GC,BC, RD,GD,BD
};


fb_t* fb_default(){
        return &_fb;
}

void fb_init(fb_t* fb){

        pinMode(COE, OUTPUT);
        pinMode(CBIT0, OUTPUT);
        pinMode(CBIT1, OUTPUT);
        pinMode(CBIT2, OUTPUT);
        pinMode(MODE, OUTPUT);
        pinMode(XLAT, OUTPUT);
        pinMode(BLANK, OUTPUT);
        pinMode(GSLCK, OUTPUT);

        SPI.setClockDivider(SPI_CLOCK_DIV2);
        SPI.setDataMode(SPI_MODE3);
        SPI.setBitOrder(MSBFIRST);

        SPI.begin();

        _fb_pin_mode_clear();
        _fb_pin_coe_set();

        for(uint8_t i=0; i<8; i++){
          for(uint8_t j=0; j<32; j++){
            fb->mem[i][j] = 0xFF00FF00;
          }
        }
}

void fb_display(fb_t* fb){
}


void fb_tick(fb_t* fb){
        _fb_pin_coe_set();
        for (uint8_t i = 0; i < 8; i++) {
                //gray data
                _fb_flush_gray(fb, i);

                // dot crecction
                _fb_flush_dot(fb, i);

                // select line
                _fb_pin_cbit0_write((i & 1) > 0);
                _fb_pin_cbit1_write((i & 2) > 0);
                _fb_pin_cbit2_write((i & 4) > 0);

                // set timer
                _fb_set_gtimer(fb);

                //watting done
                while(fb->gcnt < 1) {; }
        }
        _fb_pin_coe_clear();
        fb->fcnt++;
}

void _fb_flush_gray(fb_t* fb, uint8_t row) {
        _fb_pin_xlat_clear();
        _fb_pin_mode_clear();

        uint8_t buff[24];
        for (uint8_t a = 0; a < 6; a++) {
                _fb_fill_192bit_color(fb, row, fb->spec[a][0][0], fb->spec[a][0][1], buff);
                _fb_fill_192bit_color(fb, row, fb->spec[a][1][0], fb->spec[a][1][1], buff + 12);

                SPI.transfer(buff, sizeof(buff));

                _fb_pin_xlat_set();
                _fb_pin_xlat_clear();
        }
}

void _fb_flush_dot(fb_t* fb, uint8_t row) {
        _fb_pin_xlat_clear();
        _fb_pin_mode_set();
        uint8_t dot[12];
        for (uint8_t a = 0; a < 6; a++) {
                _fb_fill_96bit_dot(fb, row, fb->spec[a][0][0], dot);
                _fb_fill_96bit_dot(fb, row, fb->spec[a][1][0], dot + 6);

                SPI.transfer(dot, sizeof(dot));
                _fb_pin_xlat_set();
                _fb_pin_xlat_clear();

        }
}


void _fb_set_gtimer(fb_t* fb){

        fb->gcnt = 0;

        TCCR2 = 1 << COM20 | 1 << CS20 | 1 << WGM21;
        TCNT2 = 0;
        OCR2 = 0;

        //TCCR3A = 1 << COM3A0;
        TCCR3A = 0;
        OCR3A = 0x1000 * 2;
        TCCR3C = 0;
        TCNT3 = 0;
        ETIMSK = 1 << OCIE3A;
        TCCR3B = 1 << CS30 | 1 << WGM32;

        _fb_pin_blank_clear();
}


void _fb_fill_192bit_color(fb_t* fb, uint8_t row, uint8_t offset, uint8_t clr, uint8_t buff[]){
  uint32_t *p = fb->mem[row] + offset + 7;
  for (uint8_t i = 0; i < 12; i += 3) {
    uint16_t b1 = (((*(p)) >> (16 - clr * 8)) & 0xFF) * 16;
    uint16_t b2 = (((*(p - 1)) >> (16 - clr * 8)) & 0xFF) * 16;
    buff[i + 0] = b1 >> 4;
    buff[i + 1] = (0xF & b1) << 4 | b2 >> 8;
    buff[i + 2] = 0xFF & b2;
    p -= 2;
  }
}

void _fb_fill_96bit_dot(fb_t* fb, uint8_t row, uint8_t offset, uint8_t buff[]){
  uint32_t *p = fb->mem[row] + offset + 7;
  for (uint8_t i = 0; i < 6; i += 3) {
    uint8_t b1 = ((*p) >> 24) & 0x3F;
    uint8_t b2 = ((*(p - 1)) >> 24) & 0x3F;
    uint8_t b3 = ((*(p - 2)) >> 24) & 0x3F;
    uint8_t b4 = ((*(p - 3)) >> 24) & 0x3F;

    buff[i + 0] = b1 << 2 | b2 >> 4;
    buff[i + 1] = b2 << 4 | b3 >> 2;
    buff[i + 2] = b3 << 6 | b4;
    p -= 4;
  }
}


ISR(TIMER3_COMPA_vect) {
        _fb_pin_blank_set();
        bitClear(TCCR3A, CS30);
        bitClear(TCCR2, CS20);
        _fb.gcnt++;
}
