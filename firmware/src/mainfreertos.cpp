#include <Arduino_FreeRTOS.h>
#include <DS3231.h>
#include <SPI.h>
#include <dht11.h>
#include <semphr.h>
#include <wire.h>

#define COE 41
#define CBIT0 44
#define CBIT1 43
#define CBIT2 42
#define BLANK 3
#define GSLCK 15
#define MODE 13
#define XLAT 12
#define DHT11 7

#define AM2320_DATA_PIN 7

#define MAX_GSCLK_CYCLE 2

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

void init_draw();
void init_clock();
void init_wifi();
void init_freertos();

void frame_tick_task(void *param);
void read_rtc_clock_task(void *param);

int serial_putc(char c, struct __file *) {
  Serial.write(c);
  return c;
}
void printf_begin(void) { fdevopen(&serial_putc, 0); }

void setup() {
  pinMode(COE, OUTPUT);
  pinMode(CBIT0, OUTPUT);
  pinMode(CBIT1, OUTPUT);
  pinMode(CBIT2, OUTPUT);
  pinMode(MODE, OUTPUT);
  pinMode(XLAT, OUTPUT);
  pinMode(BLANK, OUTPUT);
  pinMode(GSLCK, OUTPUT);

  Serial.begin(115200);
  printf_begin();

  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();

  init_draw();
  init_clock();
  init_wifi();
  init_freertos();

  printf("init done~");
}

void set_gtimer(){
   // Timer2 GSCLK
  TCCR2 = 1 << COM20 | 1 << CS20 | 1 << WGM21;
  TCNT2 = 0;
  OCR2 = 0;
  // // TIMSK2 = 1 << OCIE2A;
  //
  //  // Timer1 BLANK
   //TCCR3A = 1 << COM3A0;
  TCCR3A = 0;
  TCCR3B = 1 << CS30 | 1 << WGM32;
  TCCR3C = 0;
  TCNT3 = 0;
  OCR3A = 0x1000*2;

  // OCR3BL = 0xFF;
  // OCR3BH = 0xFF;
  // OCR3CL = 0xFF;
  // OCR3CH = 0xFF;

  ETIMSK = 1 << OCIE3A;
}

uint8_t volatile gsclk_cnt = 0;

ISR(TIMER3_COMPA_vect) {
  gsclk_cnt++;
  bitClear(TCCR3B, CS30);
  bitClear(TCCR2, CS20);
  BLANK_HIGH();
  BLANK_LOW();
  TCNT2 = 0;
  TCNT3 = 0;
  if(gsclk_cnt < MAX_GSCLK_CYCLE){
  bitSet(TCCR2, CS20);
  bitSet(TCCR3B, CS30);
}
  // Serial.print("1");
}

///////////////////////////////////////////////////////////////
void init_wifi() {
  // 设置串口通讯的速率
  Serial1.begin(115200);

  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  digitalWrite(22, LOW);
  digitalWrite(23, LOW);
  delay(200);

  digitalWrite(22, HIGH);
  delay(100);
  digitalWrite(23, HIGH);
  delay(5);

  pinMode(31, OUTPUT);
}

///////////////////////////////////////////////////////////////
#define uint32 unsigned long
#define uint8 unsigned char
#define int8 char
#define uint16 unsigned int

void print_buffer(uint8 *buff, uint8 len) {
  uint8 *p = buff;
  uint8 c = 0;
  char s[6] = {0};
  while (p < buff + len) {
    sprintf(s, "%.2X ", (*p));
    Serial.print(s);
    // Serial.print(*p, HEX);
    // Serial.print(" ");
    if (((++c) % 8) == 0) {
      Serial.println();
    }
    p++;
  }
  Serial.println();
}

// argb格式显示
uint32 frame_buffer[8][32];
// row, offset, color RA,GA,BA, RB, GB, BB, RC,GC,BC, RD,GD,BD
uint32 driver_spec[6][2][2] = {
    {{0, 1} /*DR*/, {0, 0} /*DG*/},   {{8, 0} /*DB*/, {0, 2} /*CR*/},
    {{8, 2} /*CG*/, {8, 1} /*CB*/},   {{16, 1} /*BR*/, {16, 0} /*BG*/},
    {{24, 0} /*BB*/, {16, 2} /*AR*/}, {{24, 2} /*AG*/, {24, 1} /*AB*/}};

