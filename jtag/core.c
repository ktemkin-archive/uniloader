/**
 * JTAG FPGA/PROM Configuration Library
 *
 * ~ktemkin
 *
 */

#include "core.h"

#if !defined(NO_DELAY) || defined(BIT_DELAY)
	#include <util/delay.h>
#endif

//#define DEBUG_JTAG

#ifdef DEBUG_JTAG
	#include <stdio.h>
#endif

//local 'private' (static) functions
static char jtag_shift_char(char c, char bits, char advance);

//Stores the current TAP state.
char jtag_tap_state = 0;

//Currently specified RUNTEST delay, in number of TCK cycles.
char run_test_clocks = 0;

void jtag_initialize()
{
        //set the polarity of the JTAG pins
        JTAG_TMS_DDR |= 1 << JTAG_TMS_PIN;
        JTAG_TCK_DDR |= 1 << JTAG_TCK_PIN;
        JTAG_TDI_DDR |= 1 << JTAG_TDI_PIN;
        JTAG_TDO_DDR &= ~(1 << JTAG_TDO_PIN);


}

/*
 * tck_pulse
 *
 * Sends a single rising edge over TCK.
 * For convenience, returns the value of TDO at just before the edge.
 *
 */
static char tck_pulse(void)
{
	char tdo;

	//set TCK low, and idle for TCK_LOW
        JTAG_TCK_PORT &= ~(1 << JTAG_TCK_PIN);

	#ifndef JTAG_NO_DELAY
		#ifndef JTAG_SLOW_CLOCK
			_delay_us(JTAG_TCK_LOW);
		#else
			_delay_ms(JTAG_TCK_LOW);
		#endif
	#endif

	//read TDO, for convenience
	tdo = 0x01 & (JTAG_TDO_PORT >> JTAG_TDO_PIN);

	//set TCK high, and then idle for TCK_HIGH
	JTAG_TCK_PORT |= 1 << JTAG_TCK_PIN;

	#ifndef JTAG_NO_DELAY
		#ifndef JTAG_SLOW_CLOCK
			_delay_us(JTAG_TCK_LOW);
		#else
			_delay_ms(JTAG_TCK_LOW);
		#endif
	#endif

	//return TDO value just before the edge
	return tdo;
}

/*
 * TMS Advance convenience function.
 *
 * Sets TMS and pulses the clock. For convenience, returns the value of tck_pulse.
 * (TDO just before the clock edge.)
 */
static char tms_advance(char tms)
{
	//set TMS
	tms_set(tms);

	#ifdef JTAG_BIT_DELAY
		#ifdef JTAG_SLOW_CLOCK
			_delay_ms(JTAG_BIT_DELAY);
		#else
			_delay_us(JTAG_BIT_DELAY);
		#endif
	#endif

	//and pulse TCK
	return tck_pulse();
}

/**
 * tap_set_state
 *
 * This (rather ugly) method emulates the internal TAP FSM, and
 * supplies the correct stimuli to navigate to the current state.
 *
 * To save space, this algorithm ignores/allows illegal calls.
 *
 * Modified from some (very ugly) Xilinx code.
 *
 * new_state:	the TAP_STATE code the machine should be in
 */
