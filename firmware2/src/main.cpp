#include "driver/spi_drv.h"
#include <arduino.h>
#include <HardwareSerial.h>

#include "driver/fb.h"

int serial_putc(char c, struct __file *) {
  Serial.write(c);
  return c;
}

void print_frame_buffer() {
  char s[12] = {0};
  fb_t* fb = fb_default();
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 32; j++) {
      //      sprintf(s, "%.8X ", frame_buffer[i][j]);
      //   Serial.print(s);
      Serial.print(fb->mem[i][j], 16);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void setup() {
        Serial.begin(115200);
        fdevopen(&serial_putc, 0);

        fb_init(fb_default());
        print_frame_buffer();

        //_fb_set_gtimer(fb_default());
}



void loop() {
  fb_tick(fb_default());
}