void fill_192bit_color(uint8 row, uint8 offset, uint8 clr, uint8 buff[]) {
  char s[40];
  //  Serial.println("++++++++++++++++++++++++++++++++++");
  //  sprintf(s, ">>>row=%d, offset=%d, clr=%d", row, offset, clr);
  //  Serial.println(s);
  uint32 *p = frame_buffer[row] + offset + 7;
  for (uint8 i = 0; i < 12; i += 3) {
    uint16 b1 = (((*(p)) >> (16 - clr * 8)) & 0xFF) * 16;
    uint16 b2 = (((*(p - 1)) >> (16 - clr * 8)) & 0xFF) * 16;

    //    Serial.print("*p=");
    //    Serial.print(*p, HEX);
    //    Serial.print(",*(p-1)=");
    //    Serial.print(*(p - 1), HEX);
    //    Serial.print(", b1=");
    //    Serial.print(b1, HEX);
    //    Serial.print(",b2=");
    //    Serial.println(b2, HEX);
    buff[i + 0] = b1 >> 4;
    buff[i + 1] = (0xF & b1) << 4 | b2 >> 8;
    buff[i + 2] = 0xFF & b2;
    p -= 2;
  }
}

void fill_96bit_dot(uint8 row, uint8 offset, uint8 buff[]) {
  uint32 *p = frame_buffer[row] + offset + 7;
  for (uint8 i = 0; i < 6; i += 3) {
    uint8 b1 = ((*p) >> 24) & 0x3F;
    uint8 b2 = ((*(p - 1)) >> 24) & 0x3F;
    uint8 b3 = ((*(p - 2)) >> 24) & 0x3F;
    uint8 b4 = ((*(p - 3)) >> 24) & 0x3F;

    buff[i + 0] = b1 << 2 | b2 >> 4;
    buff[i + 1] = b2 << 4 | b3 >> 2;
    buff[i + 2] = b3 << 6 | b4;
    p -= 4;
  }
}

void flush_frame_buffer(uint8 row) {
  XLAT_LOW();
  MODE_LOW();

  uint8 buff[24];
  // Serial.println("=============================");
  for (uint8 a = 0; a < 6; a++) {
    fill_192bit_color(row, driver_spec[a][0][0], driver_spec[a][0][1], buff);
    fill_192bit_color(row, driver_spec[a][1][0], driver_spec[a][1][1],
                      buff + 12);
    // print_buffer(buff, 24);
    SPI.transfer(buff, sizeof(buff));

    // Serial.println("----------------------");
    XLAT_HIGH();
    XLAT_LOW();
  }
}

void flush_dot_correction(uint8 row) {
  XLAT_LOW();
  MODE_HIGH();
  uint8 dot[12];
  for (char a = 0; a < 6; a++) {
    fill_96bit_dot(row, driver_spec[a][0][0], dot);
    fill_96bit_dot(row, driver_spec[a][1][0], dot + 6);
    SPI.transfer(dot, sizeof(dot));
    XLAT_HIGH();
    XLAT_LOW();
  }
}

void frame_tick() {
  COE_HIGH();
  for (uint8 i = 0; i < 8; i++) {

    flush_frame_buffer(i);
    flush_dot_correction(i);

    // select line
    SET_CBIT0((i & 1) > 0);
    SET_CBIT1((i & 2) > 0);
    SET_CBIT2((i & 4) > 0);

    BLANK_LOW();

    gsclk_cnt = 0;
    set_gtimer();

    while(gsclk_cnt < MAX_GSCLK_CYCLE){
      ;
    }

    BLANK_HIGH();

    // delay for display
    // delay(1000);
    // delayMicroseconds(800);
    // while(gsclk_cnt < 2){
    //     asm volatile ("nop");
    // }
    //gsclk_cnt = 0;
  }
  COE_LOW();
}

