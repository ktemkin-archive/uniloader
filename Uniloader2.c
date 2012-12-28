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
 *  Main source file for the GenericHID demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "Uniloader2.h"

/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
uint8_t PrevHIDReportBuffer[GENERIC_REPORT_SIZE];

/** Structure to contain reports from the host, so that they can be echoed back upon request */
struct
{
	uint8_t  ReportID;
	uint16_t ReportSize;
	uint8_t  ReportData[GENERIC_REPORT_SIZE];
} HIDReportEcho;

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Generic_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = 0,

				.ReportINEndpointNumber       = GENERIC_IN_EPNUM,
				.ReportINEndpointSize         = GENERIC_EPSIZE,
				.ReportINEndpointDoubleBank   = false,

				.PrevReportINBuffer           = PrevHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevHIDReportBuffer),
			},
	};

/**
 * True iff a connection to the host PC has been made.
 */
bool connectionMade = false;

/**
 * Status LED blink speed.
 * A higher speed indicates a slower blink.
 */
unsigned int blinkOn = 1000;
unsigned int blinkOff = 20000;

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();
	sei();

	for (;;)
	{
                blink_led();
		HID_Device_USBTask(&Generic_HID_Interface);
		USB_USBTask();
	}
}

/**
 * Small daemon 'thread' which blinks the LED to indicate status.
 */
void blink_led()
{
    #    ifdef ONBOARD_LED_PORT

        //Blink the LED quickly to indicate the device is waiting for instruction.
        if ((TCNT1 % (blinkOn + blinkOff) < blinkOn))
            //LED on
            ONBOARD_LED_PORT |= (1 << ONBOARD_LED_PIN);
        else
            //LED off
            ONBOARD_LED_PORT &= ~(1 << ONBOARD_LED_PIN);

    #    endif
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
    //Disable the system watchdog, as we'll be busy waiting.
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    //Set the clock to a full 16MHz.
    clock_prescale_set(clock_div_1);

    //Set up the status timer, which is used to blink the LED, and set the LED to output mode.
    TCCR1B |= ((1 << CS12) | (0 << CS11) | (1 << CS10)); //occurs @ 15,625 hz
    ONBOARD_LED_DDR |= 1 << ONBOARD_LED_PIN;

    //Set up our 2(?)MHz clock, which drives the FPGA when the microcontroller
    //is not programmed:

    //set the clock prescalar to 1x (i.e. system clock)
    TCCR4B |= (1 << CS40);
    TCCR4B &= ~(1 << CS41 | 1 << CS42 | 1 << CS43 | 1 << DTPS40 | 1 << DTPS41); //FIXME DTPS41, DTPS40

    //set pin D7 to output
    DDRD |= 1 << PD7;
    PORTD |= 1 << PD7;


        //set pin D7 / OC4D to toggle each time the counter is cleared
        OCR4D = 0;
        TCCR4C |= 1 << COM4D0;
        TCCR4C &= ~(1 << COM4D1);

        //Each time the clock is >= 0 (i.e. every time), clear the timer
        OCR4C = 0; //2;


    /* Relocate the interrupt vector table to the bootloader section */
    MCUCR = (1 << IVCE);
    MCUCR = (1 << IVSEL);

    //init the JTAG chain
    jtag_initialize();

    /* Initialize USB subsystem */
    USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Generic_HID_Interface);

	USB_Device_EnableSOFEvents();

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Generic_HID_Interface);

        //check for req?
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Generic_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent
 *
 *  \return Boolean true to force the sending of the report, false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	if (HIDReportEcho.ReportID)
	  *ReportID = HIDReportEcho.ReportID;

	memcpy(ReportData, HIDReportEcho.ReportData, HIDReportEcho.ReportSize);

	*ReportSize = HIDReportEcho.ReportSize;
	return true;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the created report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	HIDReportEcho.ReportID   = ReportID;
	HIDReportEcho.ReportSize = ReportSize;
	memcpy(HIDReportEcho.ReportData, ReportData, ReportSize);
}


/** Event handler for the USB_UnhandledControlRequest event. This is used to catch standard and class specific
 *  control requests that are not handled internally by the USB library (including the HID commands, which are
 *  all issued via the control endpoint), so that they can be handled appropriately for the application.
 */
