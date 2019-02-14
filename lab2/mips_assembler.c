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

void write_file(uint32_t machine_instruction) {
	fprintf(output, "%x\n", machine_instruction);
}


