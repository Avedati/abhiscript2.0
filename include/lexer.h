#ifndef __LEXER_H_
#define __LEXER_H_ 0

enum TokenType {
	T_NUMBER = 0,
	T_FUNCTION
};

struct Token {
	enum TokenType type;
	double number_Value;
	char* function_Name;
};

struct Token* create_Token(enum TokenType, double, char*);
void destroy_Token(struct Token*);
struct Token** tokenize(char*, int*);
void display_Tokens(struct Token**, int);

#endif // __LEXER_H_
