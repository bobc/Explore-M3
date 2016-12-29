#ifndef _DEBUG_H
#define _DEBUG_H

#include "uart.h"

#define ENTER_ISR() do {} while (0)
#define LEAVE_ISR() do {} while (0)

#define printf(...) iprintf(__VA_ARGS__)
// #define iprintf(...) THEKERNEL->streams->printf(__VA_ARGS__)

/*
 * Use
 * - xprintf for important state changes, events
 * - eprintf for errors
 * - iprintf for verbose/detailed info
 *
 * DEBUG not defined = no debug
 * DEBUG = 1 enable x and e
 * DEBUG = 2 enable all
 */
#if DEBUG == 1
  // important debug only
  #define xprintf UART0_Printf
  #define eprintf UART0_Printf
  #define iprintf(...) do { } while (0)
#elif DEBUG == 2
  // verbose debug
  #define xprintf UART0_Printf
  #define eprintf UART0_Printf
  #define iprintf UART0_Printf
#else
  // no debug for you
  #define xprintf(...) do { } while (0)
  #define eprintf(...) do { } while (0)
  #define iprintf(...) do { } while (0)
#endif

#define setled(a, b) do {} while (0)
// extern void setled(int, bool);

#endif /* _DEBUG_H */
