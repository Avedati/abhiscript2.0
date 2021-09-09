#ifndef __SCOPE_H_
#define __SCOPE_H_ 0

struct Scope {
	int length;
	char** variable_Names;
	double* values;
	int n_Functions;
	struct Function** functions;
};

struct Scope* create_Scope(void);
struct Scope* from_Scope(struct Scope*);
void destroy_Scope(struct Scope*);
void scope_Set_Value(struct Scope*, char*, double);
double scope_Get_Value(struct Scope*, char*);
int scope_Get_Index(struct Scope*, char*);

#endif // __SCOPE_H_