////////////////////////////////////////////////////////////
void draw_char(uint8 *font_data, uint8 offset, uint32 color) {
  for (uint8 i = 0; i < 7; i++) {
    uint8 line = font_data[i];
    for (uint8 j = 0; j < 5; j++) {
      if ((line & (1 << j)) > 0 && (offset + j) >= 0 && (offset + j) < 32) {
        frame_buffer[i][offset + j] = color;
      }
    }
  }
}

//////////////////////////////////////////////////////////////

void print_frame_buffer() {
  char s[12] = {0};
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 32; j++) {
      //      sprintf(s, "%.8X ", frame_buffer[i][j]);
      //   Serial.print(s);
      Serial.print(frame_buffer[i][j], 16);
      Serial.print(" ");
    }
    Serial.println();
  }
}

// uint8 c_A[] = {0x0C, 0x12, 0x12, 0x12, 0x1E, 0x12, 0x12, 0x00}; //A0
///* (8 X 8 , Terminal )*/
//
// uint8 c_B[] = {0x0E, 0x12, 0x12, 0x0E, 0x12, 0x12, 0x0E, 0x00}; //B1
///* (8 X 8 , Terminal )*/
//
// uint8 c_C[] = {0x0C, 0x12, 0x02, 0x02, 0x02, 0x12, 0x0C, 0x00}; //C2
///* (8 X 8 , Terminal )*/
//
// uint8 c_D[] = {0x0E, 0x12, 0x12, 0x12, 0x12, 0x12, 0x0E, 0x00}; //D3
///* (8 X 8 , Terminal )*/
//
// uint8 c_AA[] = {0x00, 0x00, 0xFC, 0x00, 0x48, 0x00, 0xFE, 0x01};
// uint8 c_AB[] = {0x48, 0x00, 0x48, 0x00, 0x44, 0x00, 0x42, 0x00}; //开0
//
// uint8 c_BLANK[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //A:0

// 1(0) 2(1) 3(2) 4(3) 5(4) 6(5) 7(6) 8(7)
// 9(8) 0(9) :(10) A(11) B(12) C(13) D(14)

uint8 c_1[] = {
    0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x07,
}; // 10
/* (5 X 7 , Terminal )*/

uint8 c_2[] = {
    0x06, 0x09, 0x08, 0x04, 0x02, 0x01, 0x0F,
}; // 21
/* (5 X 7 , Terminal )*/

uint8 c_3[] = {
    0x06, 0x09, 0x08, 0x06, 0x08, 0x09, 0x06,
}; // 32
/* (5 X 7 , Terminal )*/

uint8 c_4[] = {
    0x04, 0x06, 0x06, 0x05, 0x05, 0x0F, 0x04,
}; // 43
/* (5 X 7 , Terminal )*/

uint8 c_5[] = {
    0x0F, 0x01, 0x01, 0x07, 0x08, 0x08, 0x07,
}; // 54
/* (5 X 7 , Terminal )*/

uint8 c_6[] = {
    0x06, 0x09, 0x01, 0x07, 0x09, 0x09, 0x06,
}; // 65
/* (5 X 7 , Terminal )*/

uint8 c_7[] = {
    0x0F, 0x09, 0x08, 0x04, 0x02, 0x02, 0x02,
}; // 76
/* (5 X 7 , Terminal )*/

uint8 c_8[] = {
    0x06, 0x09, 0x09, 0x06, 0x09, 0x09, 0x06,
}; // 87
/* (5 X 7 , Terminal )*/

uint8 c_9[] = {
    0x06, 0x09, 0x09, 0x0E, 0x08, 0x09, 0x06,
}; // 98
/* (5 X 7 , Terminal )*/

uint8 c_0[] = {
    0x06, 0x09, 0x09, 0x09, 0x09, 0x09, 0x06,
}; // 09
/* (5 X 7 , Terminal )*/

uint8 c__m[] = {
    0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00,
}; //:10
/* (5 X 7 , Terminal )*/

