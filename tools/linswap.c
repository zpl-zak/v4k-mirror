#define STACK_ALLOC_SIZE (128*1024*1024)
#include "v4k.c"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s [file]\n", argv[0]);
		fprintf(stderr, "file: %s\n", "source file to process");
		return 1;
	}

	char *buf = file_read(argv[1]);
	if (!buf) {
		fprintf(stderr, "error: %s\n", "file does not exist!");
		return 2;
	}
	file_delete(argv[1]);

	// determine swap mode
	char mode = strstri(buf, "//@#line 1 \"") == NULL;
	char **lines = strsplit(buf, "\n");

	printf("Switching #line %s\n", mode?"ON":"OFF");

	for (int i = 0; i < array_count(lines); i++) {
		char *line = STRDUP(lines[i]); //@leak
		if (!mode) {
			strrepl(&line, "//@#line 1 \"", "#line 1 \"");
		} else {
			strrepl(&line, "#line 1 \"", "//@#line 1 \"");
		}

		file_append(argv[1], va("%s\n", line), strlen(line)+1);
	}

	return 0;
}
