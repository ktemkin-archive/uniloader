//FIXME
#pragma once

//When chained, these two instructions bypass the FPGA and program the PROM.
//BYPASS instructions
#define FPGA_BYPASS1_BITS 8
#define FPGA_BYPASS1_INST 0xe8
#define FPGA_BYPASS2_BITS 6
#define FPGA_BYPASS2_INST 0x3f

//FADDR instructions
#define FPGA_FADDR1_BITS 8
#define FPGA_FADDR1_INST 0xeb
#define FPGA_FADDR2_BITS 6
#define FPGA_FADDR2_INST 0x3f

void prom_init_config(void);
void prom_finish_config(void);
void prom_send_config(char c, bool first, bool last);