uint8 c_A[] = {
    0x06, 0x09, 0x09, 0x09, 0x0F, 0x09, 0x09,
}; // A11
/* (5 X 7 , Terminal )*/

uint8 c_B[] = {
    0x07, 0x09, 0x09, 0x07, 0x09, 0x09, 0x07,
}; // B12
/* (5 X 7 , Terminal )*/

uint8 c_C[] = {
    0x06, 0x09, 0x01, 0x01, 0x01, 0x09, 0x06,
}; // C13
/* (5 X 7 , Terminal )*/

uint8 c_D[] = {
    0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x07,
}; // D14
/* (5 X 7 , Terminal )*/

uint8 c__q[] = {
    0x06, 0x09, 0x08, 0x04, 0x02, 0x00, 0x02,
}; //?0
/* (5 X 7 , Terminal )*/
uint8 c__s[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
}; // 0
/* (5 X 7 , Terminal )*/

uint8 *c_font_data[] = {c_1, c_2,  c_3,  c_4,  c_5, c_6, c_7, c_8, c_9,
                        c_0, c__m, c__q, c__s, c_A, c_B, c_C, c_D};
char c_font_index[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9',
                       '0', ':', '?', ' ', 'A', 'B', 'C', 'D'};

uint8 *find_font_data(char c) {
  for (uint8 i = 0; i < 16; i++) {
    if (c_font_index[i] == c) {
      return c_font_data[i];
    }
  }
  return 0;
}

void draw_str(char *str, uint8 offset, uint32 color) {
  for (uint8 i = 0; i < strlen(str) && offset < 32; i++, offset += 5) {
    uint8 *font_data = find_font_data(str[i]);
    if (font_data) {
      draw_char(font_data, offset, color);
    } else {
      draw_char(find_font_data('?'), offset, color);
    }
  }
}

void set_pixel(uint8_t x, uint8_t y, uint32_t color) {
  if (x < 0 || x > 32)
    return;
  if (y < 0 || y > 8)
    return;
  frame_buffer[y][x] = color;
}

void clear_display() { memset(frame_buffer, 0, sizeof(frame_buffer)); }

void init_draw() {
  //  //memset(frame_buffer, 1, sizeof(frame_buffer));
  // draw_chars(0);
  ////  draw_block(0, 0x0F10FF0F);
  ////      for(int i=0; i<8; i++){
  ////        for(int j=0; j<32; j++){
  ////            frame_buffer[i][j] =  (0x0f << 24) | (j*8)<<16 | (j*8 << 8) |
  ///(256 - j*8);
  ////        }
  ////    }
  ////   print_frame_buffer();
}

// void draw_chars(uint8 offset) {
//  char f = offset;
//  draw_str("12:00", f, 0x0F00FF00);
////  draw_char(c_A, 0 + f, 0x0F0000FF);
////  draw_char(c_B, 5 + f, 0x0F00FF00);
////  draw_char(c_C, 10 + f, 0x0FFF0000);
////  draw_char(c_D, 15 + f, 0x0FFFFFFF);
////  draw_char(c_1, 20 + f, 0x0FFFFFFF);
////  draw_char(c_2, 25 + f, 0x0FFFFFFF);
////  draw_char(c_3, 30 + f, 0x0FFFFFFF);
//}

void draw_block(uint8 b, uint32 color) {
  Serial.println(color, HEX);
  for (int i = 0; i < 8; i++) {
    for (int j = b; j < b + 8; j++) {
      frame_buffer[i][j] = color;
    }
  }
}

int c = 0;
int i = 0;

DS3231 Clock;
volatile uint8_t lux;
volatile uint8_t second;
volatile uint8_t minute;
volatile uint8_t hour;
volatile uint8_t temp;
volatile uint32_t color;

