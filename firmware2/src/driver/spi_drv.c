#include "spi_drv.h"
#include <avr/interrupt.h>
//#include <SPI.h>

//the global spi device data
io_device_t dev_spi;

uint8_t spi_init(uint8_t clock_div, uint8_t spi_mode, uint8_t bit_order){
  SPCR = _BV(SPIE)  // SPI intteruppt enable
  | (spi_mode == LSBFIRST ? _BV(DORD) : 0)  //LSB or MSB
  | _BV(MSTR) // only support SPI as master
  | spi_mode  // CPOL & CPHA
  | bit_order // todo .. SPI2X
  ;

  return IO_OK;
}


uint8_t spi_send(byte_t* buff, uint8_t len, callback_t callback, void* cb_param){
  if(dev_spi.state == BUSY){
    return IO_DRIVER_BUSY;
  }

  dev_spi.io_req.op = WRITE;
  dev_spi.io_req.buff = buff;
  dev_spi.io_req.len = len;
  dev_spi.io_req.pos = 0;
  dev_spi.io_req.callback = callback;
  dev_spi.io_req.cb_param = cb_param;
  dev_spi.state = BUSY;


  //init first byte send
  _spi_begin();
  _spi_send_next();

  return IO_OK;
}
uint8_t spi_recv(byte_t* buff, uint8_t len, callback_t callback, void* cb_param){
  if(dev_spi.state == BUSY){
    return IO_DRIVER_BUSY;
  }

  dev_spi.io_req.op = READ;
  dev_spi.io_req.buff = buff;
  dev_spi.io_req.len = len;
  dev_spi.io_req.pos = 0;
  dev_spi.io_req.callback = callback;
  dev_spi.io_req.cb_param = cb_param;
  dev_spi.state = BUSY;


  // init recv ..
  _spi_begin();
  _spi_recv_next();

  return IO_OK;
}


////////////////////////////////////////////////////////////////////////////
void _spi_begin(){
  // Set SS to high so a connected chip will be "deselected" by default
     uint8_t port = digitalPinToPort(SS);
     uint8_t bit = digitalPinToBitMask(SS);
     volatile uint8_t *reg = portModeRegister(port);

     // if the SS pin is not already configured as an output
     // then set it high (to enable the internal pull-up resistor)
     if(!(*reg & bit)){
       digitalWrite(SS, HIGH);
     }

     // When the SS pin is set as OUTPUT, it can be used as
     // a general purpose output port (it doesn't influence
     // SPI operations).
     pinMode(SS, OUTPUT);

     // Warning: if the SS pin ever becomes a LOW INPUT then SPI
     // automatically switches to Slave, so the data direction of
     // the SS pin MUST be kept as OUTPUT.
     SPCR |= _BV(MSTR);
     SPCR |= _BV(SPE);

     // Set direction register for SCK and MOSI pin.
     // MISO pin automatically overrides to INPUT.
     // By doing this AFTER enabling SPI, we avoid accidentally
     // clocking in a single bit since the lines go directly
     // from "input" to SPI control.
     // http://code.google.com/p/arduino/issues/detail?id=888
     pinMode(SCK, OUTPUT);
     pinMode(MOSI, OUTPUT);
}

void _spi_end(){
  SPCR &= ~_BV(SPE);
}

void _spi_send_next(){
    byte_t b = dev_spi.io_req.buff[dev_spi.io_req.pos++];
    SPDR = b;
}

void _spi_recv_next(){
  //write SPDR will inital transfer
  SPDR = 0;
}

///////////////////////////////////////////////////////////////////////////
//SPI interuppt handler
ISR(SPI_STC_vect){
  //device is idle, simple ignore..
  if(dev_spi.state == IDLE){
    return;
  }

  if(dev_spi.io_req.op == READ){
      if(dev_spi.io_req.pos < dev_spi.io_req.len){
        dev_spi.io_req.buff[dev_spi.io_req.pos++] = SPDR;
        _spi_recv_next();
      }else if(dev_spi.io_req.pos == dev_spi.io_req.len){
        dev_spi.state = IDLE;
        _spi_end();
        dev_spi.io_req.callback(dev_spi.io_req.cb_param);
      }else{
        //
      }
  }else{
        //send buffer is not empty, send next
        if(dev_spi.io_req.pos < dev_spi.io_req.len){
          _spi_send_next();
        }else if(dev_spi.io_req.pos == dev_spi.io_req.len){
          //TODO .. should call in main loop, not suitable here..
          dev_spi.state = IDLE;
          _spi_end();
          dev_spi.io_req.callback(dev_spi.io_req.cb_param);
        }else{
          //
        }
  }
}
