#include "fb.h"
#include <SPI.h>



#define COE 41
#define CBIT0 44
#define CBIT1 43
#define CBIT2 42
#define BLANK 3
#define GSLCK 15
#define MODE 13
#define XLAT 12
#define DHT11 7

#define BLANK_HIGH() bitSet(PORTE, 3)
#define BLANK_LOW() bitClear(PORTE, 3)
#define SET_CBIT0(v) bitWrite(PORTA, 0, (v))
#define SET_CBIT1(v) bitWrite(PORTA, 1, (v))
#define SET_CBIT2(v) bitWrite(PORTA, 2, (v))
#define COE_HIGH() bitSet(PORTA, 3)
#define COE_LOW() bitClear(PORTA, 3)
#define MODE_HIGH() bitSet(PORTB, 5)
#define MODE_LOW() bitClear(PORTB, 5)
#define XLAT_HIGH() bitSet(PORTB, 4)
#define XLAT_LOW() bitClear(PORTB, 4)

fb_t _fb;

fb_t* fb_default(){
        return &_fb;
}

void fb_init(fb_t* fb){
        fb->h = 8;
        fb->w = 64;
        fb->br = 10;

        pinMode(COE, OUTPUT);
        pinMode(CBIT0, OUTPUT);
        pinMode(CBIT1, OUTPUT);
        pinMode(CBIT2, OUTPUT);
        pinMode(MODE, OUTPUT);
        pinMode(XLAT, OUTPUT);
        pinMode(BLANK, OUTPUT);
        pinMode(GSLCK, OUTPUT);

      SPI.setClockDivider(SPI_CLOCK_DIV2);
      SPI.setDataMode(SPI_MODE0);
      SPI.setBitOrder(MSBFIRST);

      SPI.begin();

        MODE_LOW();
        COE_HIGH();
}

void fb_display(fb_t* fb){
}


void fb_tick(fb_t* fb){
  COE_HIGH();
  for (uint8_t i = 0; i < 8; i++) {
          //gray data
          _fb_flush_gray(fb, i);

          // dot crecction
          //if((fb->fcnt & 0x0F) == 0){
            _fb_flush_dot(fb, i);
          //}

          // select line
          SET_CBIT0((i & 1) > 0);
          SET_CBIT1((i & 2) > 0);
          SET_CBIT2((i & 4) > 0);

          // set timer
          fb->gcnt = 0;
          _fb_set_gtimer(fb);

          //watting done
          while(fb->gcnt < 1){
            ;
          }
    }
  COE_LOW();
  fb->fcnt++;
}

void _fb_flush_gray(fb_t* fb, uint8_t row) {
        XLAT_LOW();
        MODE_LOW();

        uint8_t buff[24];
        for (uint8_t a = 0; a < 6; a++) {
                _fb_fill_gray(fb, buff, row, a);
                SPI.transfer(buff, sizeof(buff));
                XLAT_HIGH();
                XLAT_LOW();
        }
}

void _fb_flush_dot(fb_t* fb, uint8_t row) {
        XLAT_LOW();
        MODE_HIGH();
        uint8_t dot[12];
        for (char a = 0; a < 6; a++) {
                _fb_fill_dot(fb, dot, row, a);
                SPI.transfer(dot, sizeof(dot));
                XLAT_HIGH();
                XLAT_LOW();
        }
}



void _fb_set_gtimer(fb_t* fb){
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

        BLANK_LOW();
}

void _fb_fill_gray(fb_t* fb, byte_t * buff, uint8_t row, uint8_t bucket){
        //TODO ..
        memset(buff, 0x18, 24);
}
void _fb_fill_dot(fb_t* fb, byte_t* buff, uint8_t row, uint8_t bucket){
        //TODO ..
        memset(buff, 0xFF, 12);
}


ISR(TIMER3_COMPA_vect) {
         BLANK_HIGH();
        bitClear(TCCR3A, CS30);
        bitClear(TCCR2, CS20);
        _fb.gcnt++;
}
