// Audiel Ortiz
// COP3402 Spring2022
// HW4 (parser.c)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

#define MAX_CODE_LENGTH 150
#define MAX_SYMBOL_COUNT 20
#define MAX_REG_HEIGHT 10

instruction *code;
int cIndex;
symbol *table;
int tIndex;
int level;
int currTok;
//  Use errorFlag to be able to recursively return out of functions and stop continuing execution
int errorFlag = 0;
// Use regHeight to keep track of how many registers are in use
int regHeight;
void emit(int opname, int level, int mvalue);
void addToSymbolTable(int k, char n[], int v, int l, int a, int m);
void mark();
int multipledeclarationcheck(char name[]);
int findsymbol(char name[], int kind);
void printparseerror(int err_code);
void printsymboltable();
void printassemblycode();

// Grammar function declarations
void block(lexeme *list);
void constDec(lexeme *list);
int varDec(lexeme *list);
void procDec(lexeme *list);
void statement(lexeme *list);
void condition(lexeme *list);
void expression(lexeme *list);
void term(lexeme *list);
void factor(lexeme *list);
void logic(lexeme *list);

instruction *parse(lexeme *list, int printTable, int printCode)
{
	code = malloc(sizeof(instruction) * MAX_CODE_LENGTH);
	table = malloc(sizeof(symbol) * MAX_SYMBOL_COUNT);
	// Initialize all variables and begin program execution in parse
	tIndex = 0;
	cIndex = 0;
	currTok = 0;
	regHeight = 0;
	emit(7, 0, 0);
	addToSymbolTable(3,"main", 0, 0, 0, 0);
	level = -1;
	block(list);
	
	if(errorFlag == 1)
	{
		return NULL;
	}
	
	if(list[currTok].type != periodsym)
	{
		printparseerror(1);
		return NULL;
	}
	emit(9, 0, 3);
	// Fix all addresses of  CAL functions
	for(int i = 0; i< cIndex; i++)
	{
		if(code[i].opcode == 5)
			code[i].m = table[code[i].m].addr;
	}
	// Fix address of main
	code[0].m = table[0].addr;
	
	if (printTable)
		printsymboltable();
	if (printCode)
		printassemblycode();
	
	code[cIndex].opcode = -1;
	return code;
}

void block(lexeme *list)
{
	int procIndex = tIndex - 1;
	int x;
	level++;
	// Have errorFlag checks after each function call to be able to recursively escape execution!
	constDec(list);
	if(errorFlag == 1)
	{
		return;
	}
	
	x = varDec(list);
	if(errorFlag == 1)
	{
		return;
	}
	
	procDec(list);
	if(errorFlag == 1)
	{
		return;
	}
	
	table[procIndex].addr = cIndex;
	
	// INCREMENT instruction done here
	emit(6, 0, x + 3);
	statement(list);
	if(errorFlag == 1)
	{
		return;
	}
	
	mark();
	level--;
}

void constDec(lexeme *list)
{
	int isConst, isVar, isProc;
	int value;
	int nameIndex;
	if(list[currTok].type == constsym)
	{
		do
		{
			currTok++;
		// Is ident?
			if(list[currTok].type == identsym)
			{
				// Check if identifier used on same level, identifiers with same name can be declared on other levels
				if(multipledeclarationcheck(list[currTok].name) != -1)
				{
					printparseerror(18);
					errorFlag = 1;
					return;
				}

				nameIndex = currTok;
				currTok++;
				// Is :=?
				if(list[currTok].type == assignsym)
				{
					currTok++;
					// Is it a number?
					if(list[currTok].type == numbersym)
					{
						// Set value to number to add to sym table
						value = list[currTok].value;
						addToSymbolTable(1 , list[nameIndex].name, value, level, 0, 0);
						currTok++;
					}
					// Keep checing to ensure grammar is correct, if not then print parse error 2
					else
					{
					printparseerror(2);
					errorFlag = 1;
					return;
					}
				}
				else
				{
					printparseerror(2);
					errorFlag = 1;
					return;
				}
			}
			else
			{
				printparseerror(2);
				errorFlag = 1;
				return;
			}
		}while(list[currTok].type == commasym);
		// Once out of comma symbols, check if last identifier ends with semi colon. If not print error 13 if identifier is found or 14 otherwise
		if(list[currTok].type == semicolonsym)
		{
			currTok++;
		}
		else if(list[currTok].type == identsym)
		{
			printparseerror(13);
			errorFlag = 1;
			return;
		}
		else
		{
			printparseerror(14);
			errorFlag = 1;
			return;
		}
	}
	return;
}

