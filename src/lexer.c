#include "../include/lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
