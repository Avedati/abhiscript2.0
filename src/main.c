#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum TokenType {
	T_NUMBER = 0,
	T_FUNCTION
};

struct Token {
	enum TokenType type;
	double number_Value;
	char* function_Name;
};

struct Loop {
	int condition_Index;
	int code_Start;
	int code_End;
};

struct Function {
	char* name;
	char** args;
	int n_Args;
	struct Token** tokens;
	int n_Tokens;
};

struct Scope {
	int length;
	char** variable_Names;
	double* values;
	int n_Functions;
	struct Function** functions;
};

struct Parser {
	struct Scope* scope;
	struct Token** tokens;
	struct Loop** loop_Stack;
	int n_Tokens;
	int index;
	int loop_Depth;
};

struct Token* create_Token(enum TokenType type, double number_Value, char* function_Name) {
	struct Token* token = malloc(sizeof(struct Token));
	token->type = type;
	if(type == T_NUMBER) {
		token->number_Value = number_Value;
		token->function_Name = NULL;
	}
	else if(type == T_FUNCTION) {
		token->number_Value = 0;
		token->function_Name = malloc(sizeof(char) * (strlen(function_Name) + 1));
		strcpy(token->function_Name, function_Name);
	}
	return token;
}

void destroy_Token(struct Token* token) {
	if(token->type == T_FUNCTION) {
		free(token->function_Name);
	}
	free(token);
}

struct Loop* create_Loop(int condition_Index, int code_Start, int code_End) {
	struct Loop* loop = malloc(sizeof(struct Loop));
	loop->condition_Index = condition_Index;
	loop->code_Start = code_Start;
	loop->code_End = code_End;
	return loop;
}

void destroy_Loop(struct Loop* loop) {
	free(loop);
}

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

struct Scope* create_Scope(void) {
	struct Scope* scope = malloc(sizeof(struct Scope));
	scope->length = 0;
	scope->variable_Names = malloc(0);
	scope->values = malloc(0);
	scope->n_Functions = 0;
	scope->functions = malloc(0);
	return scope;
}

struct Scope* from_Scope(struct Scope* scope) {
	struct Scope* result = create_Scope();
	result->length = scope->length;
	result->n_Functions = scope->n_Functions;
	result->variable_Names = malloc(sizeof(char*) * (scope->length));
	for(int i=0;i<scope->length;i++) {
		result->variable_Names[i] = malloc(sizeof(char) * (strlen(scope->variable_Names[i]) + 1));
		strcpy(result->variable_Names[i], scope->variable_Names[i]);
	}
	result->values = malloc(sizeof(double) * (scope->length));
	for(int i=0;i<scope->length;i++) {
		result->values[i] = scope->values[i];
	}
	return result;
}

void destroy_Scope(struct Scope* scope) {
	for(int i=0;i<scope->length;i++) {
		free(scope->variable_Names[i]);
	}
	for(int i=0;i<scope->n_Functions;i++) {
		destroy_Function(scope->functions[i]);
	}
	free(scope->variable_Names);
	free(scope->values);
	free(scope);
}

void scope_Set_Value(struct Scope* scope, char* variable_Name, double value) {
	for(int i=0;i<scope->length;i++) {
		if(strcmp(scope->variable_Names[i], variable_Name) == 0) {
			scope->values[i] = value;
			return;
		}
	}
	scope->variable_Names = realloc(scope->variable_Names, sizeof(char*) * (scope->length + 1));
	scope->variable_Names[scope->length] = malloc(sizeof(char) * (strlen(variable_Name) + 1));
	strcpy(scope->variable_Names[scope->length], variable_Name);
	scope->values = realloc(scope->values, sizeof(double) * (scope->length + 1));
	scope->values[(scope->length)++] = value;
}

double scope_Get_Value(struct Scope* scope, char* variable_Name) {
	for(int i=0;i<scope->length;i++) {
		if(strcmp(scope->variable_Names[i], variable_Name) == 0) {
			return scope->values[i];
		}
	}
	fprintf(stderr, "Error {scope_Get_Value}: could not find variable `%s` in global scope.\n", variable_Name);
	exit(1);
	return -1;
}

int scope_Get_Index(struct Scope* scope, char* variable_Name) {
	for(int i=0;i<scope->length;i++) {
		if(strcmp(scope->variable_Names[i], variable_Name) == 0) {
			return i;
		}
	}
	return -1;
}

