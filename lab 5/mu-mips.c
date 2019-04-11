#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

int ENABLE_FORWARDING = 0;
int stall = 0;
uint32_t ID_EX_rs = 0;
uint32_t ID_EX_rt = 0;
uint32_t EX_MEM_RegisterRd = 0;
uint32_t EX_MEM_RegisterRt = 0;
uint32_t MEM_WB_RegisterRt = 0;
uint32_t MEM_WB_RegisterRd = 0;
int EX_MEM_RegWrite = 1;
int MEM_WB_RegWrite = 1;
int forwarding = 0;
int forwardB = 0;
uint32_t prevInstr = 0;

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("forwarding\t-- enable or disable data forwarding\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {
	handle_pipeline();
	CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {

	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the terminal                                              */
/***************************************************************/
void rdump() {
	int i;
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("# Cycles Executed\t: %u\n", CYCLE_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */
/***************************************************************/
void handle_command() {
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll();
			}
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value;
			NEXT_STATE.HI = hi_reg_value;
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program();
			break;
		case 'F':
		case 'f':
			if (scanf("%d", &ENABLE_FORWARDING) != 1) {
			break;
			}
			ENABLE_FORWARDING == 0 ? printf("Forwarding OFF\n") : printf("Forwarding ON\n");
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;

	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}

	/*load program*/
	load_program();

	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* maintain the pipeline                                                                                           */
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */
	NEXT_STATE = CURRENT_STATE;
	if(stall > 0) stall = stall-1;
	printf("\nin the handle pipeline function. stall is %d",stall);
	WB();
	MEM();
	EX();
	ID();
	IF();
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */
/************************************************************/
void WB()
{

	if(MEM_WB.stalled == 1)
	{
		return;
	}

	//if(MEM_WB.stalled == 0) {
		INSTRUCTION_COUNT++;
		uint8_t opcode = (MEM_WB.IR & 0xFC000000) >> 26;
		if(opcode == 0) { //if opcode is 0, then this is an R type instruction
			opcode = MEM_WB.IR & 0x00000003F; //switch opcode to the last 6 binary digits of instruction
			switch(opcode) {
				case 0b000000: { //SLL
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b000010: { //SRL
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b000011: { //SRA
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b011000: { //MULT
					NEXT_STATE.HI = MEM_WB.HI;
					NEXT_STATE.LO = MEM_WB.LO;
					break;
				}
				case 0b011001: { //MULTU
					NEXT_STATE.HI = MEM_WB.HI;
					NEXT_STATE.LO = MEM_WB.LO;
					break;
				}
				case 0b100000: { //ADD
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b100001: { //ADDIU
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b100010: { //SUB
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b100011: { //SUBU
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b100100: {//AND
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b100101: {//OR
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b100110: {//XOR
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b100111: {//NOR
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b101010: {//SLT
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.ALUOutput;
					break;
				}
				case 0b011010: { //DIV
					NEXT_STATE.HI = MEM_WB.HI;
					NEXT_STATE.LO = MEM_WB.LO;
					break;
				}
				case 0b011011: { //DIVU
					NEXT_STATE.HI = MEM_WB.HI;
					NEXT_STATE.LO = MEM_WB.LO;
					break;
				}
				case 0b010000: { //MFHI
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.HI;
					break;
				}
				case 0b010010: { //MFLO
					NEXT_STATE.REGS[MEM_WB.RegisterRd] = MEM_WB.LO;
					break;
				}
				case 0b010001: { //MTHI
					NEXT_STATE.HI = MEM_WB.ALUOutput;
					break;
				}
				case 0b010011: { //MTLO
					NEXT_STATE.LO = MEM_WB.ALUOutput;
					break;
				}
				case 0x0C: { //SYSTEMCALL
					if(NEXT_STATE.REGS[2] == 0xA)
					{
						RUN_FLAG = FALSE;
					}
					printf("SYSCALL\n");
					break;
				}
				default: {
					printf("this instruction has not been handled\t");
				}
			}
		}
		else { //if opcode is anything else this is an I or J type instruction
			int rt = (MEM_WB.IR & 0x001F0000) >> 16;
			switch(opcode) {
				case 0b001000: { //ADDI 001000 (for signed ints)
					NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
					break;
				}
				case 0b001001: { //ADDIU 001001 (for unsigned ints)
					NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
					break;
				}
				case 0b001100: { //ANDI
					NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
					break;
				}
				case 0b001101: { //ORI
					NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
					break;
				}
				case 0b001110: { //XORI
					NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
					break;
				}
				case 0b001111: { //LUI
					NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
					break;
				}
				case 0b001010: { //SLTI
					NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
					break;
				}
				case 0b100000: { //LB
					NEXT_STATE.REGS[rt] = MEM_WB.LMD;
					break;
				}
				case 0b100001: { //LH
					NEXT_STATE.REGS[rt] = MEM_WB.LMD;
					break;
				}
				case 0b100011: { //LW
					NEXT_STATE.REGS[rt] = MEM_WB.LMD;
					break;
				}
				case 0b101000: { //SB
					break;
				}
				case 0b101001: { //SH
					break;
				}
				case 0b101011: { //SW
					break;
				}
				default: {
					printf("this instruction has not been handled\t");
				}
			}
		}
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */
/************************************************************/
void MEM() {

		if(EX_MEM.stalled == 1)
		{
			MEM_WB.stalled = 1;
			MEM_WB.IR = 0;
			MEM_WB.PC = 0;
			MEM_WB.imm = 0;
			MEM_WB.A = 0;
			MEM_WB.B = 0;
			MEM_WB.ALUOutput = 0;
			MEM_WB.LMD = 0;
			MEM_WB.HI = 0;
			MEM_WB.LO = 0;
			MEM_WB.RegisterRt = 0;
			MEM_WB.RegisterRd = 0;
			MEM_WB.RegisterRs = 0;
			MEM_WB.RegWrite = 0;
			return;
		}
		MEM_WB.IR = EX_MEM.IR;
		MEM_WB.mem_access = EX_MEM.mem_access;
		MEM_WB.PC = EX_MEM.PC;
		MEM_WB.imm = EX_MEM.imm;
		MEM_WB.A = EX_MEM.A;
		MEM_WB.B = EX_MEM.B;
		MEM_WB.ALUOutput = EX_MEM.ALUOutput;
		MEM_WB.LMD = 0;
		MEM_WB.HI = EX_MEM.HI;
		MEM_WB.LO = EX_MEM.LO;
		MEM_WB.RegisterRt = EX_MEM.RegisterRt;
		MEM_WB.RegisterRd = EX_MEM.RegisterRd;
		MEM_WB.RegisterRs = EX_MEM.RegisterRs;
		MEM_WB.RegWrite = EX_MEM.RegWrite;
		MEM_WB.stalled = EX_MEM.stalled;

		uint32_t instruction;
		instruction = MEM_WB.IR;
		MEM_WB_RegisterRt = (instruction & 0x001F0000) >> 16;
		MEM_WB_RegisterRd = (instruction & 0x0000F800) >> 11;


		//Testing
		//MEM_WB.RegisterRd = (MEM_WB.IR & 0x0000F800) >> 11;

		uint8_t opcode = (MEM_WB.IR & 0xFC000000) >> 26;

		if(opcode == 0) { //if opcode is 0, then this is an R type instruction
			opcode = MEM_WB.IR & 0x00000003F; //switch opcode to the last 6 binary digits of instruction
			switch(opcode) {
				case 0x0C: { //SYSTEMCALL
					//We don't do anything with R type instructions
					//printf("SYSCALL\n");
					break;
				}
				default: {
					//printf("this instruction has not been handled\t");
					//No R type
				}
			}
		}
		else { //if opcode is anything else this is an I or J type instruction
			switch(opcode) {
				case 0b100000: { //LB
					uint32_t byte = 0xFF & mem_read_32(EX_MEM.ALUOutput);
					if(byte >> 7) {	// then negative number
						byte = (0xFFFFFF00 | byte); //sign extend with 1's
					}
					MEM_WB.LMD = byte;
					break;
				}
				case 0b100001: { //LH
					uint32_t halfword = 0xFFFF & mem_read_32(EX_MEM.ALUOutput);
					if(halfword >> 15) {	// then negative number
						halfword = (0xFFFF0000 | halfword); //sign extend with 1's
					}
					MEM_WB.LMD = halfword;
					break;
				}
				case 0b100011: { //LW
					uint32_t word = mem_read_32(EX_MEM.ALUOutput);
					MEM_WB.LMD = word;
					break;
				}
				case 0b101000: { //SB
					mem_write_32(EX_MEM.ALUOutput,EX_MEM.B);
					break;
				}
				case 0b101001: { //SH
					mem_write_32(EX_MEM.ALUOutput,EX_MEM.B);
					break;
				}
				case 0b101011: { //SW
					printf("\n ALUOutput is %d",EX_MEM.ALUOutput);
					mem_write_32(EX_MEM.ALUOutput,EX_MEM.B);
					break;
				}
				default: {
					//printf("this instruction has not been handled\t");
					//Not an instruction accessing memory
				}
			}
		}
	//}
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */
/************************************************************/
void EX() {
	if(ID_EX.stalled == 1) {
		EX_MEM.stalled = 1;
		EX_MEM.IR = 0;
		EX_MEM.PC = 0;
		EX_MEM.imm = 0;
		EX_MEM.A = 0;
		EX_MEM.B = 0;
		EX_MEM.ALUOutput = 0;
		EX_MEM.HI = 0;
		EX_MEM.LO = 0;
		EX_MEM.RegisterRt = 0;
		EX_MEM.RegisterRd = 0;
		EX_MEM.RegisterRs = 0;
		EX_MEM.RegWrite = 0;
		printf("\nEX did not run, it is stalled");
	}
	if(ID_EX.stalled == 0)	{
			printf("\nEX is running");
			EX_MEM.mem_access = ID_EX.mem_access;
			EX_MEM.stalled = ID_EX.stalled;
			EX_MEM.IR = ID_EX.IR;
			EX_MEM.PC = ID_EX.PC;
			EX_MEM.imm = ID_EX.imm;
			EX_MEM.A = ID_EX.A;
			EX_MEM.B = ID_EX.B;
			EX_MEM.ALUOutput = 0;
			EX_MEM.HI = 0;
			EX_MEM.LO = 0;
			EX_MEM.RegisterRt = ID_EX.RegisterRt;
			EX_MEM.RegisterRd = ID_EX.RegisterRd;
			EX_MEM.RegisterRs = ID_EX.RegisterRs;
			EX_MEM.RegWrite = ID_EX.RegWrite;

			uint32_t instruction = EX_MEM.IR;
			EX_MEM_RegisterRd = (instruction & 0x0000F800) >> 11;
			uint32_t rt = (instruction & 0x001F0000) >> 16;
			EX_MEM_RegisterRt = rt;


			if(EX_MEM.IR == 0)
				return;

			uint8_t opcode = (EX_MEM.IR & 0xFC000000) >> 26;
			if(opcode == 0) { //if opcode is 0, then this is an R type instruction
				opcode = EX_MEM.IR & 0x00000003F; //switch opcode to the last 6 binary digits of instruction
				switch(opcode) {
					case 0b000000: { //SLL
						EX_MEM.ALUOutput = EX_MEM.B << EX_MEM.imm;
						break;
					}
					case 0b000010: { //SRL
						EX_MEM.ALUOutput = EX_MEM.B >> EX_MEM.imm;
						break;
					}
					case 0b000011: { //SRA
						if(EX_MEM.B >> 31) {//if negative, sign extend
							uint32_t extension = 0xFFFFFFFF << (32-EX_MEM.imm);
							EX_MEM.ALUOutput = extension | (ID_EX.B >> EX_MEM.imm);
						}
						else //else same as SRL
							EX_MEM.ALUOutput = EX_MEM.B >> EX_MEM.imm;
						break;
					}
					case 0b011000: { //MULT
						if(EX_MEM.A >> 31) {	// then negative number
							EX_MEM.A = 0xFFFFFFFF00000000 | EX_MEM.A; //sign extend with 1's
						}
						if(ID_EX.B >> 31) {	// then negative number
							EX_MEM.B = 0xFFFFFFFF00000000 | EX_MEM.B; //sign extend with 1's
						}
						uint64_t result = EX_MEM.A * EX_MEM.B;
						EX_MEM.LO = (result); //low bit
						EX_MEM.HI = (result) >> 32; //high part
						break;
					}
					case 0b011001: { //MULTU
						uint64_t result = EX_MEM.A * EX_MEM.B;
						EX_MEM.LO = (result);// & 0xFFFFFFFF; //low bit
						EX_MEM.HI = (result) >> 32; //high part
						break;
					}
					case 0b100000: { //ADD
						uint8_t bit30carry = (((EX_MEM.A >> 30) & 0x1) + (0x1 & (EX_MEM.B >> 30))) >> 1;
		    				uint8_t bit31carry = (((EX_MEM.A >> 31) & 0x1) + (0x1 & (EX_MEM.B >> 31))) >> 1; //check for overflow
						if (bit30carry == bit31carry) //check for overflow exception
							EX_MEM.ALUOutput = EX_MEM.A + EX_MEM.B;
						break;
					}
					case 0b100001: { //ADDIU
						EX_MEM.ALUOutput = EX_MEM.A + EX_MEM.B;
						break;
					}
					case 0b100010: { //SUB
						uint8_t bit30carry = (((EX_MEM.A >> 30) & 0x1) + (0x1 & (EX_MEM.B >> 30))) >> 1;
						uint8_t bit31carry = (((EX_MEM.A >> 31) & 0x1) + (0x1 & (EX_MEM.B >> 31))) >> 1; //check for overflow
						if (bit30carry == bit31carry) //check for overflow exception
							EX_MEM.ALUOutput = EX_MEM.A - EX_MEM.B;
						break;
					}
					case 0b100011: { //SUBU
						EX_MEM.ALUOutput = EX_MEM.B - EX_MEM.A;
						break;
					}
					case 0b100100: {//AND
						EX_MEM.ALUOutput = EX_MEM.A & EX_MEM.B;
						break;
					}
					case 0b100101: {//OR
						EX_MEM.ALUOutput = EX_MEM.A | EX_MEM.B;
						break;
					}
					case 0b100110: {//XOR
						EX_MEM.ALUOutput = EX_MEM.A ^ EX_MEM.B;
						break;
					}
					case 0b100111: {//NOR
						EX_MEM.ALUOutput = (~EX_MEM.A) & (~EX_MEM.B);
						break;
					}
					case 0b101010: {//SLT
						uint32_t result = EX_MEM.A - EX_MEM.B;
						EX_MEM.ALUOutput = result < 0xF0000000 ? 0 : 1;
						break;
					}
					case 0b011010: { //DIV
						EX_MEM.LO = EX_MEM.A / EX_MEM.B;
						EX_MEM.HI = EX_MEM.A % EX_MEM.B;
						break;
					}
					case 0b011011: { //DIVU
						EX_MEM.LO = EX_MEM.A / EX_MEM.B;
						EX_MEM.HI = EX_MEM.A % EX_MEM.B;
						break;
					}
					case 0b010000: { //MFHI
						break;
					}
					case 0b010010: { //MFLO
						break;
					}
					case 0b010001: { //MTHI
						EX_MEM.ALUOutput = EX_MEM.A;
						break;
					}
					case 0b010011: { //MTLO
						EX_MEM.ALUOutput = EX_MEM.A;
						break;
					}
					case 0x0C: { //SYSTEMCALL
						break;
					}
					default: {
						//printf("this instruction has not been handled\t");
					}
				}
			}
			else { //if opcode is anything else this is an I or J type instruction
				switch(opcode) {
					case 0b001000: { //ADDI 001000 (for signed ints)
						if(EX_MEM.imm >> 15) {	// then negative number
							EX_MEM.imm = 0xFFFF0000 | EX_MEM.imm; //sign extend with 1's
						}
						uint8_t bit30carry = (((EX_MEM.imm >> 30) & 0x1) + (0x1 & (EX_MEM.A >> 30))) >> 1;
						uint8_t bit31carry = (((EX_MEM.imm >> 31) & 0x1) + (0x1 & (EX_MEM.A >> 31))) >> 1; //check for overflow
						if (bit30carry == bit31carry) //check for overflow exception
							EX_MEM.ALUOutput = EX_MEM.imm + EX_MEM.A;
						break;
					}
					case 0b001001: { //ADDIU 001001 (for unsigned ints)
						if(EX_MEM.imm >> 15) {	// then negative number
							EX_MEM.imm = 0xFFFF0000 | EX_MEM.imm; //sign extend with 1's
						}
						EX_MEM.ALUOutput = EX_MEM.imm + EX_MEM.A;
						break;
					}
					case 0b001100: { //ANDI
						EX_MEM.ALUOutput = EX_MEM.imm & EX_MEM.A;
						break;
					}
					case 0b001101: { //ORI
						EX_MEM.ALUOutput = EX_MEM.imm | EX_MEM.A;
						break;
					}
					case 0b001110: { //XORI
						EX_MEM.ALUOutput = EX_MEM.imm ^ EX_MEM.A;
						break;
					}
					case 0b001111: { //LUI
						EX_MEM.ALUOutput = EX_MEM.imm << 16; //shift immediate and place into rt
						break;
					}
					case 0b001010: { //SLTI
						if(EX_MEM.imm >> 15) {	// then negative number
							EX_MEM.imm = 0xFFFF0000 | EX_MEM.imm; //sign extend with 1's
						}
						uint32_t result = EX_MEM.A - EX_MEM.imm;
						EX_MEM.ALUOutput = result < 0xF0000000 ? 0 : 1;
						break;
					}
					case 0b100000: //LB
					case 0b100001: //LH
					case 0b100011: //LW
					case 0b101000: //SB
					case 0b101001: //SH
					case 0b101011: { //SW
						if(EX_MEM.imm >> 15) {	// then negative number
							EX_MEM.imm = 0xFFFF0000 | EX_MEM.imm; //sign extend with 1's
						}
						EX_MEM.ALUOutput = EX_MEM.imm + EX_MEM.A;
						break;
					}
					default: {
						printf("this instruction has not been handled\t");
					}
				}
			}
		}
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */
/************************************************************/
void ID() //step 2
{
	printf("\nEntering ID. stall is: %d\n" ,stall);	
	if(stall > 0) {
		ID_EX.stalled = 1;
		IF_ID.IR = ID_EX.IR;
		printf("\nID did not run, because we need a stall");
		return;
	}
	if(stall == 0) {
		printf("ID stage is executing because stall == 0");
		ID_EX.IR = IF_ID.IR;
		ID_EX.PC = IF_ID.PC;
		ID_EX.imm = 0;
		ID_EX.A = 0;
		ID_EX.B = 0;
		ID_EX.RegWrite = 0;
		ID_EX.RegisterRt = 0;
		ID_EX.RegisterRd = 0;
		ID_EX.RegisterRs = 0;
		ID_EX.mem_access = 0;

		uint8_t opcode = (IF_ID.IR & 0xFC000000) >> 26;
		if(opcode == 0) { //if opcode is 0, then this is an R type instruction
			opcode = IF_ID.IR & 0x00000003F; //switch opcode to the last 6 binary digits of instruction
			r_type_struct rstruct = parse_r_type(IF_ID.IR);
			ID_EX.RegisterRs = rstruct.rs;
			ID_EX.RegisterRt = rstruct.rt;
			ID_EX.RegisterRd = rstruct.rt;
			ID_EX.RegWrite = 1;
			switch(opcode) {
				case 0b000000: { //SLL
					ID_EX.imm = rstruct.shamt;
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					break;
				}
				case 0b000010: { //SRL
					ID_EX.imm = rstruct.shamt;
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					break;
				}
				case 0b000011: { //SRA
					//sign extend in the EX() stage I think
					ID_EX.imm = rstruct.shamt;
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					break;
				}
				case 0b011000: { //MULT
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					break;
				}
				case 0b011001: { //MULTU
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					break;
				}
				case 0b100000: { //ADD
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b100001: { //ADDU
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b100010: { //SUB
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b100011: { //SUBU
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b100100: {//AND
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b100101: {//OR
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b100110: {//XOR
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b100111: {//NOR
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b101010: {//SLT
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b011010: { //DIV
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					break;
				}
				case 0b011011: { //DIVU
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.B = NEXT_STATE.REGS[rstruct.rt];
					break;
				}
				case 0b010000: { //MFHI
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b010010: { //MFLO
					ID_EX.RegisterRd = rstruct.rd;
					break;
				}
				case 0b010001: { //MTHI
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.RegWrite=0;
					break;
				}
				case 0b010011: { //MTLO
					ID_EX.A = NEXT_STATE.REGS[rstruct.rs];
					ID_EX.RegWrite=0;
					break;
				}
				case 0x0C: { //SYSTEMCALL
					break;
				}
				default: {
					printf("this instruction has not been handled\t");
				}
			}

		}
		else { //if opcode is anything else this is an I or J type instruction
			i_type_struct istruct = parse_i_type(IF_ID.IR);
			ID_EX.A = NEXT_STATE.REGS[istruct.rs];
			ID_EX.B = NEXT_STATE.REGS[istruct.rt];
			ID_EX.imm = istruct.immediate;
			ID_EX.RegisterRs = istruct.rs;
			ID_EX.RegisterRt = istruct.rt;
			ID_EX.RegisterRd = istruct.rt;

			switch(opcode) {
				case 0b100011: //LW
					ID_EX.mem_access = 1;
				case 0b101000: //SB
				case 0b101001: //SH
				case 0b101011: { //SW
					ID_EX.RegWrite = 0;
				}
				default: {
					ID_EX.RegWrite = 1;
				}
			}
		}
	check_data_hazard();
	if(stall != 0)
	{
		printf("data hazard detected in ID");
		ID_EX.stalled = 1;

	}
	else {
		ID_EX.stalled = 0;
	}
		if(forwarding == 1) {
			ID_EX.A = EX_MEM.ALUOutput;
			forwarding = 0;
		}
		if(forwarding == 2) {
			ID_EX.B = EX_MEM.ALUOutput;
			forwarding = 0;
		}
		if(forwarding == 2) {
			ID_EX.A = MEM_WB.LMD;
			forwarding = 0;
		}
		if(forwarding == 2) {
			ID_EX.B = MEM_WB.LMD;
			forwarding = 0;
		}
	}
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */
/************************************************************/
void IF() //step 1
{
	if(stall == 0) {
		IF_ID.IR = mem_read_32(CURRENT_STATE.PC);
		IF_ID.PC = CURRENT_STATE.PC + sizeof(uint32_t); //increment counter
		NEXT_STATE.PC = IF_ID.PC;
	}
	else {
		printf("\nIF is not running, we are stalled\n");
	}
}

i_type_struct parse_i_type(uint32_t instruction) {
	i_type_struct istruct;
	istruct.immediate = instruction & 0xFFFF;
	istruct.rt = (instruction & 0x001F0000) >> 16;
	istruct.rs = (instruction & 0x03E00000) >> 21;
	istruct.offset = istruct.immediate;
	istruct.base = istruct.rs;
	return istruct;
}

r_type_struct parse_r_type(uint32_t instruction) {
	r_type_struct rstruct;
	rstruct.rs = (instruction & 0x03E00000) >> 21;
	rstruct.rt = (instruction & 0x001F0000) >> 16;
	rstruct.rd = (instruction & 0x0000F800) >> 11;
	rstruct.shamt = (instruction & 0x000007C0) >> 6;
	return rstruct;
}

/************************************************************/
/* Check for Hazard                                                                                                   */
/************************************************************/
void check_data_hazard() {
	int reset = ENABLE_FORWARDING;
	if(EX_MEM.mem_access == 1 || MEM_WB.mem_access == 1) ENABLE_FORWARDING = 0;
	if((EX_MEM.RegWrite && (EX_MEM.RegisterRd != 0)) && (EX_MEM.RegisterRd == ID_EX.RegisterRs)) {
			if(ENABLE_FORWARDING == 1) {
				forwarding = 1;
			} else {
				stall = 2;
			}
		}
		if((EX_MEM.RegWrite && (EX_MEM.RegisterRd != 0)) && (EX_MEM.RegisterRd == ID_EX.RegisterRt)) {
			if(ENABLE_FORWARDING == 1) {
				forwarding = 2;
			} else {
				stall = 2;
			}
		}
		if((EX_MEM.RegWrite && (EX_MEM.RegisterRt != 0)) && (EX_MEM.RegisterRt == ID_EX.RegisterRs)) {
			if(ENABLE_FORWARDING == 1) {
				forwarding = 1;
			} else {
				stall = 2;
			}
		}
		if((EX_MEM.RegWrite && (EX_MEM.RegisterRt != 0)) && (EX_MEM.RegisterRt == ID_EX.RegisterRt)) {
			if(ENABLE_FORWARDING == 1) {
				forwarding = 2;
			} else {
				stall = 2;
			}
		}
		if((MEM_WB.RegWrite && (MEM_WB.RegisterRt != 0)) && (MEM_WB.RegisterRt == ID_EX.RegisterRs)) {
			if(ENABLE_FORWARDING == 1) {
				forwarding = 3;
			} else {
				stall = 1;
			}
		}
		if((MEM_WB.RegWrite && (MEM_WB.RegisterRt != 0)) && (MEM_WB.RegisterRt == ID_EX.RegisterRt)) {
			if(ENABLE_FORWARDING == 1) {
				forwarding = 4;
			} else {
				stall = 1;
			}
		}
		if((MEM_WB.RegWrite && (MEM_WB.RegisterRd != 0)) && (MEM_WB.RegisterRd == ID_EX.RegisterRs)) {
			if(ENABLE_FORWARDING == 1) {
				forwarding = 3;
			} else {
				stall = 1;
			}
		}
		if((MEM_WB.RegWrite && (MEM_WB.RegisterRd != 0)) && (MEM_WB.RegisterRd == ID_EX.RegisterRt)) {
			if(ENABLE_FORWARDING == 1) {
				forwarding = 4;
			} else {
				stall = 1;
			}
		}
	ENABLE_FORWARDING = reset;

}

/************************************************************/
/* Initialize Memory                                                                                                    */
/************************************************************/
void initialize() {
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
	ENABLE_FORWARDING = FALSE;
	stalled = 0;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */
/************************************************************/
void print_program(){
	/*IMPLEMENT THIS*/

	int i;
	uint32_t addr;

	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}
/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
	//read first instruction from memory
	uint32_t instruction = mem_read_32(addr);

	//check for i or j or r type
	uint8_t opcode = (instruction & 0xFC000000) >> 26;

	if(opcode == 0) { //if opcode is 0, then this is an R type instruction
		opcode = instruction & 0x00000003F; //switch opcode to the last 6 binary digits of instruction
		switch(opcode) {
			case 0b000000: { //SLL
				r_type_struct rstruct = parse_r_type(instruction);

				printf("SLL $%d, $%d, %x\n", rstruct.rd, rstruct.rt, rstruct.shamt);
				break;
			}
			case 0b000010: { //SRL
				r_type_struct rstruct = parse_r_type(instruction);

				printf("SRL $%d, $%d, %x\n", rstruct.rd, rstruct.rt, rstruct.shamt);

				break;
			}
			case 0b011000: { //MULT
				r_type_struct rstruct = parse_r_type(instruction);

				printf("MULT $%d, $%d\n", rstruct.rs, rstruct.rt);


				break;
			}
			case 0b011001: { //MULTU
				r_type_struct rstruct = parse_r_type(instruction);

				printf("MULTU $%d, $%d\n", rstruct.rs, rstruct.rt);
				break;
			}
			case 0b100000: { //ADD
				r_type_struct rstruct = parse_r_type(instruction);


				printf("ADD $%d, $%d, $%d\n", rstruct.rd, rstruct.rs, rstruct.rt);
				break;
			}
			case 0b100001: { //ADDU
				r_type_struct rstruct = parse_r_type(instruction);

				printf("ADDU $%d, $%d, $%d\n", rstruct.rd, rstruct.rs, rstruct.rt);
				break;
			}
			case 0b100010: { //SUB
				r_type_struct rstruct = parse_r_type(instruction);

				printf("SUB $%d, $%d, $%d\n", rstruct.rd, rstruct.rs, rstruct.rt);
				break;
			}
			case 0b100011: { //SUBU
				r_type_struct rstruct = parse_r_type(instruction);

				printf("SUBU $%d, $%d, $%d\n", rstruct.rd, rstruct.rs, rstruct.rt);
				break;
			}
			case 0b011010: { //DIV
				r_type_struct rstruct = parse_r_type(instruction);

				printf("DIV $%d, $%d\n", rstruct.rs, rstruct.rt);
				break;
			}
			case 0b011011: { //DIVU
				r_type_struct rstruct = parse_r_type(instruction);

				printf("DIVU $%d, $%d\n", rstruct.rs, rstruct.rt);
				break;
			}
			case 0b100100: { //AND
				r_type_struct rstruct = parse_r_type(instruction);

				printf("AND $%d, $%d, $%d\n", rstruct.rd, rstruct.rs, rstruct.rt);
				break;
			}
			case 0b100101: { //OR
				r_type_struct rstruct = parse_r_type(instruction);

				printf("OR $%d, $%d, $%d\n", rstruct.rd, rstruct.rs, rstruct.rt);
				break;
			}
			case 0b100110: { //XOR
				r_type_struct rstruct = parse_r_type(instruction);

				printf("XOR $%d, $%d, $%d\n", rstruct.rd, rstruct.rs, rstruct.rt);
				break;
			}
			case 0b100111: { //NOR
				r_type_struct rstruct = parse_r_type(instruction);

				printf("NOR $%d, $%d, $%d\n", rstruct.rd, rstruct.rs, rstruct.rt);
				break;
			}
			case 0b101010: { //SLT
				r_type_struct rstruct = parse_r_type(instruction);

				printf("SLT $%d, $%d, $%d\n", rstruct.rd, rstruct.rs, rstruct.rt);
				break;
			}
			case 0b000011: { //SRA
				r_type_struct rstruct = parse_r_type(instruction);

				printf("SRA $%d, $%d, %x\n", rstruct.rd, rstruct.rt, rstruct.shamt);
				break;
			}
			case 0b010000: { //MFHI
				r_type_struct rstruct = parse_r_type(instruction);

				printf("MFHI $%d\n", rstruct.rd);
				break;
			}
			case 0b010010: { //MFLO
				r_type_struct rstruct = parse_r_type(instruction);

				printf("MFLO $%d\n", rstruct.rd);
				break;
			}
			case 0b010001: { //MTHI
				r_type_struct rstruct = parse_r_type(instruction);

				printf("MTHI $%d\n", rstruct.rs);
				break;
			}
			case 0b010011: { //MTLO
				r_type_struct rstruct = parse_r_type(instruction);

				printf("MTLO $%d\n", rstruct.rs);
				break;
			}
			case 0b001000: { //JR 001000
				r_type_struct rstruct = parse_r_type(instruction);

				printf("JR $%d\n", rstruct.rs);
				break;
			}
			case 0b001001: { //JALR 001001
				r_type_struct rstruct = parse_r_type(instruction);

				printf("JALR $%d\n", rstruct.rs);
				printf("JALR $%d, $%d", rstruct.rd, rstruct.rs);
				break;
			}
			case 0x0C: { //SYSTEMCALL

				printf("SYSCALL\n");
				break;
			}
			default: {
				printf("this instruction has not been handled\n");
			}
		}
	}
	else { //if opcode is anything else this is an I or J type instruction
		switch(opcode) {
			case 0b001000: { //ADDI 001000 (for signed ints)
				i_type_struct istruct = parse_i_type(instruction);

				printf("ADDI $%d, $%d, %x\n", istruct.rt, istruct.rs, istruct.immediate);
				break;
			}
			case 0b001001: { //ADDIU 001001 (for unsigned ints)
				i_type_struct istruct = parse_i_type(instruction);

				printf("ADDIU $%d, $%d, %x\n", istruct.rt, istruct.rs, istruct.immediate);
				break;
			}
			case 0b001111: { //LUI 001111
				i_type_struct istruct = parse_i_type(instruction);

				printf("LUI $%d, %x\n", istruct.rt, istruct.immediate);
				break;
			}
			case 0b001100: { //ANDI 001100
				i_type_struct istruct = parse_i_type(instruction);

				printf("ANDI $%d, $%d, %x\n", istruct.rt, istruct.rs, istruct.immediate);
				break;
			}
			case 0b001101: { //ORI 001101
				i_type_struct istruct = parse_i_type(instruction);

				printf("ORI $%d, $%d, %x\n", istruct.rt, istruct.rs, istruct.immediate);
				break;
			}
			case 0b001110: { //XORI 001110
				i_type_struct istruct = parse_i_type(instruction);

				printf("XORI $%d, $%d, %x\n", istruct.rt, istruct.rs, istruct.immediate);
				break;
			}
			case 0b001010: { //SLTI 001010
				i_type_struct istruct = parse_i_type(instruction);

				printf("SLTI $%d, $%d, %x\n", istruct.rt, istruct.rs, istruct.immediate);
				break;
			}
			case 0b100011: { //LW 100011
				i_type_struct istruct = parse_i_type(instruction);

				printf("LW $%d, %x($%d)\n", istruct.rt, istruct.immediate, istruct.rs);
				break;
			}
			case 0b100000: { //LB 100000
				i_type_struct istruct = parse_i_type(instruction);

				printf("LB $%d, %x($%d)\n", istruct.rt, istruct.immediate, istruct.rs);
				break;
			}
			case 0b100001: { //LH 100001
				i_type_struct istruct = parse_i_type(instruction);

				printf("LH $%d, %x($%d)\n", istruct.rt, istruct.immediate, istruct.rs);
				break;
			}
			case 0b101011: { //SW 101011
				i_type_struct istruct = parse_i_type(instruction);

				printf("SW $%d, %x($%d)\n", istruct.rt, istruct.immediate, istruct.rs);
				break;
			}
			case 0b101000: { //SB 101000
				i_type_struct istruct = parse_i_type(instruction);

				printf("SB $%d, %x($%d)\n", istruct.rt, istruct.immediate, istruct.rs);
				break;
			}
			case 0b101001: { //SH 101001
				i_type_struct istruct = parse_i_type(instruction);

				printf("SH $%d, %x($%d)\n", istruct.rt, istruct.immediate, istruct.rs);
				break;
			}
			case 0b000100: { //BEQ 000100
				i_type_struct istruct = parse_i_type(instruction);

				printf("BEQ $%d, $%d, %x\n", istruct.rs, istruct.rt, istruct.immediate);
				break;
			}
			case 0b000101: { //BNE 000101
				i_type_struct istruct = parse_i_type(instruction);

				printf("BNE $%d, $%d, %x\n", istruct.rs, istruct.rt, istruct.immediate);
				break;
			}
			case 0b000110: { //BLEZ 000110
				i_type_struct istruct = parse_i_type(instruction);

				printf("BLEZ $%d, %x\n", istruct.rs, istruct.immediate);
				break;
			}


			case 0b000001: { //REGIMM 000001
				i_type_struct istruct = parse_i_type(instruction);

				if(istruct.rt == 0b00000)//BLTZ
				{
					printf("BLTZ $%d, %x\n", istruct.rs, istruct.immediate);
				}
				else //BGEZ rt == 0b00001
				{
					printf("BGEZ $%d, %x\n", istruct.rs, istruct.immediate);
				}
				break;
			}
			case 0b000111: { //BGTZ 000111
				i_type_struct istruct = parse_i_type(instruction);

				printf("BGTZ $%d, %x\n", istruct.rs, istruct.immediate);
				break;
			}



			default: {
				printf("this instruction has not been handled\n");
			}
		}
	}
}


/************************************************************/
/* Print the current pipeline                                                                                    */
/************************************************************/
void show_pipeline(){
	printf("\nCurrent PC: %x\n", CURRENT_STATE.PC);
	printf("Stalled cycles: %x\n\n", stall);

	printf("IF/ID.IR: %x\n", IF_ID.IR);
	printf("IF/ID.PC: %x\n", IF_ID.PC);
	printf("IF/ID.RegWrite: %x\n\n", IF_ID.RegWrite);

	printf("ID/EX.IR: %x\n", ID_EX.IR);
	printf("ID/EX.A: %x\n", ID_EX.A);
	printf("ID/EX.B: %x\n", ID_EX.B);
	printf("ID/EX.imm: %x\n", ID_EX.imm);
	printf("ID/EX.RegWrite: %x\n", ID_EX.RegWrite);
	printf("ID_EX.RegisterRs: %x\n", ID_EX.RegisterRs);
	printf("ID_EX.RegisterRt: %x\n", ID_EX.RegisterRt);
	printf("ID_EX.RegisterRd: %x\n", ID_EX.RegisterRd);
	printf("ID_EX.stalled: %x\n\n", ID_EX.stalled);

	printf("EX/MEM.IR: %x\n", EX_MEM.IR);
	printf("EX/MEM.ALUOutput: %x\n", EX_MEM.ALUOutput);
	printf("EX/MEM.imm: %x\n", EX_MEM.imm);
	printf("EX/MEM.A: %x\n", EX_MEM.A);
	printf("EX/MEM.B: %x\n", EX_MEM.B);
	printf("EX/MEM.HI: %x\n", EX_MEM.HI);
	printf("EX/MEM.LO: %x\n", EX_MEM.LO);
	printf("EX/MEM.RegWrite: %x\n", EX_MEM.RegWrite);
	printf("EX_MEM.RegisterRs: %x\n", EX_MEM.RegisterRs);
	printf("EX_MEM.RegisterRt: %x\n", EX_MEM.RegisterRt);
	printf("EX_MEM.RegisterRd: %x\n", EX_MEM.RegisterRd);
		printf("EX_MEM.stalled: %x\n\n", EX_MEM.stalled);

	printf("MEM/WB.IR: %x\n", MEM_WB.IR);
	printf("MEM/WB.ALUOutput: %x\n", MEM_WB.ALUOutput);
	printf("MEM/WB.LMD: %x\n", MEM_WB.LMD);
	printf("MEM/WB.imm: %x\n", MEM_WB.imm);
	printf("MEM/WB.A: %x\n", MEM_WB.A);
	printf("MEM/WB.B: %x\n", MEM_WB.B);
	printf("MEM/WB.HI: %x\n", MEM_WB.HI);
	printf("MEM/WB.LO: %x\n", MEM_WB.LO);
	printf("MEM/WB.RegWrite: %x\n", MEM_WB.RegWrite);
	printf("MEM/WB.stalled: %x\n\n", MEM_WB.stalled);

}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");

	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
