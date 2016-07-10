/* freeRTOSVariant.h
 *
 * Board variant (hardware) specific definitions for the AVR boards that I use regularly.
 *
 * This file is NOT part of the FreeRTOS distribution.
 *
 */

#ifndef freeRTOSVariant_h
#define freeRTOSVariant_h

#include <avr/io.h>
#include <avr/wdt.h>

#include "Arduino_FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

// System Tick - Scheduler timer
// Use the Watchdog timer, and choose the rate at which scheduler interrupts will occur.

//#define portUSE_WDTO			WDTO_30MS	// portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick
#define portUSE_TICK_PERIOD_MS      1 // 1ms tick
#define portUSE_TICK_CNT			portUSE_TICK_PERIOD_MS * 1000 / 16	// tick cnt of timer, one cnt is 16us

/* Watchdog period options: 	WDTO_15MS
								WDTO_30MS
								WDTO_60MS
								WDTO_120MS
								WDTO_250MS
								WDTO_500MS
*/
//	xxx Watchdog Timer is 128kHz nominal, but 120 kHz at 5V DC and 25 degrees is actually more accurate, from data sheet.
//#define configTICK_RATE_HZ		( (TickType_t)( (uint32_t)128000 >> (portUSE_WDTO + 11) ) )  // 2^11 = 2048 WDT scaler for 128kHz Timer
#define configTICK_RATE_HZ portUSE_TICK_PERIOD_MS*1000

/*-----------------------------------------------------------*/

void initVariant(void) __attribute__((flatten));

void vApplicationIdleHook( void ) __attribute__((flatten));

void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t xTask, portCHAR *pcTaskName );

/*-----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif // freeRTOSVariant_h