void EVENT_USB_Device_UnhandledControlRequest(void)
{
    //once we're attached via USB, stop blinking
    blinkOn = 1000;
    blinkOff = 1;

    /* Handle HID Class specific requests */
    if (USB_ControlRequest.bRequest == REQ_SetReport)
    {
        //Once communications have started, speed up our LED blink.
        connectionMade = true;

        Endpoint_ClearSETUP();

        /* Wait until the command has been sent by the host */
        while (!(Endpoint_IsOUTReceived()));

        //speed up after the first packet has been received
        //TODO: unspeed (i.e. slow) after idle
        blinkOn = blinkOff = 1600;

        /* Read in the write destination address */
        uint16_t PageAddress = Endpoint_Read_Word_LE();

        //If we've received the RESTART_BOOTLOADER command, restart.
        switch (PageAddress)
        {
            //Hard reset.
            case CMD_RESTART:
                //RunBootloader = false;
                hard_reset();
                break;

            //Soft reset
            case CMD_SOFT_RESET:
                USB_Detach();
                asm volatile("jmp 0000");
                break;

            case CMD_FPGA_OFF:
                //FIXME
                break;

                //Begin FPGA Configuration:
                //
                //Send the correct JTAG sequence to begin configuration,
                //then sends the 128 byte argument over the configuration line.
            case CMD_FPGA_CONFIG_START:

                //ensure the FPGA is powered on
                fpga_set_power(1);

                //reset the FPGA
                fpga_reset();

                //start FPGA configuration
                fpga_init_config(true);

                //and roll into the data send operation


                //Continue FPGA Configuration
                //
                //Sends the 128-bytes argument over the FPGA configuration line.
            case CMD_FPGA_CONFIG_SEND:

                //for(uint8_t byteNo = 0; byteNo < BYTES_PER_PACKET; ++byteNo)
                for (uint8_t byteNo = 0; byteNo < 128; ++byteNo) //DEBUG
                {
                    /* Check if endpoint is empty - if so clear it and wait until ready for next packet */
                    if (!(Endpoint_BytesInEndpoint()))
                    {
                        Endpoint_ClearOUT();
                        while (!(Endpoint_IsOUTReceived()));
                    }


                    //determine if this is the first byte
                    bool firstByte = (byteNo == 0) && (PageAddress == CMD_FPGA_CONFIG_START);

                    //and send the configuration data received
                    fpga_send_config((char) Endpoint_Read_Byte(), firstByte, false);

                }
                break;

                //Finishes FPGA communication.
                //
                //The argument to this command is slightly different from the others-
                //the first byte indicates the amount of argument data that should follow.
                //
                //This enables the use of variable-sized bit-streams (such as compressed bit-streams.)
            case CMD_FPGA_CONFIG_END:

            {
                //determine the data length to be read
                uint8_t dataLength = Endpoint_Read_Byte();


                //and send the configuration data
                for (uint8_t byteNo = 0; byteNo < 128; ++byteNo)
                {
                    /* Check if endpoint is empty - if so clear it and wait until ready for next packet */
                    if (!(Endpoint_BytesInEndpoint()))
                    {
                        Endpoint_ClearOUT();
                        while (!(Endpoint_IsOUTReceived()));
                    }

                    if (byteNo < dataLength)
                        fpga_send_config((char) Endpoint_Read_Byte(), false, byteNo == dataLength - 1);
                }

                //clear status (runtest takes a while?)

                Endpoint_ClearOUT();
                Endpoint_ClearStatusStage();

                //finalize the configuration and start the FPGA
                fpga_finish_config();

                //stop blinking once the programming is complete
                blinkOn = 1000;
                blinkOff = 1;

                //TODO: prevent intermittant comm. error

                break;
            }


                //SD card config items here


            default:

                //If the address to be written is beyond the end of user memory,
                //ignore the instruction
                if (PageAddress >= BOOTLOADER_START)
                    break;


                /* Erase the given FLASH page, ready to be programmed */
                boot_page_erase(PageAddress);
                boot_spm_busy_wait();

                /* Write each of the FLASH page's bytes in sequence */
                for (uint8_t PageByte = 0; PageByte < SPM_PAGESIZE; PageByte += 2)
                {
                    /* Check if endpoint is empty - if so clear it and wait until ready for next packet */
                    if (!(Endpoint_BytesInEndpoint()))
                    {
                        Endpoint_ClearOUT();
                        while (!(Endpoint_IsOUTReceived()));
                    }

                    /* Write the next data word to the FLASH page */
                    boot_page_fill(PageAddress + PageByte, Endpoint_Read_Word_LE());
                }

                /* Write the filled FLASH page to memory */
                boot_page_write(PageAddress);
                boot_spm_busy_wait();

                /* Re-enable RWW section */
                boot_rww_enable();
                break;
        }

        //discard the output data and status
        Endpoint_ClearOUT();
        Endpoint_ClearStatusStage();

    }
}