int varDec(lexeme *list)
{
	int isConst, isVar, isProc;
	int numVars = 0;
	// As with const declaration, check grammar if declarations and print parse error 2 if incorrect. If correct add to symbol table
	// Check if multiple declarations done on the same level
	// Print error 13 if identifier found instead of semicolon, otherwise 14
	if(list[currTok].type == varsym)
	{
		do
		{
			currTok++;
			if(list[currTok].type == identsym)
			{
				if(multipledeclarationcheck(list[currTok].name) != -1)
				{
					printparseerror(18);
					errorFlag = 1;
					return 0;
				}
				addToSymbolTable(2, list[currTok].name, 0, level, numVars + 3, 0);
				numVars++;
				currTok++;
			}
			
			else
			{
				printparseerror(3);
				errorFlag = 1;
				return 0;
			}
		} while (list[currTok].type == commasym);
		
		if(list[currTok].type == semicolonsym)
		{
			currTok++;
		}

		else if(list[currTok].type == identsym)
		{
			printparseerror(13);
			errorFlag = 1;
			return 0;
		}
		
		else
		{
			printparseerror(14);
			errorFlag = 1;
			return 0;
		}
		
	}
	return numVars;
}

void procDec(lexeme *list)
{
	// Same as var declaration and const declaration structure
	int isConst, isVar, isProc;
	while(list[currTok].type == procsym)
	{
		currTok++;
		if(list[currTok].type == identsym)
		{
			if(multipledeclarationcheck(list[currTok].name) != -1)
				{
					printparseerror(18);
					errorFlag = 1;
					return;
				}
			addToSymbolTable(3, list[currTok].name, 0, level, 0, 0);
			currTok++;
			if(list[currTok].type == semicolonsym)
			{
				currTok++;
				block(list);
				if(errorFlag == 1)
				{
					return;
				}
				if(list[currTok].type == semicolonsym)
				{
					currTok++;
					emit(2, 0, 0);
				}
				else
				{
					printparseerror(14);
					errorFlag = 1;
					return;
				}
			}
			else
			{
				printparseerror(4);
				errorFlag = 1;
				return;
			}
		}
		else
		{
			printparseerror(4);
			errorFlag = 1;
			return;
		}
	}
	return;
}

