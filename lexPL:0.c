// Audiel Ortiz
// COP3402 Spring2022
// HW4 (lex.c)
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"

#define MAX_NUMBER_TOKENS 1000
#define MAX_IDENT_LEN 11
#define MAX_NUMBER_LEN 5

lexeme *list;
int lex_index;

int getValue(char* buffer);
int reservedToken(char* buffer);
void printlexerror(int type);
void printtokens();

// Gets value of digit
int getValue(char * buffer)
{
	int value = atoi(buffer);
	return value;
}

// Checks to see if reserved word
// If so then assign the type, else return 1 to indicate identifier
int reservedToken(char * buffer)
{

	if(strcmp(buffer, "const") == 0)
	{
		list[lex_index].type = constsym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "var") == 0)
	{
		list[lex_index].type = varsym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "procedure") == 0)
	{
		list[lex_index].type = procsym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "call") == 0)
	{
		list[lex_index].type = callsym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "if") == 0)
	{
		list[lex_index].type = ifsym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "then") == 0)
	{
		list[lex_index].type = thensym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "else") == 0)
	{
		list[lex_index].type = elsesym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "while") == 0)
	{
		list[lex_index].type = whilesym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "do") == 0)
	{
		list[lex_index].type = dosym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "begin") == 0)
	{
		list[lex_index].type = beginsym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "end") == 0)
	{
		list[lex_index].type = endsym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "read") == 0)
	{
		list[lex_index].type = readsym;
		lex_index++;
		return 0;
	}

	if(strcmp(buffer, "write") == 0)
	{
		list[lex_index].type = writesym;
		lex_index++;
		return 0;
	}
	else
		return 1;
}

