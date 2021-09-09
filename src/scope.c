#include "../include/function.h"
#include "../include/scope.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
