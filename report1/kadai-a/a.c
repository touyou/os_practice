#include <stdio.h>
#include "c.h"

int mystrcmp(const char *s, const char *a);

const char *a = "Hello, C++ world!\n";

int main(void) {
	const char *s = hello_c();
	if (mystrcmp(s, a) < 0)
		printf("%s", s);
	else
		printf("%s", a);
	return 0;
}

