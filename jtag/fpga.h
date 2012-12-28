#pragma once
//FIXME

//standard libs
#include <stdbool.h>

//JTAG functions
#include "core.h"

#define FPGA_POWER_PORT 	PORTC
#define FPGA_POWER_DDR 	DDRC
#define FPGA_POWER_PIN 	2

//FPGA JTAG INSTRUCTIONS

//Request IDCode
#define FPGA_IDCODE_BITS 6
#define FPGA_IDCODE_INST 0x09

//Initialize programming (same as pulsing PROG_B)
#define FPGA_JPROGRAM_BITS 6
#define FPGA_JPROGRAM_INST 0x0b

//Initialize configuration via JTAG
#define FPGA_CFG_IN_BITS 6
#define FPGA_CFG_IN_INST 0x05

//Start FPGA after configuration
#define FPGA_JSTART_BITS 6
#define FPGA_JSTART_INST 0x0C

void fpga_reset(void);
long fpga_get_idcode(void);
void fpga_set_power(char x);
void fpga_init_config(bool jtag_config);
void fpga_finish_config(void);
void fpga_send_config(char c, bool first, bool last);