void tap_set_state(char new_state)
{
#ifdef DEBUG_JTAG
	printf("Changing state to %x.\n", new_state);
#endif


	//Handle device resets independently of emulated FSM.
	//(This allows us to break out of bad conditions.)
	if(new_state == TAP_STATE_RESET)
	{
		tms_reset();
		return;
	}

	//if we're already in the target state
	if(jtag_tap_state==new_state)
	{
		//according to the SVF standard, rx'ing a second pause
		//should exit from the pause state (pause is 'toggle-able')

		if(jtag_tap_state == TAP_STATE_PAUSEDR)
		{
			//exit pause
			tms_advance(1);
			jtag_tap_state = TAP_STATE_EXIT2DR;
		}
		else if(jtag_tap_state == TAP_STATE_PAUSEIR)
		{
			//exit pause
			tms_advance(1);
			jtag_tap_state = TAP_STATE_EXIT2IR;
		}
		else
			//in all other states, do nothing
			return;
	}


	//Emulate the FSM, providing the FPGA with the stimulus needed to move
	//from state to state.
	//
	//Note that an invalid desired state transition with crash the uProc;
	//we're assuming this never happens.

	while(jtag_tap_state != new_state)
	{
#ifdef DEBUG_JTAG
		//printf("Iterated. State is: %x. TMS is %x.\n", jtag_tap_state, 1 & (PORTB >> TMS));
#endif

		//FSM
		switch(jtag_tap_state)
		{
			//Test-logic-reset.
			case TAP_STATE_RESET:

				//Send a 0, advancing to run-test-idle.
				tms_advance(0);
				jtag_tap_state = TAP_STATE_IDLE;
				break;

			//Run-test-idle
			case TAP_STATE_IDLE:

				//Send a 1, advancing to Select-DR-Scan
				tms_advance(1);
				jtag_tap_state = TAP_STATE_SELECTDR;

				break;

			//Select Data Scan
			case TAP_STATE_SELECTDR:

				//If the target is an IR state, advance towards the instruction states
				if(new_state >= TAP_CATEGORY_IR)
				{
					//advance to Select Instruction
					tms_advance(1);
					jtag_tap_state = TAP_STATE_SELECTIR;
				}
				//otherwise, advance towards the data states
				else
				{
					//advance to Capture Data
					tms_advance(0);
					jtag_tap_state = TAP_STATE_CAPTUREDR;
				}

				break;

			//Capture Data
			case TAP_STATE_CAPTUREDR:

				//if the desired state is Shift Data, go there
				if(new_state == TAP_STATE_SHIFTDR)
				{
					tms_advance(0);
					jtag_tap_state = TAP_STATE_SHIFTDR;
				}
				//otherwise, move to the first exit point
				else
				{
					tms_advance(1);
					jtag_tap_state = TAP_STATE_EXIT1DR;
				}
				break;

			//Shift Data Register
			case TAP_STATE_SHIFTDR:
				//advance to the first exit point
				tms_advance(1);
				jtag_tap_state = TAP_STATE_EXIT1DR;
				break;

			//First Exit Point
			case TAP_STATE_EXIT1DR:

				//If we're looking to pause, pause
				if(new_state == TAP_STATE_PAUSEDR)
				{
					tms_advance(0);
					jtag_tap_state = TAP_STATE_PAUSEDR;
				}
				//otherwise, move to the data update state
				else
				{
					tms_advance(1);
					jtag_tap_state = TAP_STATE_UPDATEDR;
				}
				break;

			//Data wait state (pause)
			case TAP_STATE_PAUSEDR:

				//advance to the second exit point
				tms_advance(1);
				jtag_tap_state = TAP_STATE_EXIT2DR;

				break;

			//second data exit point
			case TAP_STATE_EXIT2DR:

				//if the desired state is another shiftDR state,
				//re-enter the data loop
				if(new_state == TAP_STATE_SHIFTDR)
				{
					tms_advance(0);
					jtag_tap_state = TAP_STATE_SHIFTDR;
				}
				//otherwise, let the device use the data register contents
				else
				{
					tms_advance(1);
					jtag_tap_state = TAP_STATE_UPDATEDR;
				}
				break;

			//process new data state (Update-DR)
			case TAP_STATE_UPDATEDR:

				//return to IDLE if requested
				//(this usually occurs when switching to the IR)
				if(new_state == TAP_STATE_IDLE)
				{
					tms_advance(0);
					jtag_tap_state = TAP_STATE_IDLE;
				}
				//otherwise, re-enter the Data Register branch (selectDR)
				else
				{
					tms_advance(1);
					jtag_tap_state = TAP_STATE_SELECTDR;
				}
				break;

			//entry point to the instruction register branch
			//Select-IR mode
			case TAP_STATE_SELECTIR:

				//advance to instruction capture
				tms_advance(0);
				jtag_tap_state = TAP_STATE_CAPTUREIR;
				break;

			//Instruction capture state
			case TAP_STATE_CAPTUREIR:

				//if the desired state is ShiftIR (instruction input),
				//advance to it
				if(new_state == TAP_STATE_SHIFTIR)
				{
					tms_advance(0);
					jtag_tap_state = TAP_STATE_SHIFTIR;
				}
				//otherwise, move to the first exit point
				else
				{
					tms_advance(1);
					jtag_tap_state = TAP_STATE_EXIT1IR;
				}
				break;

			//Data input state (ShiftIR)
			case TAP_STATE_SHIFTIR:

				//advance to the first exit point
				tms_advance(1);
				jtag_tap_state = TAP_STATE_EXIT1IR;
				break;

			//First instruction exit point
			case TAP_STATE_EXIT1IR:
				//if we're asked to pause, go to the wait state
				if(new_state == TAP_STATE_PAUSEIR)
				{
					tms_advance(0);
					jtag_tap_state = TAP_STATE_PAUSEIR;
				}
				//otherwise, move to the instruction processing state (UpdateIR)
				else
				{
					tms_advance(1);
					jtag_tap_state = TAP_STATE_UPDATEIR;
				}
				break;

			//Instruction wait state (PauseIR)
			case TAP_STATE_PAUSEIR:

				//advance to second exit point
				tms_advance(1);
				jtag_tap_state = TAP_STATE_EXIT2IR;
				break;

			//Second instruction exit point
			case TAP_STATE_EXIT2IR:

				//if the user wants to continue instr entry
				if(new_state == TAP_STATE_SHIFTIR)
				{
					tms_advance(0);
					jtag_tap_state = TAP_STATE_SHIFTIR;
				}
				//otherwise, move to the update state
				else
				{
					tms_advance(1);
					jtag_tap_state = TAP_STATE_UPDATEIR;
				}
				break;

			//instruction update state
			case TAP_STATE_UPDATEIR:
				//if the user is done entering instructions
				if(new_state == TAP_STATE_IDLE)
				{
					//move to Run-Test-Idle
					tms_advance(0);
					jtag_tap_state = TAP_STATE_IDLE;
				}
				//otherwise, re-enter the mode select tree
				else
				{
					tms_advance(1);
					jtag_tap_state = TAP_STATE_SELECTDR;
				}
				break;
		}
	}

#ifdef DEBUG_JTAG
	//printf("State changed.\n\n");
#endif
}

