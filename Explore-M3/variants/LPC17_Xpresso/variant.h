/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2016 Bob Cousins
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/**
 * @file
 * @author
 * @brief  LPC17_Xpresso board header
 *
 */
 
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _VARIANT_H_
#define _VARIANT_H_

#include "stdutils.h"
#include "gpio.h"

#define LED_BUILTIN 13

#define BOARD_MAX_GPIO_PINS  84

//!#define C_MaxAdcChannels_U8  16

// #define EXPLORE_M3_RESET_STRING "r$T^^3"
// #define USB_BOOT_SWITCH 41

extern const uint8_t PIN_MAP[BOARD_MAX_GPIO_PINS];


/************************************
             ADC pins
************************************/             
// Note: not all of these pins are usable as ADC input
static unsigned const int A0 = 54;
static unsigned const int A1 = 55;
static unsigned const int A2 = 56;
static unsigned const int A3 = 57;
static unsigned const int A4 = 58;
static unsigned const int A5 = 59;
static unsigned const int A6 = 60;
static unsigned const int A7 = 61;
static unsigned const int A8 = 62;
static unsigned const int A9 = 63;
static unsigned const int A10 = 64;
static unsigned const int A11 = 65;
static unsigned const int A12 = 66;
static unsigned const int A13 = 67;
static unsigned const int A14 = 68;
static unsigned const int A15 = 69;

#define ANALOG_ZERO  A0
#define ANALOG_MAX   A15
/**********End of ADC pins********/

/************************************
             PWM pins
************************************/             
#define   PWM_1      4
#define   PWM_2      6
#define   PWM_3      9
#define   PWM_4      10
#define   PWM_5      11

/**********End of PWM pins**********/

/************************************
             SPI Pins
************************************/        
//todo
#define   SPI0_MOSI  9 
#define   SPI0_MISO  8
#define   SPI0_SCK   7
#define   SPI0_SS    6 

#define   SPI1_MOSI  10 
#define   SPI1_MISO  11
#define   SPI1_SCK   12
#define   SPI1_SS    13

//By default  the SPI1 is mapped to Arduino SPI library
#define   MOSI       SPI1_MOSI  
#define   MISO       SPI1_MISO
#define   SCK        SPI1_SCK
#define   SS         SPI1_SS
/**********End of SPI pins***********/



#endif


#ifdef __cplusplus
}
#endif