struct Parser* create_Parser(struct Token** tokens, int n_Tokens) {
	struct Parser* parser = malloc(sizeof(struct Parser));
	parser->tokens = tokens;
	parser->n_Tokens = n_Tokens;
	parser->index = 0;
	parser->loop_Stack = malloc(0);
	parser->loop_Depth = 0;
	return parser;
}

void destroy_Parser(struct Parser* parser) {
	for(int i=0;i<parser->n_Tokens;i++) {
		destroy_Token(parser->tokens[i]);
	}
	for(int i=0;i<parser->loop_Depth;i++) {
		destroy_Loop(parser->loop_Stack[i]);
	}
	free(parser->loop_Stack);
	free(parser->tokens);
	free(parser);
}

struct Token** tokenize(char* expression, int* n_Tokens) {
	struct Token** tokens = malloc(0);
	*n_Tokens = 0;
	for(int i=0;i<strlen(expression);i++) {
		if(isdigit(expression[i]) || expression[i] == '.') {
			int start = i;
			int end;
			for(end=i;end<strlen(expression) && (isdigit(expression[end]) || expression[end] == '.' || expression[end] == 'e' || expression[end] == 'E');end++);
			char* number = malloc(sizeof(char) * (end - start + 1));
			strncpy(number, &expression[start], end - start);
			number[end - start] = 0;
			// https://stackoverflow.com/questions/10075294/converting-string-to-a-double-variable-in-c
			double number_Value;
			sscanf(number, "%lf", &number_Value);
			free(number);
			tokens = realloc(tokens, sizeof(struct Token) * (*n_Tokens + 1));
			tokens[(*n_Tokens)++] = create_Token(T_NUMBER, number_Value, NULL);
			i = end - 1;
		}
		else if(expression[i] == '(' || expression[i] == ')') {
			char function[2];
			function[0] = expression[i];
			function[1] = 0;
			tokens = realloc(tokens, sizeof(struct Token) * (*n_Tokens + 1));
			tokens[(*n_Tokens)++] = create_Token(T_FUNCTION, 0, function);
		}
		else if(!isspace(expression[i])) {
			int start = i;
			int end;
			for(end=i;end<strlen(expression) && !isspace(expression[end]) && expression[end]!='(' && expression[end]!=')';end++);
			char* function = malloc(sizeof(char) * (end - start + 1));
			strncpy(function, &expression[start], end - start);
			function[end - start] = 0;
			tokens = realloc(tokens, sizeof(struct Token) * (*n_Tokens + 1));
			tokens[(*n_Tokens)++] = create_Token(T_FUNCTION, 0, function);
			free(function);
			i = end - 1;
		}
	}
	return tokens;
}

void display_Tokens(struct Token** tokens, int n_Tokens) {
	printf("Recieved %d tokens.\n", n_Tokens);
	for(int i=0;i<n_Tokens;i++) {
		if(tokens[i]->type == T_NUMBER) {
			printf("Token: (T_NUMBER, %f)\n", tokens[i]->number_Value);
		}
		else {
			printf("Token: (T_FUNCTION, %s)\n", tokens[i]->function_Name);
		}
	}
}

