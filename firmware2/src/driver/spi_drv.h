#include "drv_base.h"

#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

#define API

// spi驱动， 使用事件驱动
/////////////////////////////////////////////////////////////////////////////////
// public functions
/////////////////////////////////////////////////////////////////////////////////
API uint8_t spi_init(uint8_t clock_div, uint8_t spi_mode, uint8_t bit_order);
API uint8_t spi_send(byte_t* buff, uint8_t len, callback_fn_t cb_fn, void* cb_param);
API uint8_t spi_recv(byte_t* buff, uint8_t len, callback_fn_t cb_fn, void* cb_param);


////////////////////////////////////////////////////////////////////////////////
//private functions
////////////////////////////////////////////////////////////////////////////////
void _spi_begin();
void _spi_end();
void _spi_send_next();
void _spi_recv_next();
