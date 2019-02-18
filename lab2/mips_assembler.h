#include <stdint.h>

FILE *input; //pointer to input file
FILE *output; //pointer to output file
int file_flag; //flag to signal EOF

//main accepts a file as input
int main(int argc, char *argv[]);

//load input file and parse out every line
char* parse_file();

//parse one line
uint32_t parse_instruction(char*);

void write_file(uint32_t);

//take the register name and return the register num. -1 for fail.
uint32_t nameToNum(char* registerName);
