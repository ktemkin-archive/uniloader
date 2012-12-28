/* 
 * File:   Unilab.h
 * Author: ktemkin
 *
 * Created on March 8, 2011, 5:45 AM
 */

#ifndef _UNILAB_H_
#define	_UNILAB_H_

#ifdef	__cplusplus
extern "C"
{
#endif

    //Target device:

    //Uncomment one of the following, or define in the makefile

    //Unilab backup device: a modified Basys2 board, 250K gate
    //#define UNILAB_BASYS_250K

    //Unilab backup device: a modified Basys2 board, 100K gate
    //#define UNILAB_BASYS_100K

    //Unified Lab Kit, early breadboard version
    //#define UNILAB_BREADBOARD

    //Unified Lab Kit, assembled PCB V1
    //#define UNILAB_MARK1


    /**
     * Revision codes for the different USB devices; these codes identify the
     * device sub-version to the Unified Programmer.
     */

    //basys boards
    #define DEVICE_BASYS_100K 0x01
    #define DEVICE_BASYS_250K 0x02

    //Prototype Unified Lab Kit
    #define DEVICE_UNILAB_BREADBOARD 0x03
    #define DEVICE_UNILAB_MARK1      0x04

    /**
     * Flashable region definitions
     */
    #if defined(__AVR_AT90USB162__)
        #define BOOTLOADER_START	0x3000
    #elif defined(__AVR_ATmega32U4__)
        #define BOOTLOADER_START        0x7000
    #endif


    #if defined(UNILAB_BREADBOARD)

        //Manual Hardware Bootloader Select
        //This condition, upon device reset, starts the bootloader.
        //(Comment this line out to always run the bootloader).
        //#define HWB_CONDITION !(PINB & (1 << 7))

        //If defined, the HWB condition must persist until a USB
        //connection is made.
        //#define CONDITION_MUST_PERSIST

        //TODO: define per board model
        #define ONBOARD_LED_PORT PORTD
        #define ONBOARD_LED_DDR  DDRD
        #define ONBOARD_LED_PIN  2

    #elif defined(UNILAB_BASYS_100K) | defined(UNILAB_BASYS_250K)

        //Manual Hardware Bootloader Select
        //Run the bootloader when the POWER switch is _off_.
        #define HWB_CONDITION !(PINB & (1 << 7)) 

        //If defined, the HWB condition must persist until a USB
        //connection is made.
        #define CONDITION_MUST_PERSIST

        //basys board status LED
        #define ONBOARD_LED_ON_STATE    1 //active high (inverted XOR mask)
        #define ONBOARD_LED_PORT        PORTB
        #define ONBOARD_LED_DDR         DDRB
        #define ONBOARD_LED_PIN         4

        //Test Mode Select (TMS)
        #define JTAG_TMS_PORT   PORTB
        #define JTAG_TMS_DDR    DDRB
        #define JTAG_TMS_PIN    0

        //Test clock (TCK)
        #define JTAG_TCK_PORT   PORTB
        #define JTAG_TCK_DDR    DDRB
        #define JTAG_TCK_PIN    1

        //Test Data In (TDI)
        #define JTAG_TDI_PORT   PORTB
        #define JTAG_TDI_DDR    DDRB
        #define JTAG_TDI_PIN    2

        //Test Data Out (TDO)
        #define JTAG_TDO_PORT   PINB
        #define JTAG_TDO_DDR    DDRB
        #define JTAG_TDO_PIN    3

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

    #elif defined(UNILAB_MARK1)

        //Manual Hardware Bootloader Select
        //Run the bootloader when the POWER switch is _off_.
        //#define HWB_CONDITION !(PINB & (1 << 7))

        //If defined, the HWB condition must persist until a USB
        //connection is made.
        //#define CONDITION_MUST_PERSIST

        //basys board status LED
        #define ONBOARD_LED_ON_STATE    1 //active high (inverted XOR mask)
        #define ONBOARD_LED_PORT        PORTE
        #define ONBOARD_LED_DDR         DDRE
        #define ONBOARD_LED_PIN         6

        //JTAG pins:

        //Test Mode Select (TMS)
        #define JTAG_TMS_PORT   PORTC
        #define JTAG_TMS_DDR    DDRC
        #define JTAG_TMS_PIN    6

        //Test clock (TCK)
        #define JTAG_TCK_PORT   PORTC
        #define JTAG_TCK_DDR    DDRC
        #define JTAG_TCK_PIN    7

        //Test Data In (TDI)
        #define JTAG_TDI_PORT   PORTF
        #define JTAG_TDI_DDR    DDRF
        #define JTAG_TDI_PIN    0

        //Test Data Out (TDO)
        #define JTAG_TDO_PORT   PINF
        #define JTAG_TDO_DDR    DDRF
        #define JTAG_TDO_PIN    1

        //JTAG Timing

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


#ifdef	__cplusplus
}
#endif

#endif	/* UNILAB_H */

