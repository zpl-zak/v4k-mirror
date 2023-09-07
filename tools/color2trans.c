#include "v4k.h"

int main() {
	if (argc() < 3) {
		printf("%s [color] [trans]\n", argv(0));
		return 1;
	}

	FILE *f = fopen(argv(1), "rb");
	if (!f) {
		printf("File not found.\n");
		return 2;
	}

	//stbi_uc *stbi_load_from_file  (FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
	int x, y, bpp;
	char *buf = stbi_load_from_file()

	return 0;
}