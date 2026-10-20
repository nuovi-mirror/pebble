#include <stdio.h>

int main(int argc, char **argv) {
	if (argc < 3) return 1;

	FILE *f = fopen(argv[1], "rb");
	if (!f) return 1;

	printf("unsigned char %s_data[] = {\n", argv[2]);

	int c, col = 0;
	unsigned long len = 0;

	while ((c = fgetc(f)) != EOF) {
		if (col == 0) printf("    ");

		printf("0x%02x,", (unsigned char)c);

		if (++col == 12) {
			printf("\n");
			col = 0;
	        } else {
			printf(" ");
	        }
		
		++len;
	}

	if (col != 0) printf("\n");

	printf("};\n");
	printf("unsigned long %s_len = %lu;\n", argv[2], len);

	fclose(f);
	return ferror(stdout) ? 1 : 0;
}

