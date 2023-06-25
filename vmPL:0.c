// Audiel Ortiz
// COP3402 Spring2022
// HW4 (vm.c)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#define MAX_REG_LENGTH 10
#define MAX_DATA_LENGTH 50
#define MAX_PROGRAM_LENGTH 150

// Initialize global variables to be able to be used across the code
struct instruction IR;
int line = 0;
int SP = -1;
int BP = 0;
int PC = 0;
int RP = MAX_REG_LENGTH;
int register_stack[MAX_REG_LENGTH];
int data_stack[MAX_DATA_LENGTH];

void print_execution(int line, char *opname, instruction IR, int PC, int BP, int SP, int RP, int *data_stack, int *register_stack)
{
	int i;
	// print out instruction and registers
	printf("%2d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t\t", line, opname, IR.l, IR.m, PC, BP, SP, RP);

	// print register stack
	for (i = MAX_REG_LENGTH - 1; i >= RP; i--)
		printf("%d ", register_stack[i]);
	printf("\n");

	// print data stack
	printf("\tdata stack : ");
	for (i = 0; i <= SP; i++)
		printf("%d ", data_stack[i]);
	printf("\n");
}
// Function to help variable in different activation record L levels down
int base(int L)
{
	int arb = BP;
	while (L > 0)
	{
		arb = data_stack[arb];
		L--;
	}
	return arb;
}

void execute_program(instruction *code, int printFlag)
{
	// Initialize halt flag to 1 and create opname string to hold opcode's name
	char opname[5];
	int halt = 1;
	if(printFlag)
	{
		printf("\t\t\t\tPC\tBP\tSP\tRP\n");
		printf("Initial values:\t\t\t%d\t%d\t%d\t%d\n", PC, BP, SP, RP);
		// Set while loop to continue until halt flag is changed in halt instruction
		while (halt == 1)
		{
			// Use instruction register to fetch current instruction
			IR = code[PC];
      PC++;
			// Copy opcode's name into opname and change corresponding values
			switch(IR.opcode)
			{
				case 1: strcpy(opname, "LIT");
					register_stack[RP - 1] = IR.m;
					RP--;
					break;

				//Use M to further find correct opcode
				case 2:
					switch(IR.m)
					{
						// Return decrements PC so that when it exits the switch it will get incremented again
						case 0: strcpy(opname, "RET");
							SP = BP - 1;
							BP = data_stack[SP + 2];
							PC = data_stack[SP + 3];
							break;

						case 1: strcpy(opname, "NEG");
							register_stack[RP] = register_stack[RP] * -1;
							break;

						case 2: strcpy(opname, "ADD");
							RP++;
							register_stack[RP] = register_stack[RP] + register_stack[RP - 1];
							break;

						case 3: strcpy(opname, "SUB");
							RP++;
							register_stack[RP] = register_stack[RP] - register_stack[RP - 1];
							break;

						case 4: strcpy(opname, "MUL");
							RP++;
							register_stack[RP] = register_stack[RP] * register_stack[RP - 1];
							break;

						case 5: strcpy(opname, "DIV");
							RP++;
							register_stack[RP] = register_stack[RP] / register_stack[RP - 1];
							break;

						case 6: strcpy(opname, "EQL");
							RP++;
							if(register_stack[RP] == register_stack[RP - 1])
								register_stack[RP] = 1;
							else
							 	register_stack[RP] = 0;
							break;

						case 7: strcpy(opname, "NEQ");
							RP++;
							if(register_stack[RP] != register_stack[RP - 1])
								register_stack[RP] = 1;
							else
								register_stack[RP] = 0;
							break;

						case 8: strcpy(opname, "LSS");
							RP++;
							if(register_stack[RP] < register_stack[RP - 1])
								register_stack[RP] = 1;
							else
								register_stack[RP] = 0;
							break;

						case 9: strcpy(opname, "LEQ");
							RP++;
							if(register_stack[RP] <= register_stack[RP - 1])
								register_stack[RP] = 1;
							else
								register_stack[RP] = 0;
							break;

						case 10: strcpy(opname, "GTR");
							RP++;
							if(register_stack[RP] > register_stack[RP - 1])
								register_stack[RP] = 1;
							else
								register_stack[RP] = 0;
							break;

						case 11: strcpy(opname, "GEQ");
							RP++;
							if(register_stack[RP] >= register_stack[RP - 1])
								register_stack[RP] = 1;
							else
								register_stack[RP] = 0;
							break;
						
						case 12: strcpy(opname, "AND");
							RP++;
							if(register_stack[RP] == 1 && register_stack[RP - 1] == 1)
								register_stack[RP] = 1;
							else
								register_stack[RP] = 0;
							break;

						case 13: strcpy(opname, "ORR");
							RP++;
							if(register_stack[RP] == 1 || register_stack[RP - 1] == 1)
								register_stack[RP] = 1;
							else
								register_stack[RP] = 0;
							break;
						
						case 14: strcpy(opname, "NOT");
							if(register_stack[RP] == 1)
								register_stack[RP] = 0;
							else if(register_stack[RP] == 0)
								register_stack[RP] = 1;

					}
						break;

				case 3: strcpy(opname, "LOD");
					RP--;
					register_stack[RP] = data_stack[base(IR.l) + IR.m];
					break;

				case 4: strcpy(opname, "STO");
					data_stack[base(IR.l) + IR.m] = register_stack[RP];
					RP++;
					break;

				case 5: strcpy(opname, "CAL");
					data_stack[SP + 1] = base(IR.l);
					data_stack[SP + 2] = BP;
					data_stack[SP + 3] = PC;
					BP = SP + 1;
					PC = IR.m;
					break;

				case 6: strcpy(opname, "INC");
					SP = SP + IR.m;
					break;

				case 7: strcpy(opname, "JMP");
					PC = IR.m;
					break;

				case 8: strcpy(opname, "JPC");
					if(register_stack[RP] == 0)
					{
						RP++;
						PC = IR.m;
					}
					else
					{
						RP++;
					}
					break;

				case 9:
					switch(IR.m)
					{
						case 1: strcpy(opname, "WRT");
							printf("Top of Stack Value: %d\n", register_stack[RP]);
							RP++;
							break;

						case 2: strcpy(opname, "RED");
							int value;
							printf("Please Enter an Integer: ");
							scanf("%d", &value);
							printf("\n");
							RP--;
							register_stack[RP] = value;
							break;

						case 3: strcpy(opname, "HAL");
							halt = 0;
						break;
					}
					break;
			}
			print_execution(line, opname, IR, PC, BP, SP, RP, data_stack, register_stack);
				// Adjust line variable duriung CAL, RET, JPC or JMP to match output
				if(IR.opcode == 7 || IR.opcode == 5 || IR.opcode == 8 || (IR.opcode == 2 && IR.m == 0));
					line = PC - 1;
				line++;
		}
	}
}
