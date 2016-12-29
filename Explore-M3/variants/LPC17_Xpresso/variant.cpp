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
 * @brief  LPC_Xpresso board file.
 */

#include "Arduino.h"
#include "variant.h"
#include "gpio.h"
//#include "stdutils.h"

#include "SDCard.h"
#include "USBMSD.h"


/**
 * Pin mapping for LPC_Xpresso
 */	
const uint8_t PIN_MAP[BOARD_MAX_GPIO_PINS] = {
    
    P0_3 ,    /* 0 */
    P0_2 ,    /* 1 */
    P1_25 ,   /* 2 */
    P1_24 ,   /* 3 */
    P1_18,    /* 4 */
    P1_19,    /* 5 */
    P1_21,    /* 6 */
    P_NC,     /* 7 */
    P2_7,     /* 8 */
    P2_4,     /* 9 */
    P2_5 ,    /* 10 */
    P1_20 ,   /* 11 */
    P2_12 ,   /* 12 */
    P4_28 ,   /* 13 */
    P1_26 ,   /* 14 */
    P1_27 ,   /* 15 */
    P1_29,    /* 16 */
    P0_18,    /* 17 */
    P1_29,    /* 18 */
    P1_28,    /* 19 */
    P0_0,     /* 20 */
    P0_1 ,    /* 21 */
    P_NC ,    /* 22 */
    P0_15 ,   /* 23 */
    P0_4 ,    /* 24 */
    P_NC ,    /* 25 */
    P2_0 ,    /* 26 */
    P_NC ,    /* 27 */
    P0_5 ,    /* 28 */
    P_NC ,    /* 29 */
    P4_29,    /* 30 */
    P3_26,    /* 31 */
    P_NC,     /* 32 */
    P3_25,    /* 33 */
    P2_13,    /* 34 */
    P2_11,    /* 35 */
    P2_8,     /* 36 */
    P1_30,    /* 37 */
    P0_10 ,   /* 38 */
    P_NC ,    /* 39 */
    P_NC ,    /* 40 */
    P1_22,    /* 41 */
    P_NC,     /* 42 */
    P_NC,     /* 43 */
    P_NC,     /* 44 */
    P_NC,     /* 45 */
    P2_3,     /* 46 */
    P_NC,     /* 47 */
    P0_22,    /* 48 */
    P1_31,    /* 49 */
    P0_17,    /* 50 */
    P0_18,    /* 51 */
    P0_15,    /* 52 */
    P1_23,    /* 53 */
    P2_1,     /* 54 A0 */
    P0_11,    /* 55 A1 */
    P0_19,    /* 56 A2 */
    P0_27,    /* 57 A3 */
    P0_28,    /* 58 A4 */
    P2_6,     /* 59 A5 */
    P2_2,     /* 60 A6 */
    P0_20,    /* 61 A7 */
    P0_21,    /* 62 A8 */
    P0_26,    /* 63 A9 */
    P_NC,     /* 64 A10 */
    P_NC,     /* 65 A11 */
    P_NC,     /* 66 A12 */
    P0_23,    /* 67 A13 */
    P0_24,    /* 68 A14 */
    P0_25,    /* 69 A15 */
    P1_0,     /* 70 J12 */  /* On J12 Ethernet/GPIO */
    P1_1,     /* 71 J12 */
    P1_4,     /* 72 J12 */
    P1_8,     /* 73 J12 */
    P1_9,     /* 74 J12 */
    P1_10,    /* 75 J12 */
    P1_14,    /* 76 J12 */
    P1_15,    /* 77 J12 */
    P1_16,    /* 78 J12 */
    P1_17,    /* 79 J12 */
    P0_6,     /* 80 SSEL1 */  /* for onboard SPI, not available on header */
    P0_7,     /* 81 SCK1  */
    P0_8,     /* 82 MISO1 */
    P0_9      /* 83 MOSI1 */

};

SDCard sdcard_interface  (1, P0_6);      // this selects SPI1 as the sdcard as it is on Smoothieboard

USB usb_interface;
USBSerial Serial (&usb_interface);
USBMSD usb_msc (&usb_interface, &sdcard_interface);



void variant_init (void) {

  //!debug
  Serial0.begin(115200);

  UART0_Printf ("START\n");

  sdcard_interface.disk_initialize(); // not needed?

  usb_interface.init();
  usb_interface.connect();

  usb_msc.connect();

  Serial.begin(9600); // ?

  char * buf = "hello\n";
//  Serial.puts(buf);


}




 