void statement(lexeme *list)
{
	int valid;
	int jpcIdx;
	int jmpIdx;
	
	// ASSIGNMENT
	if(list[currTok].type == identsym)
	{
		// Check to see if we are assigning to a an existing variables only
		valid = findsymbol(list[currTok].name, 2);
		if(valid == -1)
		{
			if(findsymbol(list[currTok].name, 1) != -1 || findsymbol(list[currTok].name, 3) != -1)
			{
				printparseerror(6);
				errorFlag = 1;
				return; 
			}
			else
			{
				printparseerror(19);
				errorFlag = 1;
				return;
			}
		}
		currTok++;
		// Continue assignment if grammar is correct
		if(list[currTok].type == assignsym)
		{
			currTok++;
			expression(list);
			if(errorFlag == 1)
			{
				return;
			}
			// STORE! REG--!
			emit(4, level - table[valid].level, table[valid].addr);
			regHeight--;
		}
		else
		{
			printparseerror(5);
			errorFlag = 1;
			return;
		}
	}

	// BEGIN
	// We check for grammar, ensuring begin is followed by a statement(s) and finally end, otherwise print errors
	else if(list[currTok].type == beginsym)
	{
		do
		{
			currTok++;
			statement(list);
			if(errorFlag == 1)
			{
				return;
			}
		} while (list[currTok].type == semicolonsym);
		if(list[currTok].type == endsym)
			currTok++;
		else if(list[currTok].type == identsym || list[currTok].type == readsym || list[currTok].type == writesym  || list[currTok].type == beginsym || list[currTok].type == callsym || list[currTok].type == ifsym || list[currTok].type == whilesym)
		{
			printparseerror(15);
			errorFlag = 1;
			return; 
		}
		else
		{
			printparseerror(16);
			errorFlag = 1;
			return;
		}
	}

	// IF
	// Check to see if grammar is correct with if followed by then and then else, setting the corresponding jump indexes
	else if(list[currTok].type == ifsym)
	{
		currTok++;
		logic(list);
		if(errorFlag == 1)
		{
			return;
		}
		jpcIdx = cIndex;
		emit(8, 0, 0);
		if(list[currTok].type == thensym)
		{
			currTok++;
			statement(list);
			if(errorFlag == 1)
			{
				return;
			}
			if(list[currTok].type == elsesym)
			{
				currTok++;
				jmpIdx = cIndex;
				emit(7, 0, 0);
				code[jpcIdx].m = cIndex;
				statement(list);
				if(errorFlag == 1)
				{
					return;
				}
				code[jmpIdx].m = cIndex;

			}
			else
				code[jpcIdx].m = cIndex;
		}
		else
		{
			printparseerror(8);
			errorFlag = 1;
			return;
		}
	}
	
	// WHILE
	// Check grammar of while loop ensuring it has a condition and set corresponding jump indexes
	else if(list[currTok].type == whilesym)
	{
		currTok++;
		int loopIdx = cIndex;
		logic(list);
		if(errorFlag == 1)
		{
			return;
		}
		jpcIdx = cIndex;
		emit(8, 0, 0);
		if(list[currTok].type == dosym)
		{
			currTok++;
			statement(list);
			if(errorFlag == 1)
			{
				return;
			}
			emit(7, 0, loopIdx);
			code[jpcIdx].m = cIndex;
		}
		else
		{
			printparseerror(9);
			errorFlag = 1;
			return;
		}
	}

	// READ
	// Ensure only an existing variable is read, then emit the read instruction and store instruction
	else if(list[currTok].type == readsym)
	{
		currTok++;
		valid = findsymbol(list[currTok].name, 2);
		if(valid == -1)
		{
			printparseerror(6);
			errorFlag = 1;
			return;
		}
		// READ! REG++!
		emit(9, 0, 2);
		
		if(regHeight == 10)
		{
			printparseerror(20);
			errorFlag = 1;
			return;
		}
		regHeight++;
		
		//STORE! REG--!
		emit(4, level - table[valid].level, table[valid].addr);
		regHeight--;
		currTok++;

	}

	// WRITE
	// Emit write instruction
	else if(list[currTok].type == writesym)
	{
		currTok++;
		expression(list);
		if(errorFlag == 1)
		{
			return;
		}
		emit(9, 0, 1);
	}

	//call?
	// Ensure call instruction has correct grammar, such as using a valid procedure identifier
	else if(list[currTok].type == callsym)
	{
		currTok++;
        if(list[currTok].type == identsym)
        {
			valid = findsymbol(list[currTok].name, 3);
			if(valid == -1)
        	{
				if(findsymbol(list[currTok].name, 2) != -1)
				{
					printparseerror(7);
					errorFlag = 1;
					return;
				}
				else if(findsymbol(list[currTok].name, 1) != -1)
				{
					printparseerror(7);
					errorFlag = 1;
					return;
				}
				else
				{
					printparseerror(19);
					errorFlag = 1;
					return;
				}
        	}
        	emit(5, level - table[valid].level, valid);
			currTok++;
        }
        else
        {
            printparseerror(7);
            errorFlag = 1;
            return;
        }
    }
	return;
}