void ReadDS3231();
void init_clock() {

  // Start the I2C interface
  Wire.begin();
  //        Clock.setSecond(00);//Set the second
  //        Clock.setMinute(27);//Set the minute
  //        Clock.setHour(19);  //Set the hour
  //        //Clock.setDoW(9);    //Set the day of the week
  //        Clock.setDate(9);  //Set the date of the month
  //        Clock.setMonth(6);  //Set the month of the year
  //        Clock.setYear(16);  //Set the year (Last two digits of the year)
  // Start the serial interface
  Clock.setClockMode(false);
  Clock.enableOscillator(true, true, 0);
  // Clock.enable32kHz(true);
  ReadDS3231();

  EICRB = 1 << ISC61;
  EIMSK = 1 << INT6;
}
void ReadDS3231() {
  bool h12 = false;
  bool PM = false;

  second = Clock.getSecond();
  minute = Clock.getMinute();
  hour = Clock.getHour(h12, PM);
  temp = Clock.getTemperature();

  //  Serial.print("20");
  //  Serial.print(year,DEC);
  //  Serial.print('-');
  //  Serial.print(month,DEC);
  //  Serial.print('-');
  //  Serial.print(date,DEC);
  //  Serial.print(' ');
  //  Serial.print(hour,DEC);
  //  Serial.print(':');
  //  Serial.print(minute,DEC);
  //  Serial.print(':');
  //  Serial.print(second,DEC);
  //  Serial.print('\n');
  //  Serial.print("Temperature=");
  //  Serial.print(temperature);
  //  Serial.print('\n');
}

ISR(INT6_vect) { second++; }

bool am_int = false;
// StaticSemaphore_t am_sem;
SemaphoreHandle_t am_sem_handle;
dht11 _dht11;
bool _dht11_ok = false;
uint8_t display_type = 0;
uint8_t display_rotate = 0;
#define DISPLAY_TIME 0
#define DISPLAY_DHT11 1
ISR(INT7_vect) {
  if (am_int) {
  }
}

void loop() {
  //   c++;
  //   if (c == 20) {
  //    memset(frame_buffer, 0x0, sizeof(frame_buffer));
  //   }
  //   frame_tick();
  //   if(read){
  //     ReadDS3231();
  //     read = false;
  //   }
  //
  //   if (Serial1.available()){
  //   Serial.write(Serial1.read());
  //   digitalWrite(31, HIGH);   // turn the LED on (HIGH is the voltage level)
  // delay(1);              // wait for a second
  // digitalWrite(31, LOW);    // turn the LED off by making the voltage LOW
  // delay(1);
  // }
  //
  // if (Serial.available())
  //   Serial1.write(Serial.read());
}

void invalidate_clock() {
  uint8_t r = lux;
  r = max(25, r);
  r = min(160, r);
  r = r - 20; //[0,140]

  randomSeed(millis());
  //color = random(0x100000, 0xFFFFFF);
  //Serial.println(color, HEX);
  color = 0x0F003F0F;

  // uint8_t a = (uint8_t)((63 / 140.0) * (r));
  // uint32_t mask = ((uint32_t)(a)) << 24;
  // color = mask | color;

  clear_display();

  char s[6];
  sprintf(s, "%02d", hour);
  draw_str(s, 0, color);
  sprintf(s, "%02d", minute);
  draw_str(s, 12, color);
}

void invaildate_dht11() {
  char s[10];
  clear_display();
  if (_dht11_ok) {
    sprintf(s, "%02d", _dht11.temperature);
    draw_str(s, 0, color);
    sprintf(s, "%02d", _dht11.humidity);
    draw_str(s, 12, color);
  } else {
    draw_str("00", 0, color);
    draw_str("00", 12, color);
  }
}

void frame_tick_task(void *param) {
  for (;;) {
    frame_tick();
  }
}

void read_rtc_clock_task(void *param) {
  for (;;) {
    Serial.println("Read RTC");
    ReadDS3231();
    invalidate_clock();
    vTaskDelay(((60 - second) * 1000) / portTICK_PERIOD_MS);
    // print_frame_buffer();
    // vTaskDelay(1000/ portTICK_PERIOD_MS );
  }
}

