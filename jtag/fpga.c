
//JTAG
#include "core.h"
#include "fpga.h"

#include <stdbool.h>

/*
 * fpga_reset
 *
 * Reset the FPGA; can occur anytime.
 */
void fpga_reset()
{
	//Reset the device via JTAG.
	tms_reset();
}


/**
 * fpga_set_power
 *
 * Enable/disable power to the FPGA.
 *
 * on: if 0, cuts power to the FPGA; otherwise enables power
 */
void fpga_set_power(char on)
{
	FPGA_POWER_DDR |= 1 << FPGA_POWER_PIN;

	if(on)
		FPGA_POWER_PORT |= 1 << FPGA_POWER_PIN;
	else
		FPGA_POWER_PORT &= (1 << FPGA_POWER_PIN);

}

/**
 * fpga_get_idcode()
 *
 * Returns: The hex-valued IDcode for the FPGA.
 */
long fpga_get_idcode()
{
	unsigned long data = 0;
	char buffer;

	//reset the FPGA
	fpga_reset();

	//Shift the IDCode instruction (6 bits)
	jtag_shift_instruction(FPGA_IDCODE_INST, FPGA_IDCODE_BITS, true, true);

	//then retrieve the ID core
	for(int i=0; i<4; ++i)
	{
		//shift in the response
		buffer = jtag_shift_data(0x00, 8, i==0, i==3);

		//append the buffer to the data
		data |= (long)buffer << (i * 8);
	}

	//and return the IDCode
	return data;
}

/**
 * fpga_init_config
 *
 * Reset the FPGA and initializes configuration.
 *
 * jtag_config:		If false, configuration progresses as set by the FPGA's mode flags.
 * 					On the basys2 board, this initializes PROM configuration.
 *
 */
void fpga_init_config(bool jtag_config)
{
	//reset the FPGA
	fpga_reset();

	//initialize configuration- this simulates pulsing the PROG_B pin
	jtag_shift_instruction(FPGA_JPROGRAM_INST, FPGA_JPROGRAM_BITS, true, true);

	//if the user wants to configure the device via JTAG, send the
	//appropriate instruction
	if(jtag_config)
	{
		//send the CFG_IN instruction
		jtag_shift_instruction(FPGA_CFG_IN_INST, FPGA_CFG_IN_BITS, true, true);

		//wait 14,000 clock cycles
		run_test(14000);

		//send the CFG_IN instruction
		jtag_shift_instruction(FPGA_CFG_IN_INST, FPGA_CFG_IN_BITS, true, true);

		//send 95 zeroes (flush register?)

		//88 zeroes
		for(int i=0; i < 11; ++i)
			jtag_shift_data(0x00, 8, i==0, false);

		//and 7 zeroes
		jtag_shift_data(0x00, 7, false, true);

		//send the CFG_IN instruction
		jtag_shift_instruction(FPGA_CFG_IN_INST, FPGA_CFG_IN_BITS, true, true);
	}
}

/**
 * fpga_send_config_byte
 *
 * Sends a single byte of the configuration bitstream.
 * Should be preceded by fpga_init_config.
 */
void fpga_send_config(char c, bool first, bool last)
{
	jtag_shift_data(c, 8, first, last);
}

/**
 * fpga_finish_config
 *
 * Sends the instructions needed to complete configuration.
 * If successful, the FPGA should now be functional.
 */
void fpga_finish_config()
{
	//send the JSTART command
	jtag_shift_instruction(FPGA_JSTART_INST, FPGA_JSTART_BITS, true, true);

	//send 16 zeroes
	for(int i = 0; i < 4; ++i)
		jtag_shift_data(0x00, 8, i==0, i==3);

	//finish configuration in the idle state
	tap_set_state(TAP_STATE_IDLE);

	//startup time
	run_test(100);
}