/*
 * tms_reset
 *
 * Resets the target device without the TRST pin.
 */
void tms_reset(void)
{
	//JTAG reset is five TCK cycles with TMS high

	//set the TMS pin high
	tms_set(1);

#ifdef DEBUG_JTAG
	printf("Resetting chain...\n");
#endif

	//and clock five times
	for(int i=0; i < 5; ++i)
		tck_pulse();

	tms_set(0);

	//set the known state to reset
	jtag_tap_state = TAP_STATE_RESET;
}


/*
 * set_tms
 *
 * Sets the TMS line to a given value.
 */
void tms_set(char value)
{
	if(value)
		JTAG_TMS_PORT |= 1 << JTAG_TMS_PIN;
	else
		JTAG_TMS_PORT &= ~(1 << JTAG_TMS_PIN);
}

inline void jtag_instruction_header(void)
{
	//FIXME
	jtag_shift_char(0xFF, 8, 0);
}

inline void jtag_instruction_trailer(void)
{
	//FIXME
}


/**
 * jtag_shift_instruction
 *
 * Sends a single char of an instruction to the target device.
 *
 * c:		A single character of the instruction to be sent.
 * bits:	The number of bits in the character to send, 8 or less.
 * first:	If nonzero, this is the first character in the instruction,
 * 			and will be prefixed with the appropriate headers.
 * more:	If nonzero, this is the last character in the instruction,
 * 			and will be suffixed with the appropriate trailers.
 * 			The device will move to the EXIT1 state.
 *
 */
