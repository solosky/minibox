#include "driver/spi_drv.h"
#include <arduino.h>
#include <HardwareSerial.h>

#include "driver/fb.h"

int serial_putc(char c, struct __file *) {
  Serial.write(c);
  return c;
}


void setup() {
        Serial.begin(115200);
        fdevopen(&serial_putc, 0);

        fb_init(fb_default());
        //_fb_set_gtimer(fb_default());
}

void loop() {
  fb_tick(fb_default());
}
