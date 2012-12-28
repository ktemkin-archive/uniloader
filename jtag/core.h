#pragma once
//FIXME

#include <avr/io.h>
#include "../unilab.h"

#if \
    !defined(JTAG_TMS_PORT) || \
    !defined(JTAG_TMS_DDR)  || \
    !defined(JTAG_TMS_PIN)  || \
    \
    !defined(JTAG_TMS_PORT) || \
    !defined(JTAG_TCK_DDR)  || \
    !defined(JTAG_TCK_PIN)  || \
    \
    !defined(JTAG_TDI_PORT) || \
    !defined(JTAG_TDI_DDR)  || \
    !defined(JTAG_TDI_PIN)  || \
    \
    !defined(JTAG_TDO_PORT) || \
    !defined(JTAG_TDO_DDR)  || \
    !defined(JTAG_TDO_PIN)

#error Define the four JTAG pins before including jtag/core.h.
#error See core.h or the documentation for more information.

#endif


#if !defined(JTAG_TCK_HIGH) || !defined(JTAG_TCK_LOW)

#warning JTAG clock periods were not defined before inclusion of jtag/core.h; 
#warning the delays will be as small as possible. This may not be appropriate
#warning for all applications.

    //Define NO_DELAY to prevent the program from inducing delays.
    //This runs the clock as fast as possible; on sufficiently slow
    //microprocessors, it should not impact execution.
    #define JTAG_NO_DELAY

    //TCK times: the minimum amount of microseconds to spend high and low,
    //respectively, per one TCK pulse; TCK idles high
    #define JTAG_TCK_HIGH 1
    #define JTAG_TCK_LOW 1

    //The delay between bit-shifts, in microseconds.
    //Comment out this line for no delay.
    //#define JTAG_BIT_DELAY 5

    //If SLOW_CLOCK is defined, the above timings will be multiplied by 1000.
    //i.e. the times represented will be in ms
    //#define JTAG_SLOW_CLOCK

#endif



//TAP state 'enumeration'
typedef char tap_state;
#define TAP_STATE_RESET  	0x00
#define TAP_STATE_RUNTEST	0x01    /* a.k.a. IDLE */
#define TAP_STATE_IDLE		0x01
#define TAP_STATE_SELECTDR 	0x02
#define TAP_STATE_CAPTUREDR 	0x03
#define TAP_STATE_SHIFTDR   	0x04
#define TAP_STATE_EXIT1DR   	0x05
#define TAP_STATE_PAUSEDR   	0x06
#define TAP_STATE_EXIT2DR   	0x07
#define TAP_STATE_UPDATEDR  	0x08

#define TAP_CATEGORY_IR	  	0x09    /* Virtual state category for IR state */

#define TAP_STATE_SELECTIR  	0x09
#define TAP_STATE_CAPTUREIR 	0x0A
#define TAP_STATE_SHIFTIR   	0x0B
#define TAP_STATE_EXIT1IR   	0x0C
#define TAP_STATE_PAUSEIR   	0x0D
#define TAP_STATE_EXIT2IR   	0x0E
#define TAP_STATE_UPDATEIR  	0x0F

//JTAG functions
void tms_set(char value);
void tms_reset(void);
void jtag_shift_instruction(char c, char bits, char first, char last);
char jtag_shift_data(char c, char bits, char first, char last);
void jtag_initialize(void);
void tap_set_state(char);
void run_test(long clocks);

