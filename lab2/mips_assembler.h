#include <stdint.h>

//main accepts a file as input
int main(int argc, char *argv[]);

//load input file and parse out every line
char*[] parse_file(File *input);

//parse one line
uint32_t parse_instruction(char*);

char* assembly_instructions[];
uint32_t machine_instructions[];

