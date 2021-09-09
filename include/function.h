#include "lexer.h"
#ifndef __FUNCTION_H_
#define __FUNCTION_H_ 0

struct Function {
	char* name;
	char** args;
	int n_Args;
	struct Token** tokens;
	int n_Tokens;
};

struct Function* create_Function(char*, char**, int, struct Token**, int);
void destroy_Function(struct Function*);

#endif // __FUNCTION_H_
