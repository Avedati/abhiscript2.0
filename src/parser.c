#include "../include/function.h"
#include "../include/lexer.h"
#include "../include/loop.h"
#include "../include/parser.h"
#include "../include/scope.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		else if(strcmp(parser->tokens[parser->index]->function_Name, "==") == 0) {
			parser->index++;
			double v1 = parse_Unit(parser);
			double v2 = parse_Unit(parser);
			return v1 == v2;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "!=") == 0) {
			parser->index++;
			double v1 = parse_Unit(parser);
			double v2 = parse_Unit(parser);
			return v1 != v2;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "<=") == 0) {
			parser->index++;
			double v1 = parse_Unit(parser);
			double v2 = parse_Unit(parser);
			return v1 <= v2;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, ">=") == 0) {
			parser->index++;
			double v1 = parse_Unit(parser);
			double v2 = parse_Unit(parser);
			return v1 >= v2;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, "<") == 0) {
			parser->index++;
			double v1 = parse_Unit(parser);
			double v2 = parse_Unit(parser);
			return v1 < v2;
		}
		else if(strcmp(parser->tokens[parser->index]->function_Name, ">") == 0) {
			parser->index++;
			double v1 = parse_Unit(parser);
			double v2 = parse_Unit(parser);
			return v1 > v2;
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
