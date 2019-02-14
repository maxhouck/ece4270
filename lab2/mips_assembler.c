#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mips_assembler.h"

//main accepts a file as input
int main(int argc, char *argv[])
{
	//check for correct number of arguments and that the argument is a valid file	
	parse_file(argv[]);
}

//load input file and parse out every line
char*[] parse_file(File *input) {

}

//parse one line
uint32_t parse_instruction(char* line)
{
	char[10] instruction, arg1, arg2, arg3;
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
		//do something
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
}


char* assembly_instructions[];
uint32_t machine_instructions[];

