#include "v4k.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "usage: %s [file] [mode]\n", argv[0]);
		fprintf(stderr, "file: %s\n", "source file to process");
		fprintf(stderr, "mode: %s\n", "to|from");
		return 1;
	}

	char *buf = file_read(argv[1]);
	if (!buf) {
		fprintf(stderr, "error: %s\n", "file does not exist!");
		return 2;
	}

	char mode = !strcmp(argv[2], "from");

	if (!mode) {
		buf = strswap(buf, "fwk", "v4k");
		buf = strswap(buf, "FWK", "V4K");
	} else {
		buf = strswap(buf, "v4k", "fwk");
		buf = strswap(buf, "V4K", "FWK");
	}

	file_write(argv[1], buf, strlen(buf));

	return 0;	
}
