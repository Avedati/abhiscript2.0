#include "../include/function.h"
#include "../include/lexer.h"
#include <stdlib.h>
#include <string.h>

struct Function* create_Function(char* name, char** args, int n_Args, struct Token** tokens, int n_Tokens) {
	struct Function* function = malloc(sizeof(struct Function));
	function->name = malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(function->name, name);
	function->args = malloc(sizeof(char*) * (n_Args));
	function->n_Args = n_Args;
	for(int i=0;i<n_Args;i++) {
		function->args[i] = malloc(sizeof(char) * (strlen(args[i]) + 1));
		strcpy(function->args[i], args[i]);
	}
	function->tokens = malloc(sizeof(struct Token*) * (n_Tokens + 1));
	for(int i=0;i<n_Tokens;i++) {
		function->tokens[i] = create_Token(tokens[i]->type, tokens[i]->number_Value, tokens[i]->function_Name);
	}
	function->n_Tokens = n_Tokens;
	return function;
}

void destroy_Function(struct Function* function) {
	free(function->name);
	for(int i=0;i<function->n_Args;i++) {
		free(function->args[i]);
	}
	free(function->args);
	for(int i=0;i<function->n_Tokens;i++) {
		destroy_Token(function->tokens[i]);
	}
	free(function);
}
