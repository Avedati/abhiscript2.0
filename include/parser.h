#ifndef __PARSER_H_
#define __PARSER_H_ 0

#include "lexer.h"
#include "loop.h"
#include "scope.h"

struct Parser {
	struct Scope* scope;
	struct Token** tokens;
	struct Loop** loop_Stack;
	int n_Tokens;
	int index;
	int loop_Depth;
};

struct Parser* create_Parser(struct Token**, int);
void destroy_Parser(struct Parser*);
double parse_Unit(struct Parser*);

#endif // __PARSER_H_
