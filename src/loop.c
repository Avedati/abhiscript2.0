#include "../include/loop.h"
#include <stdlib.h>

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