double parse_Unit(struct Parser* parser) {
	if(parser->n_Tokens == 0) { return 0; }
	if(parser->tokens[parser->index]->type == T_NUMBER) {
		return parser->tokens[(parser->index)++]->number_Value;
	}
	else if(parser->tokens[parser->index]->type == T_FUNCTION && parser->tokens[parser->index]->function_Name != NULL) {
		if(strcmp(parser->tokens[parser->index]->function_Name, "(") == 0) {
			parser->index++;
			double value = 0;
			while(parser->index < parser->n_Tokens &&
			      !(parser->tokens[parser->index]->type == T_FUNCTION &&
			      strcmp(parser->tokens[parser->index]->function_Name, ")") == 0)) {
				value = parse_Unit(parser);
			}
			if(parser->index < parser->n_Tokens &&
			   parser->tokens[parser->index]->type == T_FUNCTION &&
			   strcmp(parser->tokens[parser->index]->function_Name, ")") == 0) {
				parser->index++;
			}
			else if(parser->index >= parser->n_Tokens) {
				fprintf(stderr, "Error {parse_Unit}: expected `)`, program ended without resolution.\n");
			}
			else {
				fprintf(stderr, "Error {parse_Unit}: expected `)`, got token {%lf, %s}.\n", parser->tokens[parser->index]->number_Value, parser->tokens[parser->index]->function_Name);
			}
			return value;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "+") == 0) {
			parser->index++;
			double x = parse_Unit(parser);
			double y = parse_Unit(parser);
			return x + y;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "-") == 0) {
			parser->index++;
			double x = parse_Unit(parser);
			double y = parse_Unit(parser);
			return x - y;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "*") == 0) {
			parser->index++;
			double x = parse_Unit(parser);
			double y = parse_Unit(parser);
			return x * y;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "/") == 0) {
			parser->index++;
			double x = parse_Unit(parser);
			double y = parse_Unit(parser);
			if(y == 0) {
				fprintf(stderr, "Error {parse_Unit}: cannot divide by 0.\n");
				exit(1);
			}
			return x / y;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "%") == 0) {
			parser->index++;
			int x = (int)(parse_Unit(parser));
			int y = (int)(parse_Unit(parser));
			if(y == 0) {
				fprintf(stderr, "Error {parse_Unit}: cannot apply modulo operator to a dividend of 0.\n");
				exit(1);
			}
			return (double)(x % y);
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "set") == 0) {
			parser->index++;
			char* variable_Name = parser->tokens[(parser->index)++]->function_Name;
			double value = parse_Unit(parser);
			scope_Set_Value(parser->scope, variable_Name, value);
			return value;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "not") == 0) {
			parser->index++;
			double value = parse_Unit(parser);
			if(value == 0) { return 1; }
			return 0;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "or") == 0) {
			parser->index++;
			double v1 = parse_Unit(parser);
			double v2 = parse_Unit(parser);
			if(v1 == 0 && v2 == 0) { return 0; }
			return 1;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "and") == 0) {
			parser->index++;
			double v1 = parse_Unit(parser);
			double v2 = parse_Unit(parser);
			if(v1 == 0 || v2 == 0) { return 0; }
			return 1;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "if") == 0) {
			parser->index++;
			double condition = parse_Unit(parser);
			if(parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, "then") == 0) {
				parser->index++;
				double res1 = parse_Unit(parser);
				if(parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, "else") == 0) {
					parser->index++;
					double res2 = parse_Unit(parser);
					if(condition) { return res1; }
					return res2;
				}
				else {
					if(parser->tokens[parser->index]->type == T_NUMBER) {
						fprintf(stderr, "Error {parse_Unit}: expected `else`, got `%lf`.\n", parser->tokens[parser->index]->number_Value);
					}
					else if(parser->tokens[parser->index]->type == T_FUNCTION) {
						fprintf(stderr, "Error {parse_Unit}: expected `else`, got `%s`.\n", parser->tokens[parser->index]->function_Name);
					}
					exit(1);
				}
			}
			else {
				if(parser->tokens[parser->index]->type == T_NUMBER) {
					fprintf(stderr, "Error {parse_Unit}: expected `then`, got `%lf`.\n", parser->tokens[parser->index]->number_Value);
				}
				else if(parser->tokens[parser->index]->type == T_FUNCTION) {
					fprintf(stderr, "Error {parse_Unit}: expected `then`, got `%s`.\n", parser->tokens[parser->index]->function_Name);
				}
				exit(1);
			}
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "loop") == 0) {
			parser->index++;
			int condition_Index = parser->index;
			while(parser->index < parser->n_Tokens && !(parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, "begin") == 0)) {
				parser->index++;
			}
			if(parser->index < parser->n_Tokens && parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, "begin") == 0) {
				parser->index++;
				int code_Start = parser->index;
				while(parser->index < parser->n_Tokens && !(parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, "end") == 0)) {
					parser->index++;
				}
				if(parser->index < parser->n_Tokens && parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, "end") == 0) {
					int code_End = parser->index + 1;
					parser->loop_Stack = realloc(parser->loop_Stack, sizeof(struct Loop*) * (parser->loop_Depth + 1));
					parser->loop_Stack[(parser->loop_Depth)++] = create_Loop(condition_Index, code_Start, code_End);
					double result = 0;
					while(1) {
						parser->index = parser->loop_Stack[parser->loop_Depth - 1]->condition_Index;
						double condition = parse_Unit(parser);
						if(condition) {
							parser->index = parser->loop_Stack[parser->loop_Depth - 1]->code_Start;
							while(parser->index < parser->n_Tokens && !(parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, "end") == 0)) {
								result = parse_Unit(parser);
							}
							if(parser->index < parser->n_Tokens && parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, "end") == 0) {
								continue;
							}
							else {
								if(parser->index >= parser->n_Tokens) {
									fprintf(stderr, "Error {parse_Unit}: expected `end`, program ended without resolution.\n");
								}
								else {
									fprintf(stderr, "Error {parse_Unit}: expected `end`, got token {%lf, %s}.\n", parser->tokens[parser->index]->number_Value, parser->tokens[parser->index]->function_Name);
								}
								exit(1);
							}
						}
						else {
							parser->index = parser->loop_Stack[parser->loop_Depth - 1]->code_End;
							parser->loop_Depth--;
							destroy_Loop(parser->loop_Stack[parser->loop_Depth]);
							parser->loop_Stack = realloc(parser->loop_Stack, sizeof(struct Loop*) * (parser->loop_Depth));
							return result;
						}
					}
				}
				else {
					if(parser->index >= parser->n_Tokens) {
						fprintf(stderr, "Error {parse_Unit}: expected `end`, program ended without resolution.\n");
					}
					else {
						fprintf(stderr, "Error {parse_Unit}: expected `end`, got token {%lf, %s}.\n", parser->tokens[parser->index]->number_Value, parser->tokens[parser->index]->function_Name);
					}
					exit(1);
				}
			}
			else {
				if(parser->index >= parser->n_Tokens) {
					fprintf(stderr, "Error {parse_Unit}: expected `begin`, program ended without resolution.\n");
				}
				else {
					fprintf(stderr, "Error {parse_Unit}: expected `begin`, got token {%lf, %s}.\n", parser->tokens[parser->index]->number_Value, parser->tokens[parser->index]->function_Name);
				}
				exit(1);
			}
			return 0;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "print") == 0) {
			parser->index++;
			double value = parse_Unit(parser);
			printf("%.3f\n", value);
			return value;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "func") == 0) {
			parser->index++;
			if(parser->index >= parser->n_Tokens) {
				fprintf(stderr, "Error {parse_Unit}: expected function name, program ended without resolution.\n");
				exit(1);
			}
			else if(parser->tokens[parser->index]->type != T_FUNCTION) {
				fprintf(stderr, "Error {parse_Unit}: expected function name, got `%lf`.\n", parser->tokens[parser->index]->number_Value);
				exit(1);
			}
			char* function_Name = parser->tokens[(parser->index)++]->function_Name;
			if(parser->index >= parser->n_Tokens) {
				fprintf(stderr, "Error {parse_Unit}: expected `(`, program ended without resolution.\n");
				exit(1);
			}
			else if(parser->tokens[parser->index]->type != T_FUNCTION || strcmp(parser->tokens[parser->index]->function_Name, "(") != 0) {
				fprintf(stderr, "Error {parse_Unit}: expected `(`, got token {%lf, %s}.\n", parser->tokens[parser->index]->number_Value, parser->tokens[parser->index]->function_Name);
				exit(1);
			}
			parser->index++;
			char** args = malloc(0);
			int n_Args = 0;
			while(parser->index < parser->n_Tokens && parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, ")") != 0) {
				args = realloc(args, sizeof(char*) * (n_Args + 1));
				args[n_Args] = malloc(sizeof(char) * (strlen(parser->tokens[parser->index]->function_Name) + 1));
				strcpy(args[n_Args++], parser->tokens[(parser->index)++]->function_Name);
			}
			if(parser->index >= parser->n_Tokens) {
				fprintf(stderr, "Error {parse_Unit}: expected `)`, program ended without resolution.\n");
				exit(1);
			}
			else if(parser->tokens[parser->index]->type != T_FUNCTION || strcmp(parser->tokens[parser->index]->function_Name, ")") != 0) {
				fprintf(stderr, "Error {parse_Unit}: expected `)`, got token {%lf, %s}.\n", parser->tokens[parser->index]->number_Value, parser->tokens[parser->index]->function_Name);
				exit(1);
			}
			parser->index++;
			if(parser->index >= parser->n_Tokens) {
				fprintf(stderr, "Error {parse_Unit}: expected `begin`, program ended without resolution.\n");
				exit(1);
			}
			else if(parser->tokens[parser->index]->type != T_FUNCTION || strcmp(parser->tokens[parser->index]->function_Name, "begin") != 0) {
				fprintf(stderr, "Error {parse_Unit}: expected `begin`, got token {%lf, %s}.\n", parser->tokens[parser->index]->number_Value, parser->tokens[parser->index]->function_Name);
				exit(1);
			}
			parser->index++;
			int start = parser->index;
			int depth = 1;
			for(;parser->index < parser->n_Tokens && depth != 0;parser->index++) {
				if(parser->tokens[parser->index]->type == T_FUNCTION) {
					if(strcmp(parser->tokens[parser->index]->function_Name, "begin") == 0) {
						depth++;
					}
					else if(strcmp(parser->tokens[parser->index]->function_Name, "end") == 0) {
						depth--;
					}
				}
			}
			if(depth != 0) {
				fprintf(stderr, "Error {parse_Unit}: program ended abruptly while parsing function.\n");
				exit(1);
			}
			int end = parser->index;
			parser->scope->functions = realloc(parser->scope->functions, sizeof(struct Function*) * (parser->scope->n_Functions + 1));
			parser->scope->functions[(parser->scope->n_Functions)++] = create_Function(function_Name, args, n_Args, parser->tokens + start, end - start);
			return 0;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "call") == 0) {
			parser->index++;
			if(parser->index >= parser->n_Tokens) {
				fprintf(stderr, "Error {parse_Unit}: expected function name, program ended without resolution.\n");
				exit(1);
			}
			else if(parser->tokens[parser->index]->type != T_FUNCTION) {
				fprintf(stderr, "Error {parse_Unit}: expected function name, got `%lf`.\n", parser->tokens[parser->index]->number_Value);
				exit(1);
			}
			char* function_Name = parser->tokens[(parser->index)++]->function_Name;
			int function_Index = -1;
			for(int i=0;i<parser->scope->n_Functions;i++) {
				if(strcmp(parser->scope->functions[i]->name, function_Name) == 0) {
					function_Index = i;
					break;
				}
			}
			if(function_Index == -1) {
				fprintf(stderr, "Error {parse_Unit}: `%s` is not a valid function.\n", function_Name);
				exit(1);
			}
			if(parser->index >= parser->n_Tokens) {
				fprintf(stderr, "Error {parse_Unit}: expected `(`, program ended without resolution.\n");
				exit(1);
			}
			else if(parser->tokens[parser->index]->type != T_FUNCTION || strcmp(parser->tokens[parser->index]->function_Name, "(") != 0) {
				fprintf(stderr, "Error {parse_Unit}: expected `(`, got token {%lf, %s}.\n", parser->tokens[parser->index]->number_Value, parser->tokens[parser->index]->function_Name);
				exit(1);
			}
			parser->index++;
			double* args = malloc(0);
			int n_Args = 0;
			while(parser->index < parser->n_Tokens && !(parser->tokens[parser->index]->type == T_FUNCTION && strcmp(parser->tokens[parser->index]->function_Name, ")") == 0)) {
				args = realloc(args, sizeof(double) * (n_Args + 1));
				double value = parse_Unit(parser);
				args[n_Args++] = value;
			}
			if(parser->index >= parser->n_Tokens) {
				fprintf(stderr, "Error {parse_Unit}: expected `)`, program ended without resolution.\n");
				exit(1);
			}
			else if(parser->tokens[parser->index]->type != T_FUNCTION || strcmp(parser->tokens[parser->index]->function_Name, ")") != 0) {
				fprintf(stderr, "Error {parse_Unit}: expected `)`, got token {%lf, %s}.\n", parser->tokens[parser->index]->number_Value, parser->tokens[parser->index]->function_Name);
				exit(1);
			}
			parser->index++;
			struct Parser* function_Parser = create_Parser(parser->scope->functions[function_Index]->tokens, parser->scope->functions[function_Index]->n_Tokens);
			function_Parser->scope = from_Scope(parser->scope);
			function_Parser->scope->functions = parser->scope->functions;
			for(int i=0;i<n_Args;i++) {
				scope_Set_Value(function_Parser->scope, parser->scope->functions[function_Index]->args[i], args[i]);
			}
			double value = parse_Unit(function_Parser);
			for(int i=0;i<function_Parser->scope->length;i++) {
				free(function_Parser->scope->variable_Names[i]);
			}
			free(function_Parser->scope->variable_Names);
			free(function_Parser->scope->values);
			free(function_Parser->scope);
			for(int i=0;i<function_Parser->loop_Depth;i++) {
				destroy_Loop(function_Parser->loop_Stack[i]);
			}
			free(function_Parser->loop_Stack);
			free(function_Parser);
			free(args);
			return value;
		}
		else if(scope_Get_Index(parser->scope, parser->tokens[parser->index]->function_Name) != -1) {
			return scope_Get_Value(parser->scope, parser->tokens[(parser->index)++]->function_Name);
		}
		else {
			fprintf(stderr, "Error {parse_Unit}: could not find lexeme `%s` in the global scope.\n", parser->tokens[parser->index]->function_Name);
			exit(1);
		}
	}
	fprintf(stderr, "Error {parse_Unit}: unknown token type.\n");
	exit(1);
	return 0;
}

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