lexeme *lexanalyzer(char *input, int printFlag)
{
	list = malloc(sizeof(lexeme) * MAX_NUMBER_TOKENS);
	lex_index = 0;
	int number = 0;
	int errortype = 0;
	int currChar = 0;
	int id = 0;
	int neverEnd = 1;
	int dig = 0;
	int sym = 0;
	char* symbol = (char*)malloc(3 * sizeof(char));
	char* ident = (char*)malloc(12 * sizeof(char));
	char* digit = (char*)malloc(6 * sizeof(char));

	// Begin tokenization
	while(currChar < strlen(input))
	{
		int len = 0;
		int notRes = 1;
		int i = 0;

		// Clear whitepsace and control characters then restart loop after clearing
		if(input[currChar] == ' ' || iscntrl(input[currChar]))
		{
			while(input[currChar] == ' ' || iscntrl(input[currChar]))
				currChar++;
			continue;
		}

		// Checks if identifier or reserved word!
		if(isalpha(input[currChar]))
		{

			while(i <= 11)
			{
				if(i == 11 && (isalpha(input[currChar]) || isdigit(input[currChar])))
				{
					printlexerror(3);
					return NULL;
				}

				if(isalpha(input[currChar]) || isdigit(input[currChar]))
				{
					strncat(ident, &input[currChar], 1);
					currChar++;
					i++;
				}

				// If its a symbol or whitespace stop reading and tokenize!
				else
					break;

			}

			// Check if valid string, if so then tokenize and clear buffer
			len = strlen(ident);
			if(len != 0)
			{
				notRes = reservedToken(ident);
				if(notRes == 1)
				{

						list[lex_index].type = identsym;
						strcpy(list[lex_index].name, ident);
						lex_index++;
						memset(ident, 0, len);

				}
				else
				{
					memset(ident, 0, len);
				}
			}

			continue;
		}

		// Check for digits!
		else if(isdigit(input[currChar]))
		{
			while(i <= 5)
			{

				if(isdigit(input[currChar]))
				{
					// Number Length too long
					if(i == 5 && (isdigit(input[currChar])))
					{
						printlexerror(2);
						return NULL;
					}

					strncat(digit, &input[currChar], 1);
					currChar++;
					i++;
				}

				// Invalid Identifier with number starting identifier
				else if(isalpha(input[currChar]))
				{
					printlexerror(1);
					return NULL;
				}

				// Either whitespace or symbol, so break and continue to next token if no digits inputted
				else
				{
					break;
				}
			}

			len = strlen(digit);

			//Put in digit value and token
			if(len != 0)
			{
				list[lex_index].type = numbersym;
				int digitVal = getValue(digit);
				list[lex_index].value = digitVal;
				lex_index++;
				memset(digit, 0, len);
			}

			continue;
		}

		// Check for comments and symbols!
		else if(!isalpha(input[currChar]) || !isdigit(input[currChar]) || !iscntrl(input[currChar]) || input[currChar] != ' ')
		{

			// Quickly go through a comment
			if(input[currChar] == '/' && input[currChar + 1] == '*')
			{
				currChar += 2;
				neverEnd = 1;
				while(currChar < strlen(input) - 1)
				{
					if(input[currChar] == '*' && input[currChar + 1] == '/')
					{
						neverEnd = 0;
						currChar += 2;
						break;
					}
					currChar++;
				}

				if(currChar >= strlen(input) - 1 && neverEnd == 1)
				{
					printlexerror(5);
					return NULL;
				}
			}

			// Check for symbols if invalid then print error!
			else
			{
				// Checks for '=='
				if(input[currChar] == '=' && input[currChar + 1] == '=')
				{
					currChar += 2;
					list[lex_index].type = eqlsym;
					lex_index++;
				}

				// Checks for '!=' or '!'
				else if(input[currChar] == '!')
				{
					if(input[currChar + 1] == '=')
					{
						currChar += 2;
						list[lex_index].type = neqsym;
						lex_index++;
					}
					else
					{
						currChar++;
						list[lex_index].type = notsym;
						lex_index++;
					}
				}

				// Checks for '<' or '<='
				else if(input[currChar] == '<')
				{
					if(input[currChar + 1] == '=')
					{
						currChar += 2;
						list[lex_index].type = leqsym;
						lex_index++;
					}
					else
					{
						currChar++;
						list[lex_index].type = lsssym;
						lex_index++;
					}
				}

				// Checks for '>' or '>='
				else if(input[currChar] == '>')
				{
					if(input[currChar + 1] == '=')
					{
						currChar += 2;
						list[lex_index].type = geqsym;
						lex_index++;
					}
					else
					{
						currChar++;
						list[lex_index].type = gtrsym;
						lex_index++;
					}
				}

				// Checks for '*'
				else if(input[currChar] == '*')
				{
					currChar++;
					list[lex_index].type = multsym;
					lex_index++;
				}

				// Checks for '/'
				else if(input[currChar] == '/')
				{
					currChar++;
					list[lex_index].type = divsym;
					lex_index++;
				}

				// Checks for '+'
				else if(input[currChar] == '+')
				{
					currChar++;
					list[lex_index].type = plussym;
					lex_index++;
				}

				// Checks for '-'
				else if(input[currChar] == '-')
				{
					currChar++;
					list[lex_index].type = minussym;
					lex_index++;
				}

				// Checks for '('
				else if(input[currChar] == '(')
				{
					currChar++;
					list[lex_index].type = lparensym;
					lex_index++;
				}

				// Checks for ')'
				else if(input[currChar] == ')')
				{
					currChar++;
					list[lex_index].type = rparensym;
					lex_index++;
				}

				// Checks for ','
				else if(input[currChar] == ',')
				{
					currChar++;
					list[lex_index].type = commasym;
					lex_index++;
				}

				// Checks for '.'
				else if(input[currChar] == '.')
				{
					currChar++;
					list[lex_index].type = periodsym;
					lex_index++;
				}

				// Checks for ';'
				else if(input[currChar] == ';')
				{
					currChar++;
					list[lex_index].type = semicolonsym;
					lex_index++;
				}

				// Checks for '&&'
				else if(input[currChar] == '&' && input[currChar + 1] == '&')
				{
					currChar += 2;
					list[lex_index].type = andsym;
					lex_index++;
				}

				//Checks for '||'
				else if(input[currChar] == '|' && input[currChar + 1] == '|')
				{
					currChar += 2;
					list[lex_index].type = orsym;
					lex_index++;
				}

				// Checks for ':='
				else if(input[currChar] == ':' && input[currChar + 1] == '=')
				{
					currChar += 2;
					list[lex_index].type = assignsym;
					lex_index++;
				}
				else
				{
					printlexerror(4);
					return NULL;
				}
			}
		}



	}
	//Print tokens!
	if (printFlag)
		printtokens();

	// these last two lines are really important for the rest of the package to run
	list[lex_index].type = -1;
	return list;
}





