/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Header file for GenericHID.c.
 */

#ifndef _UNILOADER_2_H_
#define _UNILOADER_2_H_

	/* Includes: */

                #include <string.h>
                #include <stdbool.h>
                #include <util/delay.h>

		#include <avr/io.h>
		#include <avr/wdt.h>
                #include <avr/boot.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>

                #include "unilab.h"
                #include "jtag/fpga.h"

		#include "Descriptors.h"

		#include <LUFA/Version.h>
		#include <LUFA/Drivers/Board/LEDs.h>
		#include <LUFA/Drivers/USB/USB.h>

	/* Macros: */
		/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
		#define LEDMASK_USB_NOTREADY      LEDS_LED1

		/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
		#define LEDMASK_USB_ENUMERATING  (LEDS_LED2 | LEDS_LED3)

		/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
		#define LEDMASK_USB_READY        (LEDS_LED2 | LEDS_LED4)

		/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
		#define LEDMASK_USB_ERROR        (LEDS_LED1 | LEDS_LED3)


	/** HID Class specific request to send the next HID report to the device. */
	#define REQ_SetReport             0x09




	/**
	 * Communications constants.
	 */
	#define WORDS_PER_PACKET	64
	#define BYTES_PER_PACKET	WORDS_PER_PACKET * 2


	/**
	 * Bootloader commands.
	 */

	//Hard reset the microprocessor.
	//This value is kept at FFFF so the Unilab kit is binary compatible with the Teensy loader.
	#define CMD_RESTART   0xFFFF

	//Hard reset the uProc, if possible.
	#define CMD_HARD_RESET 0xF000

	//'Soft reset' the microprocessor.
	#define CMD_SOFT_RESET 0xF001

	//Request the device sends identification.
	#define CMD_WHOAMI	0xF002

        //Request a change in the bootloader clock (the clock sent to the FPGA).
        #define CMD_SET_CLOCK_OUT

	//Basys2 board commands
        #if defined(UNILAB_BASYS_100K) || defined(UNILAB_BASYS_250K) || defined(UNILAB_MARK1)

			#define CMD_FPGA_OFF 0xF020
			#define CMD_FPGA_ON  0xF021

			#define CMD_FPGA_CONFIG_START 0xF022
			#define CMD_FPGA_CONFIG_SEND  0xF023
			#define CMD_FPGA_CONFIG_END   0xF024

	#endif



	/* Function Prototypes: */
		void SetupHardware(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);
		void EVENT_USB_Device_StartOfFrame(void);

		bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                         uint8_t* const ReportID,
		                                         const uint8_t ReportType,
		                                         void* ReportData,
		                                         uint16_t* const ReportSize);
		void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                          const uint8_t ReportID,
		                                          const uint8_t ReportType,
		                                          const void* ReportData,
		                                          const uint16_t ReportSize);

                void EVENT_USB_Device_UnhandledControlRequest(void);

                void hard_reset(void);
                void blink_led(void);

#endif