// Emit instructions for corresponding logic operators
void logic(lexeme *list)
{
	if(list[currTok].type == notsym)
	{
		currTok++;
		condition(list);
		if(errorFlag == 1)
		{
			return;
		}
		emit(2, 0, 14);
	}
	else
	{
		condition(list);
		if(errorFlag == 1)
		{
			return;
		}
		while(list[currTok].type == andsym || list[currTok].type == orsym)
		{
			if(list[currTok].type == andsym)
			{
				currTok++;
				condition(list);
				if(errorFlag == 1)
				{	
					return;
				}
				emit(2, 0, 12);
				regHeight--;
			}
			else
			{
				currTok++;
				condition(list);
				if(errorFlag == 1)
				{
					return;
				}
				emit(2, 0, 13);
				regHeight--;
			}
		}
	}
}

// Emit instructions for corresponding symbol and reading in expressions
void condition(lexeme *list)
{
	if(list[currTok].type == lparensym)
	{
		currTok++;
		logic(list);
		if(errorFlag == 1)
			return;
		if(list[currTok].type == rparensym)
		{
			currTok++;
		}
		else
		{
			printparseerror(12);
			errorFlag = 1;
			return;
		}
	}
	else
	{
		expression(list);
		if(errorFlag == 1)
				return;
		// ==
		if(list[currTok].type == eqlsym)
		{
			currTok++;
			expression(list);
			if(errorFlag == 1)
				return;
			// EQL! REG--!
			emit(2, 0, 6);
			regHeight--;
		}
		// !=
		else if(list[currTok].type == neqsym)
		{
			currTok++;
			expression(list);
			if(errorFlag == 1)
				return;
			// NEQ! REG--!
			emit(2, 0, 7);
			regHeight--;
		}
		// <
		else if(list[currTok].type == lsssym)
		{
			currTok++;
			expression(list);
			if(errorFlag == 1)
				return;
			// LSS! REG--
			emit(2, 0, 8);
			regHeight--;
		}
		// <=
		else if(list[currTok].type == leqsym)
		{
			currTok++;
			expression(list);
			if(errorFlag == 1)
				return;
			// LEQ! REG--!
			emit(2, 0, 9);
			regHeight--;
		}
		// >
		else if(list[currTok].type == gtrsym)
		{
			currTok++;
			expression(list);
			if(errorFlag == 1)
				return;
			// GTR! REG--!
			emit(2, 0, 10);
			regHeight--;
		}
		// >=
		else if(list[currTok].type == geqsym)
		{
			currTok++;
			expression(list);
			if(errorFlag == 1)
				return;
			// GEQ! REG--!
			emit(2, 0, 11);
			regHeight--;
		}
		else
		{
			printparseerror(10);
			errorFlag = 1;
			return;
		}
	}
	return;
}
// Check to see grammar of expression is correct and emit correct instructions such as add or subtract
void expression(lexeme *list)
{
	if(list[currTok].type == minussym)
	{
		currTok++;
		term(list);
		if(errorFlag == 1)
			return;
		emit(2, 0, 1);
		while(list[currTok].type == plussym || list[currTok].type == minussym)
		{
			if(list[currTok].type == plussym)
			{
				currTok++;
				term(list);
				if(errorFlag == 1)
					return;
				// ADD! REG--!
				emit(2, 0, 2);
				regHeight--;
			}
			else
			{
				currTok++;
				term(list);
				if(errorFlag == 1)
					return;
				// SUB! REG--!
				emit(2, 0, 3);
				regHeight--;
			}
		}
	}
	else
	{
		if(list[currTok].type == plussym)
			currTok++;
		term(list);
		if(errorFlag == 1)
			return;
		while(list[currTok].type == plussym || list[currTok].type == minussym)
		{
			if(list[currTok].type == plussym)
			{
				currTok++;
				term(list);
				if(errorFlag == 1)
					return;
				// ADD! REG--!
				emit(2, 0, 2);
				regHeight--;
			}
			else
			{
				currTok++;
				term(list);
				if(errorFlag == 1)
					return;
				// SUB! REG--!
				emit(2, 0, 3);
				regHeight--;
			}
		}
	}
	return;
}
// Check for factors and emit multipication or division instructions
void term(lexeme *list)
{
	factor(list);
	if(errorFlag == 1)
			return;
	if(list[currTok].type == multsym || list[currTok].type == divsym)
	{
		if(list[currTok].type == multsym)
		{
			currTok++;
			factor(list);
			if(errorFlag == 1)
				return;
			// MULT! REG--!
			emit(2, 0, 4);
			regHeight--;
		}
		else
		{
			currTok++;
			factor(list);
			if(errorFlag == 1)
				return;
			// DIV! REG--!
			emit(2, 0, 5);
			regHeight--;
		}
	}
	return;
}
// Emit the rest of arithmetic instructions and check grammar
void factor(lexeme *list)
{
	
	int isVar;
	int isConst;
	if(list[currTok].type == identsym)
	{
		isVar = findsymbol(list[currTok].name, 2);
		isConst = findsymbol(list[currTok].name, 1);
		if(isConst == -1 && isVar == -1 && findsymbol(list[currTok].name, 3) != -1)
		{
			printparseerror(11);
			errorFlag = 1;
			return;
		}
		else if(isConst == -1 && isVar == -1)
		{
			printparseerror(19);
			errorFlag = 1;
			return;
		}
		else if(isConst == -1 || table[isVar].level > table[isConst].level)
		{
			// LOAD! REG++!
			emit(3, level - table[isVar].level, table[isVar].addr);					
			if(regHeight == 10)
			{
				printparseerror(20);
				errorFlag = 1;
				return;
			}
			regHeight++;
		}
		else
		{
			// LIT! REG++!
			emit(1, 0, table[isConst].val);		
			if(regHeight == 10)
			{
				printparseerror(20);
				errorFlag = 1;
				return;
			}
			regHeight++;
		}
		currTok++;
	}
	else if(list[currTok].type == numbersym)
	{
		// LIT! REG++!
		emit(1, 0, list[currTok].value);
		currTok++;
		if(regHeight == 10)
		{
			printparseerror(20);
			errorFlag = 1;
			return;
		}
		regHeight++;
	}
	else if(list[currTok].type == lparensym)
	{
		currTok++;
		expression(list);
		if(errorFlag == 1)
			return;
		if(list[currTok].type == rparensym)
		{
			currTok++;
		}
		else
		{
			printparseerror(12);
			errorFlag = 1;
			return;
		}
	}
	else
	{
		printparseerror(11);
		errorFlag = 1;
		return;
	}
	return;
}
// adds a line of code to the program
void emit(int opname, int level, int mvalue)
{
	code[cIndex].opcode = opname;
	code[cIndex].l = level;
	code[cIndex].m = mvalue;
	cIndex++;
}