void printtokens()
{
	int i;
	printf("Lexeme Table:\n");
	printf("lexeme\t\ttoken type\n");
	for (i = 0; i < lex_index; i++)
	{
		switch (list[i].type)
		{
			case eqlsym:
				printf("%11s\t%d", "==", eqlsym);
				break;
			case neqsym:
				printf("%11s\t%d", "!=", neqsym);
				break;
			case lsssym:
				printf("%11s\t%d", "<", lsssym);
				break;
			case leqsym:
				printf("%11s\t%d", "<=", leqsym);
				break;
			case gtrsym:
				printf("%11s\t%d", ">", gtrsym);
				break;
			case geqsym:
				printf("%11s\t%d", ">=", geqsym);
				break;
			case multsym:
				printf("%11s\t%d", "*", multsym);
				break;
			case divsym:
				printf("%11s\t%d", "/", divsym);
				break;
			case plussym:
				printf("%11s\t%d", "+", plussym);
				break;
			case minussym:
				printf("%11s\t%d", "-", minussym);
				break;
			case lparensym:
				printf("%11s\t%d", "(", lparensym);
				break;
			case rparensym:
				printf("%11s\t%d", ")", rparensym);
				break;
			case commasym:
				printf("%11s\t%d", ",", commasym);
				break;
			case periodsym:
				printf("%11s\t%d", ".", periodsym);
				break;
			case semicolonsym:
				printf("%11s\t%d", ";", semicolonsym);
				break;
			case assignsym:
				printf("%11s\t%d", ":=", assignsym);
				break;
			case beginsym:
				printf("%11s\t%d", "begin", beginsym);
				break;
			case endsym:
				printf("%11s\t%d", "end", endsym);
				break;
			case ifsym:
				printf("%11s\t%d", "if", ifsym);
				break;
			case thensym:
				printf("%11s\t%d", "then", thensym);
				break;
			case elsesym:
				printf("%11s\t%d", "else", elsesym);
				break;
			case whilesym:
				printf("%11s\t%d", "while", whilesym);
				break;
			case dosym:
				printf("%11s\t%d", "do", dosym);
				break;
			case callsym:
				printf("%11s\t%d", "call", callsym);
				break;
			case writesym:
				printf("%11s\t%d", "write", writesym);
				break;
			case readsym:
				printf("%11s\t%d", "read", readsym);
				break;
			case constsym:
				printf("%11s\t%d", "const", constsym);
				break;
			case varsym:
				printf("%11s\t%d", "var", varsym);
				break;
			case procsym:
				printf("%11s\t%d", "procedure", procsym);
				break;
			case identsym:
				printf("%11s\t%d", list[i].name, identsym);
				break;
			case numbersym:
				printf("%11d\t%d", list[i].value, numbersym);
				break;
			case andsym:
				printf("%11s\t%d", "&&", andsym);
				break;
			case orsym:
				printf("%11s\t%d", "||", orsym);
				break;
			case notsym:
				printf("%11s\t%d", "!", notsym);
				break;
		}
		printf("\n");
	}
	printf("\n");
	printf("Token List:\n");
	for (i = 0; i < lex_index; i++)
	{
		if (list[i].type == numbersym)
			printf("%d %d ", numbersym, list[i].value);
		else if (list[i].type == identsym)
			printf("%d %s ", identsym, list[i].name);
		else
			printf("%d ", list[i].type);
	}
	printf("\n");
}

void printlexerror(int type)
{
	if (type == 1)
		printf("Lexical Analyzer Error: Invalid Identifier\n");
	else if (type == 2)
		printf("Lexical Analyzer Error: Number Length\n");
	else if (type == 3)
		printf("Lexical Analyzer Error: Identifier Length\n");
	else if (type == 4)
		printf("Lexical Analyzer Error: Invalid Symbol\n");
	else if (type == 5)
		printf("Lexical Analyzer Error: Never-ending comment\n");
	else
		printf("Implementation Error: Unrecognized Error Type\n");

	free(list);
	return;
}
