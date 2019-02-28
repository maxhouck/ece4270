#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

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
	//i think this whole block will go away
	MEM_WB.IR = EX_MEM.IR;
	MEM_WB.PC = EX_MEM.PC;
	MEM_WB.imm = EX_MEM.imm;
	MEM_WB.A = EX_MEM.A;
	MEM_WB.B = EX_MEM.B;
	MEM_WB.ALUOutput = EX_MEM.ALUOutput;
	MEM_WB.LMD = 0;

	uint8_t opcode = (MEM_WB.IR & 0xFC000000) >> 26;
	if(opcode == 0) { //if opcode is 0, then this is an R type instruction
		opcode = instruction & 0x00000003F; //switch opcode to the last 6 binary digits of instruction
		switch(opcode) {
			case 0b000000: { //SLL
				CPU_STATE.REGS[(MEM_WB.IR & 0x0000F800) >> 11] = MEM_WB.ALUOutput;
				break;
			}
			case 0b000010: { //SRL
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt;
				break;
			}
			case 0b000011: { //SRA
				r_type_struct rstruct = parse_r_type(instruction);
				if(CURRENT_STATE.REGS[rstruct.rt] >> 31) {//if negative, sign extend
					uint32_t extension = 0xFFFFFFFF << (32-rstruct.shamt);
					NEXT_STATE.REGS[rstruct.rd] = extension | (CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt);
				}
				else //else same as SRL
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt;
				break;
			}
			case 0b011000: { //MULT
				r_type_struct rstruct = parse_r_type(instruction);
				uint64_t rt = CURRENT_STATE.REGS[rstruct.rt];
				uint64_t rs = CURRENT_STATE.REGS[rstruct.rs];
				if(rt >> 31) {	// then negative number
					rt = 0xFFFFFFFF00000000 | rt; //sign extend with 1's
				}
				if(rs >> 31) {	// then negative number
					rs = 0xFFFFFFFF00000000 | rs; //sign extend with 1's
				}
				uint64_t result = rt * rs;
				NEXT_STATE.LO = (result); //low bit
				NEXT_STATE.HI = (result) >> 32; //high part
				break;
			}
			case 0b011001: { //MULTU
				r_type_struct rstruct = parse_r_type(instruction);
				uint64_t rt = CURRENT_STATE.REGS[rstruct.rt];
				uint64_t rs = CURRENT_STATE.REGS[rstruct.rs];
				uint64_t result = rt*rs;
				NEXT_STATE.LO = (result);// & 0xFFFFFFFF; //low bit
				NEXT_STATE.HI = (result) >> 32; //high part
				break;
			}
			case 0b100000: { //ADD
				r_type_struct rstruct = parse_r_type(instruction);
				uint8_t bit30carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] + CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b100001: { //ADDIU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] + CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b100010: { //SUB
				r_type_struct rstruct = parse_r_type(instruction);
				uint8_t bit30carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100011: { //SUBU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100100: {//AND
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] & CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100101: {//OR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] | CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100110: {//XOR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] ^ CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100111: {//NOR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = (~CURRENT_STATE.REGS[rstruct.rs]) & (~CURRENT_STATE.REGS[rstruct.rt]);
				break;
			}
			case 0b101010: {//SLT
				r_type_struct rstruct = parse_r_type(instruction);
				uint32_t result = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				NEXT_STATE.REGS[rstruct.rd] = result < 0xF0000000 ? 0 : 1;
				break;
			}
			case 0b011010: { //DIV
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = CURRENT_STATE.REGS[rstruct.rs] / CURRENT_STATE.REGS[rstruct.rt];
				NEXT_STATE.HI = CURRENT_STATE.REGS[rstruct.rs] % CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b011011: { //DIVU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = (CURRENT_STATE.REGS[rstruct.rs]) / (CURRENT_STATE.REGS[rstruct.rt]);
				NEXT_STATE.HI = (CURRENT_STATE.REGS[rstruct.rs]) % (CURRENT_STATE.REGS[rstruct.rt]);
				break;
			}
			case 0b010000: { //MFHI
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.HI;
				break;
			}
			case 0b010010: { //MFLO
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.LO;
				break;
			}
			case 0b010001: { //MTHI
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.HI = CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b010011: { //MTLO
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0x0C: { //SYSTEMCALL
				if(CURRENT_STATE.REGS[2] == 0xA)
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
		switch(opcode) {
			case 0b001000: { //ADDI 001000 (for signed ints)
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				uint8_t bit30carry = (((immediate >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[istruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((immediate >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[istruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[istruct.rt] = immediate + CURRENT_STATE.REGS[istruct.rs];
				break;
			}
			case 0b001001: { //ADDIU 001001 (for unsigned ints)
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = immediate + CURRENT_STATE.REGS[istruct.rs];
				break;
			}
			case 0b001100: { //ANDI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] & istruct.immediate;
				break;
			}
			case 0b001101: { //ORI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] | istruct.immediate;
				break;
			}
			case 0b001110: { //XORI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] ^ istruct.immediate;
				break;
			}
			case 0b001111: { //LUI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = istruct.immediate << 16; //shift immediate and place into rt
				break;
			}
			case 0b001010: { //SLTI
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				uint32_t result = CURRENT_STATE.REGS[istruct.rs] - immediate;
				NEXT_STATE.REGS[istruct.rt] = result < 0xF0000000 ? 0 : 1;
				break;
			}
			case 0b100000: { //LB
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				uint32_t byte = 0xFF & mem_read_32(address);
				if(byte >> 7) {	// then negative number
					byte = (0xFFFFFF00 | byte); //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = byte;
				break;
			}
			case 0b100001: { //LH
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				uint32_t halfword = 0xFFFF & mem_read_32(address);
				if(halfword >> 15) {	// then negative number
					halfword = (0xFFFF0000 | halfword); //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = halfword;
				break;
			}
			case 0b100011: { //LW
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				NEXT_STATE.REGS[istruct.rt] = mem_read_32(address);
				break;
			}
			case 0b101000: { //SB
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt] & 0xFF);
				break;
			}
			case 0b101001: { //SH
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt] & 0xFFFF);
				break;
			}
			case 0b101011: { //SW
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt]);
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
void MEM()
{
	MEM_WB.IR = EX_MEM.IR;
	MEM_WB.PC = EX_MEM.PC;
	MEM_WB.imm = EX_MEM.imm;
	MEM_WB.A = EX_MEM.A;
	MEM_WB.B = EX_MEM.B;
	MEM_WB.ALUOutput = EX_MEM.ALUOutput;
	MEM_WB.LMD = 0;

	uint8_t opcode = (MEM_WB.IR & 0xFC000000) >> 26;
	if(opcode == 0) { //if opcode is 0, then this is an R type instruction
		opcode = instruction & 0x00000003F; //switch opcode to the last 6 binary digits of instruction
		switch(opcode) {
			case 0b000000: { //SLL
				break;
			}
			case 0b000010: { //SRL
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt;
				break;
			}
			case 0b000011: { //SRA
				r_type_struct rstruct = parse_r_type(instruction);
				if(CURRENT_STATE.REGS[rstruct.rt] >> 31) {//if negative, sign extend
					uint32_t extension = 0xFFFFFFFF << (32-rstruct.shamt);
					NEXT_STATE.REGS[rstruct.rd] = extension | (CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt);
				}
				else //else same as SRL
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt;
				break;
			}
			case 0b011000: { //MULT
				r_type_struct rstruct = parse_r_type(instruction);
				uint64_t rt = CURRENT_STATE.REGS[rstruct.rt];
				uint64_t rs = CURRENT_STATE.REGS[rstruct.rs];
				if(rt >> 31) {	// then negative number
					rt = 0xFFFFFFFF00000000 | rt; //sign extend with 1's
				}
				if(rs >> 31) {	// then negative number
					rs = 0xFFFFFFFF00000000 | rs; //sign extend with 1's
				}
				uint64_t result = rt * rs;
				NEXT_STATE.LO = (result); //low bit
				NEXT_STATE.HI = (result) >> 32; //high part
				break;
			}
			case 0b011001: { //MULTU
				r_type_struct rstruct = parse_r_type(instruction);
				uint64_t rt = CURRENT_STATE.REGS[rstruct.rt];
				uint64_t rs = CURRENT_STATE.REGS[rstruct.rs];
				uint64_t result = rt*rs;
				NEXT_STATE.LO = (result);// & 0xFFFFFFFF; //low bit
				NEXT_STATE.HI = (result) >> 32; //high part
				break;
			}
			case 0b100000: { //ADD
				r_type_struct rstruct = parse_r_type(instruction);
				uint8_t bit30carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] + CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b100001: { //ADDIU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] + CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b100010: { //SUB
				r_type_struct rstruct = parse_r_type(instruction);
				uint8_t bit30carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100011: { //SUBU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100100: {//AND
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] & CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100101: {//OR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] | CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100110: {//XOR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] ^ CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100111: {//NOR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = (~CURRENT_STATE.REGS[rstruct.rs]) & (~CURRENT_STATE.REGS[rstruct.rt]);
				break;
			}
			case 0b101010: {//SLT
				r_type_struct rstruct = parse_r_type(instruction);
				uint32_t result = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				NEXT_STATE.REGS[rstruct.rd] = result < 0xF0000000 ? 0 : 1;
				break;
			}
			case 0b011010: { //DIV
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = CURRENT_STATE.REGS[rstruct.rs] / CURRENT_STATE.REGS[rstruct.rt];
				NEXT_STATE.HI = CURRENT_STATE.REGS[rstruct.rs] % CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b011011: { //DIVU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = (CURRENT_STATE.REGS[rstruct.rs]) / (CURRENT_STATE.REGS[rstruct.rt]);
				NEXT_STATE.HI = (CURRENT_STATE.REGS[rstruct.rs]) % (CURRENT_STATE.REGS[rstruct.rt]);
				break;
			}
			case 0b010000: { //MFHI
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.HI;
				break;
			}
			case 0b010010: { //MFLO
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.LO;
				break;
			}
			case 0b010001: { //MTHI
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.HI = CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b010011: { //MTLO
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0x0C: { //SYSTEMCALL
				if(CURRENT_STATE.REGS[2] == 0xA)
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
		switch(opcode) {
			case 0b001000: { //ADDI 001000 (for signed ints)
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				uint8_t bit30carry = (((immediate >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[istruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((immediate >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[istruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[istruct.rt] = immediate + CURRENT_STATE.REGS[istruct.rs];
				break;
			}
			case 0b001001: { //ADDIU 001001 (for unsigned ints)
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = immediate + CURRENT_STATE.REGS[istruct.rs];
				break;
			}
			case 0b001100: { //ANDI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] & istruct.immediate;
				break;
			}
			case 0b001101: { //ORI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] | istruct.immediate;
				break;
			}
			case 0b001110: { //XORI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] ^ istruct.immediate;
				break;
			}
			case 0b001111: { //LUI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = istruct.immediate << 16; //shift immediate and place into rt
				break;
			}
			case 0b001010: { //SLTI
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				uint32_t result = CURRENT_STATE.REGS[istruct.rs] - immediate;
				NEXT_STATE.REGS[istruct.rt] = result < 0xF0000000 ? 0 : 1;
				break;
			}
			case 0b100000: { //LB
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				uint32_t byte = 0xFF & mem_read_32(address);
				if(byte >> 7) {	// then negative number
					byte = (0xFFFFFF00 | byte); //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = byte;
				break;
			}
			case 0b100001: { //LH
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				uint32_t halfword = 0xFFFF & mem_read_32(address);
				if(halfword >> 15) {	// then negative number
					halfword = (0xFFFF0000 | halfword); //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = halfword;
				break;
			}
			case 0b100011: { //LW
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				NEXT_STATE.REGS[istruct.rt] = mem_read_32(address);
				break;
			}
			case 0b101000: { //SB
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt] & 0xFF);
				break;
			}
			case 0b101001: { //SH
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt] & 0xFFFF);
				break;
			}
			case 0b101011: { //SW
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt]);
				break;
			}
			default: {
				printf("this instruction has not been handled\t");
			}
		}
	}
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{
	EX_MEM.IR = ID_EX.IR;
	EX_MEM.PC = ID_EX.PC;
	EX_MEM.imm = ID_EX.imm;
	EX_MEM.A = ID_EX.A;
	EX_MEM.B = ID_EX.B;
	EX_MEM.ALUOutput = 0;

	uint8_t opcode = (EX_MEM.IR & 0xFC000000) >> 26;
	if(opcode == 0) { //if opcode is 0, then this is an R type instruction
		opcode = instruction & 0x00000003F; //switch opcode to the last 6 binary digits of instruction
		switch(opcode) {
			case 0b000000: { //SLL
				EX_MEM.ALUOutput = ID_EX.B << ID_EX.imm;
				break;
			}
			case 0b000010: { //SRL
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt;
				break;
			}
			case 0b000011: { //SRA
				r_type_struct rstruct = parse_r_type(instruction);
				if(CURRENT_STATE.REGS[rstruct.rt] >> 31) {//if negative, sign extend
					uint32_t extension = 0xFFFFFFFF << (32-rstruct.shamt);
					NEXT_STATE.REGS[rstruct.rd] = extension | (CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt);
				}
				else //else same as SRL
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt;
				break;
			}
			case 0b011000: { //MULT
				r_type_struct rstruct = parse_r_type(instruction);
				uint64_t rt = CURRENT_STATE.REGS[rstruct.rt];
				uint64_t rs = CURRENT_STATE.REGS[rstruct.rs];
				if(rt >> 31) {	// then negative number
					rt = 0xFFFFFFFF00000000 | rt; //sign extend with 1's
				}
				if(rs >> 31) {	// then negative number
					rs = 0xFFFFFFFF00000000 | rs; //sign extend with 1's
				}
				uint64_t result = rt * rs;
				NEXT_STATE.LO = (result); //low bit
				NEXT_STATE.HI = (result) >> 32; //high part
				break;
			}
			case 0b011001: { //MULTU
				r_type_struct rstruct = parse_r_type(instruction);
				uint64_t rt = CURRENT_STATE.REGS[rstruct.rt];
				uint64_t rs = CURRENT_STATE.REGS[rstruct.rs];
				uint64_t result = rt*rs;
				NEXT_STATE.LO = (result);// & 0xFFFFFFFF; //low bit
				NEXT_STATE.HI = (result) >> 32; //high part
				break;
			}
			case 0b100000: { //ADD
				r_type_struct rstruct = parse_r_type(instruction);
				uint8_t bit30carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] + CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b100001: { //ADDIU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] + CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b100010: { //SUB
				r_type_struct rstruct = parse_r_type(instruction);
				uint8_t bit30carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100011: { //SUBU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100100: {//AND
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] & CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100101: {//OR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] | CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100110: {//XOR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] ^ CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100111: {//NOR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = (~CURRENT_STATE.REGS[rstruct.rs]) & (~CURRENT_STATE.REGS[rstruct.rt]);
				break;
			}
			case 0b101010: {//SLT
				r_type_struct rstruct = parse_r_type(instruction);
				uint32_t result = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				NEXT_STATE.REGS[rstruct.rd] = result < 0xF0000000 ? 0 : 1;
				break;
			}
			case 0b011010: { //DIV
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = CURRENT_STATE.REGS[rstruct.rs] / CURRENT_STATE.REGS[rstruct.rt];
				NEXT_STATE.HI = CURRENT_STATE.REGS[rstruct.rs] % CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b011011: { //DIVU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = (CURRENT_STATE.REGS[rstruct.rs]) / (CURRENT_STATE.REGS[rstruct.rt]);
				NEXT_STATE.HI = (CURRENT_STATE.REGS[rstruct.rs]) % (CURRENT_STATE.REGS[rstruct.rt]);
				break;
			}
			case 0b010000: { //MFHI
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.HI;
				break;
			}
			case 0b010010: { //MFLO
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.LO;
				break;
			}
			case 0b010001: { //MTHI
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.HI = CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b010011: { //MTLO
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0x0C: { //SYSTEMCALL
				if(CURRENT_STATE.REGS[2] == 0xA)
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
		switch(opcode) {
			case 0b001000: { //ADDI 001000 (for signed ints)
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				uint8_t bit30carry = (((immediate >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[istruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((immediate >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[istruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[istruct.rt] = immediate + CURRENT_STATE.REGS[istruct.rs];
				break;
			}
			case 0b001001: { //ADDIU 001001 (for unsigned ints)
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = immediate + CURRENT_STATE.REGS[istruct.rs];
				break;
			}
			case 0b001100: { //ANDI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] & istruct.immediate;
				break;
			}
			case 0b001101: { //ORI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] | istruct.immediate;
				break;
			}
			case 0b001110: { //XORI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] ^ istruct.immediate;
				break;
			}
			case 0b001111: { //LUI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = istruct.immediate << 16; //shift immediate and place into rt
				break;
			}
			case 0b001010: { //SLTI
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				uint32_t result = CURRENT_STATE.REGS[istruct.rs] - immediate;
				NEXT_STATE.REGS[istruct.rt] = result < 0xF0000000 ? 0 : 1;
				break;
			}
			case 0b100000: { //LB
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				uint32_t byte = 0xFF & mem_read_32(address);
				if(byte >> 7) {	// then negative number
					byte = (0xFFFFFF00 | byte); //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = byte;
				break;
			}
			case 0b100001: { //LH
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				uint32_t halfword = 0xFFFF & mem_read_32(address);
				if(halfword >> 15) {	// then negative number
					halfword = (0xFFFF0000 | halfword); //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = halfword;
				break;
			}
			case 0b100011: { //LW
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				NEXT_STATE.REGS[istruct.rt] = mem_read_32(address);
				break;
			}
			case 0b101000: { //SB
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt] & 0xFF);
				break;
			}
			case 0b101001: { //SH
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt] & 0xFFFF);
				break;
			}
			case 0b101011: { //SW
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt]);
				break;
			}
			default: {
				printf("this instruction has not been handled\t");
			}
		}
	}
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID() //step 2
{
	ID_EX.IR = IF_ID.IR;
	ID_EX.PC = IF_ID.PC;
	ID_EX.imm = 0;
	ID_EX.A = 0;
	ID_EX.B = 0;

	uint8_t opcode = (IF_ID.IR & 0xFC000000) >> 26;
	if(opcode == 0) { //if opcode is 0, then this is an R type instruction
		opcode = instruction & 0x00000003F; //switch opcode to the last 6 binary digits of instruction
		switch(opcode) {
			case 0b000000: { //SLL
				r_type_struct rstruct = parse_r_type(instruction);
				ID_EX.imm = rstruct.shamt;
				ID_EX.B = CPU_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b000010: { //SRL
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt;
				break;
			}
			case 0b000011: { //SRA
				r_type_struct rstruct = parse_r_type(instruction);
				if(CURRENT_STATE.REGS[rstruct.rt] >> 31) {//if negative, sign extend
					uint32_t extension = 0xFFFFFFFF << (32-rstruct.shamt);
					NEXT_STATE.REGS[rstruct.rd] = extension | (CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt);
				}
				else //else same as SRL
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] >> rstruct.shamt;
				break;
			}
			case 0b011000: { //MULT
				r_type_struct rstruct = parse_r_type(instruction);
				uint64_t rt = CURRENT_STATE.REGS[rstruct.rt];
				uint64_t rs = CURRENT_STATE.REGS[rstruct.rs];
				if(rt >> 31) {	// then negative number
					rt = 0xFFFFFFFF00000000 | rt; //sign extend with 1's
				}
				if(rs >> 31) {	// then negative number
					rs = 0xFFFFFFFF00000000 | rs; //sign extend with 1's
				}
				uint64_t result = rt * rs;
				NEXT_STATE.LO = (result); //low bit
				NEXT_STATE.HI = (result) >> 32; //high part
				break;
			}
			case 0b011001: { //MULTU
				r_type_struct rstruct = parse_r_type(instruction);
				uint64_t rt = CURRENT_STATE.REGS[rstruct.rt];
				uint64_t rs = CURRENT_STATE.REGS[rstruct.rs];
				uint64_t result = rt*rs;
				NEXT_STATE.LO = (result);// & 0xFFFFFFFF; //low bit
				NEXT_STATE.HI = (result) >> 32; //high part
				break;
			}
			case 0b100000: { //ADD
				r_type_struct rstruct = parse_r_type(instruction);
				uint8_t bit30carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] + CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b100001: { //ADDIU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rt] + CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b100010: { //SUB
				r_type_struct rstruct = parse_r_type(instruction);
				uint8_t bit30carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((CURRENT_STATE.REGS[rstruct.rt] >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[rstruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100011: { //SUBU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100100: {//AND
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] & CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100101: {//OR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] | CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100110: {//XOR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.REGS[rstruct.rs] ^ CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b100111: {//NOR
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = (~CURRENT_STATE.REGS[rstruct.rs]) & (~CURRENT_STATE.REGS[rstruct.rt]);
				break;
			}
			case 0b101010: {//SLT
				r_type_struct rstruct = parse_r_type(instruction);
				uint32_t result = CURRENT_STATE.REGS[rstruct.rs] - CURRENT_STATE.REGS[rstruct.rt];
				NEXT_STATE.REGS[rstruct.rd] = result < 0xF0000000 ? 0 : 1;
				break;
			}
			case 0b011010: { //DIV
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = CURRENT_STATE.REGS[rstruct.rs] / CURRENT_STATE.REGS[rstruct.rt];
				NEXT_STATE.HI = CURRENT_STATE.REGS[rstruct.rs] % CURRENT_STATE.REGS[rstruct.rt];
				break;
			}
			case 0b011011: { //DIVU
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = (CURRENT_STATE.REGS[rstruct.rs]) / (CURRENT_STATE.REGS[rstruct.rt]);
				NEXT_STATE.HI = (CURRENT_STATE.REGS[rstruct.rs]) % (CURRENT_STATE.REGS[rstruct.rt]);
				break;
			}
			case 0b010000: { //MFHI
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.HI;
				break;
			}
			case 0b010010: { //MFLO
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.REGS[rstruct.rd] = CURRENT_STATE.LO;
				break;
			}
			case 0b010001: { //MTHI
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.HI = CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0b010011: { //MTLO
				r_type_struct rstruct = parse_r_type(instruction);
				NEXT_STATE.LO = CURRENT_STATE.REGS[rstruct.rs];
				break;
			}
			case 0x0C: { //SYSTEMCALL
				if(CURRENT_STATE.REGS[2] == 0xA)
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
		switch(opcode) {
			case 0b001000: { //ADDI 001000 (for signed ints)
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				uint8_t bit30carry = (((immediate >> 30) & 0x1) + (0x1 & (CURRENT_STATE.REGS[istruct.rs] >> 30))) >> 1;
				uint8_t bit31carry = (((immediate >> 31) & 0x1) + (0x1 & (CURRENT_STATE.REGS[istruct.rs] >> 31))) >> 1; //check for overflow
				if (bit30carry == bit31carry) //check for overflow exception
					NEXT_STATE.REGS[istruct.rt] = immediate + CURRENT_STATE.REGS[istruct.rs];
				break;
			}
			case 0b001001: { //ADDIU 001001 (for unsigned ints)
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = immediate + CURRENT_STATE.REGS[istruct.rs];
				break;
			}
			case 0b001100: { //ANDI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] & istruct.immediate;
				break;
			}
			case 0b001101: { //ORI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] | istruct.immediate;
				break;
			}
			case 0b001110: { //XORI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = CURRENT_STATE.REGS[istruct.rs] ^ istruct.immediate;
				break;
			}
			case 0b001111: { //LUI
				i_type_struct istruct = parse_i_type(instruction);
				NEXT_STATE.REGS[istruct.rt] = istruct.immediate << 16; //shift immediate and place into rt
				break;
			}
			case 0b001010: { //SLTI
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t immediate = istruct.immediate;
				if(immediate >> 15) {	// then negative number
					immediate = 0xFFFF0000 | immediate; //sign extend with 1's
				}
				uint32_t result = CURRENT_STATE.REGS[istruct.rs] - immediate;
				NEXT_STATE.REGS[istruct.rt] = result < 0xF0000000 ? 0 : 1;
				break;
			}
			case 0b100000: { //LB
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				uint32_t byte = 0xFF & mem_read_32(address);
				if(byte >> 7) {	// then negative number
					byte = (0xFFFFFF00 | byte); //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = byte;
				break;
			}
			case 0b100001: { //LH
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				uint32_t halfword = 0xFFFF & mem_read_32(address);
				if(halfword >> 15) {	// then negative number
					halfword = (0xFFFF0000 | halfword); //sign extend with 1's
				}
				NEXT_STATE.REGS[istruct.rt] = halfword;
				break;
			}
			case 0b100011: { //LW
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				NEXT_STATE.REGS[istruct.rt] = mem_read_32(address);
				break;
			}
			case 0b101000: { //SB
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt] & 0xFF);
				break;
			}
			case 0b101001: { //SH
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt] & 0xFFFF);
				break;
			}
			case 0b101011: { //SW
				i_type_struct istruct = parse_i_type(instruction);
				uint32_t address = istruct.offset;
				if(address >> 15) {	// then negative number
					address = 0xFFFF0000 | address; //sign extend with 1's
				}
				address += CURRENT_STATE.REGS[istruct.base];
				mem_write_32(address,CURRENT_STATE.REGS[istruct.rt]);
				break;
			}
			default: {
				printf("this instruction has not been handled\t");
			}
		}
	}

}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF() //step 1
{
	ID_IF.IR = mem_read_32(ID_IF.PC);
	ID_IF.PC = ID_IF.PC + sizeof(uint32_t); //increment counter
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
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	/*IMPLEMENT THIS*/
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
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
