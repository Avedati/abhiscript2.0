#include "../include/function.h"
#include "../include/lexer.h"
#include "../include/loop.h"
#include "../include/parser.h"
#include "../include/scope.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
	FILE* input;
	if(argc >= 2) {
		input = fopen(argv[1], "rb");
	}
	else {
		input = stdin;
	}
	int n_Tokens = 0;
	char* line;
	size_t length;
	struct Scope* scope = create_Scope();
	while(1) {
		if(input == stdin) {
			printf("$ ");
		}
		if(getline(&line, &length, input) == -1) {
			printf("\n");
			break;
		}
		else if(strncmp(line, "quit", 4) == 0 || strncmp(line, "exit", 4) == 0) {
			break;
		}
		struct Token** tokens = tokenize(line, &n_Tokens);
		struct Parser* parser = create_Parser(tokens, n_Tokens);
		parser->scope = scope;
		double value = parse_Unit(parser);
		if(input == stdin) {
			printf("-> %.3f\n", value);
		}
		destroy_Parser(parser);
	}
	destroy_Scope(scope);
	return 0;
}
