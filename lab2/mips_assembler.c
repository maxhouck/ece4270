#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>

#include "mips_assembler.h"

const char *regName[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0",
	"t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4",
	"s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};
	//the register numbers correspond to the array number

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
	assembly_instruction = parse_file();
	while(file_flag != NULL) {
		//printf("%s\n", assembly_instruction);
		machine_instruction = parse_instruction(assembly_instruction);
		write_file(machine_instruction);
		assembly_instruction = parse_file();
		//printf("%x %s", machine_instruction, assembly_instruction);
	}
	fclose(input);
	fclose(output);

}

//load input file and parse out every line
char* parse_file() {
	static char current_line[100];
	file_flag = fgets(current_line, sizeof(current_line), input);
	return current_line;

}

//take the register name and return the register num. -1 for fail.
uint32_t nameToNum(char* registerName)
{
	uint32_t i = 0;

	for(i=0;i<32;i++)
	{
		if(strcmp(registerName, regName[i]) == 0)
		{
			return i;
		}
	}

	return -1;
}

uint32_t limitBits(uint32_t line, int bits)
{
	uint32_t num = 0;

	num = num | ((1 << bits) - 1);
	//printf("%x\n", num);

	line = line & num;

	return line;
}

//parse one line
uint32_t parse_instruction(char* line)
{
	//printf("Test\n");
	//printf("%s\n", line);
	uint32_t machine_instruction = 0;
	uint32_t rt, rd, sa, immediate, rs, base, target, offset;//, target = 0;
	char *instruction, *arg1, *arg2, *arg3;
	const char s[6] = " ,$()"; //can use multiple delimiters
	instruction = strtok(line, s);
	//printf("Test2\n");
	//printf("%s\n", instruction);

	int i = 0;
	while(instruction[i] != '\0')
	{
		instruction[i] = toupper(instruction[i]);
		i++;
	}

	printf("%s\n", instruction);

	if(strcmp(instruction, "SLL") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg2);
		rd = nameToNum(arg1);
		sa = (uint32_t)strtoul(arg3, NULL, 0);

		sa = limitBits(sa, 5);

		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = (sa << 6) | machine_instruction;
	}
	else if(strcmp(instruction, "SRL") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg2);
		rd = nameToNum(arg1);
		sa = (uint32_t)strtoul(arg3, NULL, 0);

		sa = limitBits(sa, 5);

		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = (sa << 6) | machine_instruction;
		machine_instruction = 0b000010 | machine_instruction;
	}
	else if(strcmp(instruction, "SRA") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg2);
		rd = nameToNum(arg1);
		sa = (uint32_t)strtoul(arg3, NULL, 0);

		sa = limitBits(sa, 5);

		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = (sa << 6) | machine_instruction;
		machine_instruction = 0b000011 | machine_instruction;
	}
	else if(strcmp(instruction, "MULT") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		rs = nameToNum(arg1);
		rt = nameToNum(arg2);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = 0b011000 | machine_instruction;
	}
	else if(strcmp(instruction, "MULTU") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		rs = nameToNum(arg1);
		rt = nameToNum(arg2);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = 0b011001 | machine_instruction;
	}
	else if(strcmp(instruction, "ADD") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rd = nameToNum(arg1);
		rs = nameToNum(arg2);
		rt = nameToNum(arg3);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b100000 | machine_instruction;
	}
	else if(strcmp(instruction, "ADDU") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rd = nameToNum(arg1);
		rs = nameToNum(arg2);
		rt = nameToNum(arg3);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b100001 | machine_instruction;
	}
	else if(strcmp(instruction, "SUB") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rd = nameToNum(arg1);
		rs = nameToNum(arg2);
		rt = nameToNum(arg3);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b100010 | machine_instruction;
	}
	else if(strcmp(instruction, "SUBU") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rd = nameToNum(arg1);
		rs = nameToNum(arg2);
		rt = nameToNum(arg3);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b100011 | machine_instruction;
	}
	else if(strcmp(instruction, "AND") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rd = nameToNum(arg1);
		rs = nameToNum(arg2);
		rt = nameToNum(arg3);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b100100 | machine_instruction;
	}
	else if(strcmp(instruction, "OR") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rd = nameToNum(arg1);
		rs = nameToNum(arg2);
		rt = nameToNum(arg3);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b100101 | machine_instruction;
	}
	else if(strcmp(instruction, "XOR") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rd = nameToNum(arg1);
		rs = nameToNum(arg2);
		rt = nameToNum(arg3);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b100110 | machine_instruction;
	}
	else if(strcmp(instruction, "NOR") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rd = nameToNum(arg1);
		rs = nameToNum(arg2);
		rt = nameToNum(arg3);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b100111 | machine_instruction;
	}
	else if(strcmp(instruction, "SLT") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rd = nameToNum(arg1);
		rs = nameToNum(arg2);
		rt = nameToNum(arg3);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b101010 | machine_instruction;
	}
	else if(strcmp(instruction, "DIV") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		rs = nameToNum(arg1);
		rt = nameToNum(arg2);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = 0b011010 | machine_instruction;
	}
	else if(strcmp(instruction, "DIVU") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		rs = nameToNum(arg1);
		rt = nameToNum(arg2);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = 0b011011 | machine_instruction;
	}
	else if(strcmp(instruction, "MFHI") == 0)
	{
		arg1 = strtok(NULL, s);

		rd = nameToNum(arg1);

		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b010000 | machine_instruction;
	}
	else if(strcmp(instruction, "MFLO") == 0)
	{
		arg1 = strtok(NULL, s);

		rd = nameToNum(arg1);

		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b010010 | machine_instruction;
	}
	else if(strcmp(instruction, "MTHI") == 0)
	{
		arg1 = strtok(NULL, s);

		rs = nameToNum(arg1);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = 0b010010 | machine_instruction;
	}
	else if(strcmp(instruction, "MTLO") == 0)
	{
		arg1 = strtok(NULL, s);

		rs = nameToNum(arg1);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = 0b010011 | machine_instruction;
	}
	else if(strcmp(instruction, "JR") == 0)
	{
		arg1 = strtok(NULL, s);

		rs = nameToNum(arg1);

		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = 0b001000 | machine_instruction;
	}
	else if(strcmp(instruction, "JALR") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		if(arg2 == NULL)  { //JALR rs
			rs = nameToNum(arg1);
			rd = 31;
		}
		else {//JALR rd, rs
			rs = nameToNum(arg1);
			rd = nameToNum(arg2);
		}
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rd << 11) | machine_instruction;
		machine_instruction = 0b011010 | machine_instruction;
	}
	else if(strncmp(instruction, "SYSCALL", 7) == 0)
	{
		machine_instruction = 0b001100 | machine_instruction;
	}
	else if(strcmp(instruction, "ADDI") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		rs = nameToNum(arg2);
		immediate = (uint32_t)strtoul(arg3, NULL, 0);

		immediate = limitBits(immediate, 16);

		machine_instruction = (0b001000 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = immediate | machine_instruction;
	}
	else if(strcmp(instruction, "ADDIU") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		rs = nameToNum(arg2);
		immediate = (uint32_t)strtoul(arg3, NULL, 0);

		immediate = limitBits(immediate, 16);

		machine_instruction = (0b001001 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = immediate | machine_instruction;
	}
	else if(strcmp(instruction, "ANDI") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		rs = nameToNum(arg2);
		immediate = (uint32_t)strtoul(arg3, NULL, 0);

		immediate = limitBits(immediate, 16);

		machine_instruction = (0b001100 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = immediate | machine_instruction;
	}
	else if(strcmp(instruction, "ORI") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		rs = nameToNum(arg2);
		immediate = (uint32_t)strtoul(arg3, NULL, 0);

		immediate = limitBits(immediate, 16);

		machine_instruction = (0b001101 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = immediate | machine_instruction;
	}
	else if(strcmp(instruction, "XORI") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		rs = nameToNum(arg2);
		immediate = (uint32_t)strtoul(arg3, NULL, 0);

		immediate = limitBits(immediate, 16);

		machine_instruction = (0b001110 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = immediate | machine_instruction;
	}
	else if(strcmp(instruction, "LUI") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		rt = nameToNum(arg1);
		immediate = (uint32_t)strtoul(arg2, NULL, 0);

		immediate = limitBits(immediate, 16);

		machine_instruction = (0b001110 << 26) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = immediate | machine_instruction;
	}
	else if(strcmp(instruction, "SLTI") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		rs = nameToNum(arg2);
		immediate = (uint32_t)strtoul(arg3, NULL, 0);

		immediate = limitBits(immediate, 16);

		machine_instruction = (0b001110 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = immediate | machine_instruction;
	}
	else if(strcmp(instruction, "LB") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		base = nameToNum(arg3);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b100000 << 26) | machine_instruction;
		machine_instruction = (base << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "LH") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		base = nameToNum(arg3);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b100001 << 26) | machine_instruction;
		machine_instruction = (base << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "LW") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		base = nameToNum(arg3);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b100011 << 26) | machine_instruction;
		machine_instruction = (base << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "SB") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		base = nameToNum(arg3);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b101000 << 26) | machine_instruction;
		machine_instruction = (base << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "SH") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		base = nameToNum(arg3);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b101001 << 26) | machine_instruction;
		machine_instruction = (base << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "SW") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rt = nameToNum(arg1);
		base = nameToNum(arg3);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b101011 << 26) | machine_instruction;
		machine_instruction = (base << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "BEQ") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rs = nameToNum(arg1);
		rt = nameToNum(arg2);
		offset = (uint32_t)strtoul(arg3, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b000100 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "BNE") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);
		arg3 = strtok(NULL, s);

		rs = nameToNum(arg1);
		rt = nameToNum(arg2);
		offset = (uint32_t)strtoul(arg3, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b000101 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (rt << 16) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "BLEZ") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		rs = nameToNum(arg1);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b000110 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "BLTZ") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		rs = nameToNum(arg1);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b000001 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "BGEZ") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		rs = nameToNum(arg1);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b000001 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = (0b00001 << 16) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "BGTZ") == 0)
	{
		arg1 = strtok(NULL, s);
		arg2 = strtok(NULL, s);

		rs = nameToNum(arg1);
		offset = (uint32_t)strtoul(arg2, NULL, 0);

		offset = limitBits(offset, 16);

		machine_instruction = (0b000111 << 26) | machine_instruction;
		machine_instruction = (rs << 21) | machine_instruction;
		machine_instruction = offset | machine_instruction;
	}
	else if(strcmp(instruction, "J") == 0)
	{
		arg1 = strtok(NULL, s);

		target = (uint32_t)strtoul(arg1, NULL, 0);

		target = limitBits(target, 26);

		machine_instruction = (0b000010 << 26) | machine_instruction;
		machine_instruction = target | machine_instruction;
	}
	else if(strcmp(instruction, "JAL") == 0)
	{
		arg1 = strtok(NULL, s);

		target = (uint32_t)strtoul(arg1, NULL, 0);

		target = limitBits(target, 26);

		machine_instruction = (0b000011 << 26) | machine_instruction;
		machine_instruction = target | machine_instruction;
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