// add a symbol to the symbol table
void addToSymbolTable(int k, char n[], int v, int l, int a, int m)
{
	table[tIndex].kind = k;
	strcpy(table[tIndex].name, n);
	table[tIndex].val = v;
	table[tIndex].level = l;
	table[tIndex].addr = a;
	table[tIndex].mark = m;
	tIndex++;
}

// mark the symbols belonging to the current procedure, should be called at the end of block
void mark()
{
	int i;
	for (i = tIndex - 1; i >= 0; i--)
	{
		if (table[i].mark == 1)
			continue;
		if (table[i].level < level)
			return;
		table[i].mark = 1;
	}
}

// checks if a new symbol has a valid name, by checking if there's an existing symbol
// with the same name in the procedure
int multipledeclarationcheck(char name[])
{
	int i;
	for (i = 0; i < tIndex; i++)
		if (table[i].mark == 0 && table[i].level == level && strcmp(name, table[i].name) == 0)
			return i;
	return -1;
}

// returns the index of a symbol with a given name and kind in the symbol table
// returns -1 if not found
// prioritizes lower lex levels
int findsymbol(char name[], int kind)
{
	int i;
	int max_idx = -1;
	int max_lvl = -1;
	for (i = 0; i < tIndex; i++)
	{
		if (table[i].mark == 0 && table[i].kind == kind && strcmp(name, table[i].name) == 0)
		{
			if (max_idx == -1 || table[i].level > max_lvl)
			{
				max_idx = i;
				max_lvl = table[i].level;
			}
		}
	}
	return max_idx;
}

