#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mips_assembler.h"

//main accepts a file as input
int main(int argc, char *argv[])
{
	//check for correct number of arguments
	if(argc != 3) {
		printf("Incorrect number of arguments\nUsage: ./mips_assembler <inputfile> <outputfile>\n");
		exit(0);
	}
	input = fopen(argv[1], "r");
 	if (input == NULL) {  //check that input file is valid
		printf("Failed to open input file\n");
		exit(0);
	}
	output = fopen(argv[2], "w");
 	if (input == NULL) {  //check that output file is valid
		printf("Failed to open output file\n");
		exit(0);
	}
	char* assembly_instruction;
	uint32_t machine_instruction;
	while(file_flag != EOF) {
		assembly_instruction = parse_file();
		machine_instruction = parse_instruction(assembly_instruction);
		write_file(machine_instruction);
		printf("%x %s", machine_instruction, assembly_instruction);
	}
	fclose(input);
	fclose(output);
	
}

//load input file and parse out every line
char* parse_file() {
	char* current_line = 0;
	file_flag = fscanf(input, "%s", current_line);
	return current_line;
		
}

//parse one line
uint32_t parse_instruction(char* line)
{
	uint32_t machine_instruction;
	char *instruction, *arg1, *arg2, *arg3;
	const char s[2] = " ";
	instruction = strtok(line, s);

	if(strcmp(instruction, "SLL") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);
	}
	else if(strcmp(instruction, "SRL") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "SRA") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "MULT") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "MULTU") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "ADD") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "ADDU") == 0)
	{
		//do something[20]
	}
	else if(strcmp(instruction, "SUB") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "SUBU") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "AND") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "OR") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "XOR") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "NOR") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "SLT") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "DIV") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "DIVU") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "MFHI") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "MFLO") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "MTHI") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "MTLO") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "JR") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "JALR") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "syscall") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "ADDI") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "ADDIU") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "ANDI") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "ORI") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "XORI") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "LUI") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "SLTI") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "LB") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "LH") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "LW") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "SB") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "SH") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "SW") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "BEQ") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "BNE") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "BLEZ") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "BLTZ") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "BGEZ") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "BGTZ") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "J") == 0)
	{
		//do something
	}
	else if(strcmp(instruction, "JAL") == 0)
	{
		//do something
	}
	else
	{
		printf("Instruction not handled\n");
	}
	return machine_instruction;
}

void write_file(uint32_t machine_instruction) {
	fprintf(output, "%x\n", machine_instruction);
}


