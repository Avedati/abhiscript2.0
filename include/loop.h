#ifndef __LOOP_H_
#define __LOOP_H_ 0

struct Loop {
	int condition_Index;
	int code_Start;
	int code_End;
};

struct Loop* create_Loop(int, int, int);
void destroy_Loop(struct Loop*);

#endif // __LOOP_H_