void printparseerror(int err_code)
{
	switch (err_code)
	{
		case 1:
			printf("Parser Error: Program must be closed by a period\n");
			break;
		case 2:
			printf("Parser Error: Constant declarations should follow the pattern 'ident := number {, ident := number}'\n");
			break;
		case 3:
			printf("Parser Error: Variable declarations should follow the pattern 'ident {, ident}'\n");
			break;
		case 4:
			printf("Parser Error: Procedure declarations should follow the pattern 'ident ;'\n");
			break;
		case 5:
			printf("Parser Error: Variables must be assigned using :=\n");
			break;
		case 6:
			printf("Parser Error: Only variables may be assigned to or read\n");
			break;
		case 7:
			printf("Parser Error: call must be followed by a procedure identifier\n");
			break;
		case 8:
			printf("Parser Error: if must be followed by then\n");
			break;
		case 9:
			printf("Parser Error: while must be followed by do\n");
			break;
		case 10:
			printf("Parser Error: Relational operator missing from condition\n");
			break;
		case 11:
			printf("Parser Error: Arithmetic expressions may only contain arithmetic operators, numbers, parentheses, constants, and variables\n");
			break;
		case 12:
			printf("Parser Error: ( must be followed by )\n");
			break;
		case 13:
			printf("Parser Error: Multiple symbols in variable and constant declarations must be separated by commas\n");
			break;
		case 14:
			printf("Parser Error: Symbol declarations should close with a semicolon\n");
			break;
		case 15:
			printf("Parser Error: Statements within begin-end must be separated by a semicolon\n");
			break;
		case 16:
			printf("Parser Error: begin must be followed by end\n");
			break;
		case 17:
			printf("Parser Error: Bad arithmetic\n");
			break;
		case 18:
			printf("Parser Error: Confliciting symbol declarations\n");
			break;
		case 19:
			printf("Parser Error: Undeclared identifier\n");
			break;
		case 20:
			printf("Parser Error: Register Overflow Error\n");
			break;
		default:
			printf("Implementation Error: unrecognized error code\n");
			break;
	}
	
	free(code);
	free(table);
}

void printsymboltable()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i < tIndex; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr, table[i].mark); 
	
	free(table);
	table = NULL;
}

void printassemblycode()
{
	int i;
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < cIndex; i++)
	{
		printf("%d\t", i);
		printf("%d\t", code[i].opcode);
		switch (code[i].opcode)
		{
			case 1:
				printf("LIT\t");
				break;
			case 2:
				switch (code[i].m)
				{
					case 0:
						printf("RET\t");
						break;
					case 1:
						printf("NEG\t");
						break;
					case 2:
						printf("ADD\t");
						break;
					case 3:
						printf("SUB\t");
						break;
					case 4:
						printf("MUL\t");
						break;
					case 5:
						printf("DIV\t");
						break;
					case 6:
						printf("EQL\t");
						break;
					case 7:
						printf("NEQ\t");
						break;
					case 8:
						printf("LSS\t");
						break;
					case 9:
						printf("LEQ\t");
						break;
					case 10:
						printf("GTR\t");
						break;
					case 11:
						printf("GEQ\t");
						break;
					case 12:
						printf("AND\t");
						break;
					case 13:
						printf("ORR\t");
						break;
					case 14:	
						printf("NOT\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			case 3:
				printf("LOD\t");
				break;
			case 4:
				printf("STO\t");
				break;
			case 5:
				printf("CAL\t");
				break;
			case 6:
				printf("INC\t");
				break;
			case 7:
				printf("JMP\t");
				break;
			case 8:
				printf("JPC\t");
				break;
			case 9:
				switch (code[i].m)
				{
					case 1:
						printf("WRT\t");
						break;
					case 2:
						printf("RED\t");
						break;
					case 3:
						printf("HAL\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			default:
				printf("err\t");
				break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
	}
	if (table != NULL)
		free(table);
}