void jtag_shift_instruction(char c, char bits, char first, char last)
{
	//prepare the device for instruction input
	if(first)
	{
		tap_set_state(TAP_STATE_SHIFTIR);
		jtag_instruction_header();
	}

	//and shift the instructions
	//FIXME: handle trailer(?)
	jtag_shift_char(c, bits, last);

	//if there's no more to shift, send the trailer
	if(last)
	{
		jtag_instruction_trailer();
		//tap_set_state(TAP_STATE_IDLE); //TODO: possibly remove
	}
}

inline void jtag_data_header(void)
{
	//FIXME: abstract
	jtag_shift_char(0x00, 1, 0);
}

inline void jtag_data_trailer(void)
{
	//FIXME abstract
}


//Stay in the Run-Test state for a set amount of clocks
void run_test(long clocks)
{
	//set the state to run test
	tap_set_state(TAP_STATE_RUNTEST);

	//and send the set amount of clocks
	for(long i = 0; i < clocks; ++i)
		tck_pulse();
}

/**
 * jtag_shift_data
 *
 * Sends a single char of data to the target device.
 *
 * c:		A single character of the data to be sent.
 * bits:	The number of bits in the character to send, 8 or less.
 * first:	If nonzero, this is the first character in the data,
 * 			and will be prefixed with the appropriate headers.
 * more:	If nonzero, this is the last character in the data,
 * 			and will be suffixed with the appropriate trailers.
 * 			The device will move to the EXIT1 state.
 *
 */
char jtag_shift_data(char c, char bits, char first, char last)
{
	char buffer;

	//if this is the first packet, prefix headers
	if(first)
	{
		//prepare the device for instruction input
		tap_set_state(TAP_STATE_SHIFTDR);
		jtag_data_header();
	}

	//and shift the instructions
	//FIXME: handle trailer
	buffer = jtag_shift_char(c, bits, last);


	//if there's no more to shift, send the trailer and
	//return to the idle state
	if(last)
	{
		jtag_data_trailer();
		//tap_set_state(TAP_STATE_IDLE); //TODO: possibly remove?
	}

	return buffer;
}

/*
 * jtag_shift_char
 *
 * Sends a single char over TDO;
 * receiving a character in the process.
 *
 * c:		The character to send, LSB first.
 * bits:	The amount of bits to send, max 8.
 * advance:	Advance to the next exit state after transmission.
 *
 */
static char jtag_shift_char(char c, char bits, char advance)
{
	char in = 0;

	//send each bit in the char
	int i;
	for(i = 0; i < bits; ++i)
	{

		//output each bit, one by one
		if(c & (1 << i))
			JTAG_TDI_PORT |= 1 << JTAG_TDI_PIN;
		else
			JTAG_TDI_PORT &= ~(1 << JTAG_TDI_PIN);



		#ifdef JTAG_BIT_DELAY
			#ifdef JTAG_SLOW_CLOCK
							_delay_ms(JTAG_BIT_DELAY);
			#else
							_delay_us(JTAG_BIT_DELAY);
			#endif
		#endif


		//if this is the last bit, advance to the exit point at the same time
		if(advance && (i == bits -1))
		{
			//advance to the exit point at the end of the shift
			in |= tms_advance(1) << i;
			++jtag_tap_state;
		}
		//otherwise, pulse the clock and store the value of TDO just before the edge
		else
		{
			//pulse the clock, and store the value of TDO just before the edge
			in |= tck_pulse() << i;
		}

	}

	#ifdef DEBUG_JTAG
		printf("Shifted out %d bits of %x, received %x.\n", bits, c, in);
	#endif

	//return the character received
	return in;
}
