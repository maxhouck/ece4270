#include <stdint.h>

#define FALSE 0
#define TRUE  1

/******************************************************************************/
/* MIPS memory layout                                                                                                                                      */
/******************************************************************************/
#define MEM_TEXT_BEGIN  0x00400000
#define MEM_TEXT_END      0x0FFFFFFF
/*Memory address 0x10000000 to 0x1000FFFF access by $gp*/
#define MEM_DATA_BEGIN  0x10010000
#define MEM_DATA_END   0x7FFFFFFF

#define MEM_KTEXT_BEGIN 0x80000000
#define MEM_KTEXT_END  0x8FFFFFFF

#define MEM_KDATA_BEGIN 0x90000000
#define MEM_KDATA_END  0xFFFEFFFF

/*stack and data segments occupy the same memory space. Stack grows backward (from higher address to lower address) */
#define MEM_STACK_BEGIN 0x7FFFFFFF
#define MEM_STACK_END  0x10010000

typedef struct {
	uint32_t begin, end;
	uint8_t *mem;
} mem_region_t;

/* memory will be dynamically allocated at initialization */
mem_region_t MEM_REGIONS[] = {
	{ MEM_TEXT_BEGIN, MEM_TEXT_END, NULL },
	{ MEM_DATA_BEGIN, MEM_DATA_END, NULL },
	{ MEM_KDATA_BEGIN, MEM_KDATA_END, NULL },
	{ MEM_KTEXT_BEGIN, MEM_KTEXT_END, NULL }
};

#define NUM_MEM_REGION 4
#define MIPS_REGS 32

typedef struct CPU_State_Struct {

  uint32_t PC;		                   /* program counter */
  uint32_t REGS[MIPS_REGS]; /* register file. */
  uint32_t HI, LO;                          /* special regs for mult/div. */
} CPU_State;

typedef struct CPU_Pipeline_Reg_Struct{
	uint32_t PC;
	uint32_t IR;
	uint32_t A;
	uint32_t B;
	uint32_t imm;
	uint32_t ALUOutput;
	uint32_t LO;
	uint32_t HI;
	uint32_t LMD;
	uint32_t RegWrite;
	uint32_t RegisterRd;
	uint32_t RegisterRs;
	uint32_t RegisterRt;
	int stalled;
} CPU_Pipeline_Reg;

/***************************************************************/
/* CPU State info.                                                                                                               */
/***************************************************************/

CPU_State CURRENT_STATE, NEXT_STATE;
int RUN_FLAG;	/* run flag*/
uint32_t INSTRUCTION_COUNT;
uint32_t CYCLE_COUNT;
uint32_t PROGRAM_SIZE; /*in words*/
int stalled;


/***************************************************************/
/* Pipeline Registers.                                                                                                        */
/***************************************************************/
CPU_Pipeline_Reg IF_ID;
CPU_Pipeline_Reg ID_EX;
CPU_Pipeline_Reg EX_MEM;
CPU_Pipeline_Reg MEM_WB;

char prog_file[32];

int ENABLE_FORWARDING;


/***************************************************************/
/* Function Declerations.                                                                                                */
/***************************************************************/
void help();
uint32_t mem_read_32(uint32_t address);
void mem_write_32(uint32_t address, uint32_t value);
void cycle();
void run(int num_cycles);
void runAll();
void mdump(uint32_t start, uint32_t stop) ;
void rdump();
void handle_command();
void reset();
void init_memory();
void load_program();
void handle_pipeline(); /*IMPLEMENT THIS*/
void WB();/*IMPLEMENT THIS*/
void MEM();/*IMPLEMENT THIS*/
void EX();/*IMPLEMENT THIS*/
void ID();/*IMPLEMENT THIS*/
void IF();/*IMPLEMENT THIS*/
void show_pipeline();/*IMPLEMENT THIS*/
void initialize();
void print_program(); /*IMPLEMENT THIS*/
void print_instruction(uint32_t addr);

typedef struct r_type_structure {
  uint8_t rs, rt, rd, shamt;
} r_type_struct;

r_type_struct parse_r_type(uint32_t instruction);

typedef struct i_type_structure {
  uint16_t immediate, offset; //these will be the same value, just renamed it for use with load commands
  uint8_t rs, rt, base;
} i_type_struct;

typedef struct j_type_structure {
	uint32_t target;
} j_type_struct;

i_type_struct parse_i_type(uint32_t instruction);
r_type_struct parse_r_type(uint32_t instruction);
j_type_struct parse_j_type(uint32_t instruction);
void check_data_hazard();

