#include <stdio.h>

#include "v4k.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		return -1;
	}
	printf("%llx", hash_str(argv[1]));
	return 0;
}