void flash_dot_tick_task(void *param) {
  for (;;) {
    display_rotate++;
    if (display_rotate > 10) {
      printf("display rotate..\n");
      display_rotate = 0;
      if (display_type == DISPLAY_TIME) {
        display_type = DISPLAY_DHT11;
        invaildate_dht11();
      } else {
        display_type = DISPLAY_TIME;
        invalidate_clock();
      }
    }

    if (display_type == DISPLAY_TIME) {
      set_pixel(10, 2, second % 2 ? color : 0);
      set_pixel(10, 4, second % 2 ? color : 0);
      for (uint8_t i = 0; i < 32; i++) {
        set_pixel(i, 7, i < second / 2 + 1 ? color : 0);
      }
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void read_lux_task(void *param) {
  for (;;) {
    lux = analogRead(A0);
    // printf("lux: %d\n", lux);
    vTaskDelay(1300 / portTICK_PERIOD_MS);
  }
}

void read_dht11_task(void *param) {
  for (;;) {
  //  printf("read dht11...\n");
    int ret = _dht11.read(DHT11);
    if (ret == DHTLIB_OK) {
      _dht11_ok = true;
      //printf("read dht11 OK: %02doC, %02d%%\n", _dht11.temperature,
      //       _dht11.humidity);
    } else {
      _dht11_ok = false;
    //  printf("read dht11 ERROR:%d\n", ret);
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void flush_wifi_task(void *param) {
  for (;;) {
    if (Serial1.available()) {
      Serial.write(Serial1.read());
      digitalWrite(31, HIGH); // turn the LED on (HIGH is the voltage level)
      vTaskDelay(1);          // wait for a second
      digitalWrite(31, LOW);  // turn the LED off by making the voltage LOW
      vTaskDelay(1);
    }

    if (Serial.available()) {
      Serial1.write(Serial.read());
    }
    vTaskDelay(1);
  }
}

void read_am2320(void *param) {
  printf("init sem");
  vSemaphoreCreateBinary(am_sem_handle);
  printf("sem done");
  for (;;) {
    printf("reading am2320..\n");
    pinMode(AM2320_DATA_PIN, OUTPUT);
    digitalWrite(AM2320_DATA_PIN, LOW);
    vTaskDelay(1);
    pinMode(AM2320_DATA_PIN, INPUT);

    am_int = true;

    printf("reading am2320 done~\n");
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void init_freertos() {
  // Now set up two tasks to run independently.
  xTaskCreate(frame_tick_task,
              (const portCHAR *)"FrameTick" // A name just for humans
              ,
              128 // This stack size can be checked & adjusted by reading the
                  // Stack Highwater
              ,
              NULL, 1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the
                      // highest, and 0 being the lowest.
              ,
              NULL);

  xTaskCreate(read_rtc_clock_task, (const portCHAR *)"ReadRTC",
              128 // Stack size
              ,
              NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the
                      // highest, and 0 being the lowest.
              ,
              NULL);

  xTaskCreate(flash_dot_tick_task, (const portCHAR *)"FlashDot",
              128 // Stack size
              ,
              NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the
                      // highest, and 0 being the lowest.
              ,
              NULL);

  xTaskCreate(read_dht11_task, (const portCHAR *)"ReadDHT11", 128 // Stack size
              ,
              NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the
                      // highest, and 0 being the lowest.
              ,
              NULL);

  // xTaskCreate(
  //   flush_wifi_task
  //   ,  (const portCHAR *) "FlushWIFI"
  //   ,  128  // Stack size
  //   ,  NULL
  //   ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest,
  //   and 0 being the lowest.
  //   ,  NULL );

  // xTaskCreate(
  //   read_am2320
  //   ,  (const portCHAR *) "ReadAM2320"
  //   ,  128  // Stack size
  //   ,  NULL
  //   ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest,
  //   and 0 being the lowest.
  //   ,  NULL );

  // xTaskCreate(
  //   read_lux_task,
  //   (const portCHAR *) "ReadLUX",
  //   128,
  //   NULL,
  //   1,
  //   NULL
  // );

  // Now the task scheduler, which takes over control of scheduling individual
  // tasks, is automatically started.
}
