#include <stdio.h>
#include <string.h>
#include "compress.h"
#include "decompress.h"

void print_help() {
	puts("Usage: huffman -c|-d -i <input_file> -o <output_file>");
	puts("Options:");
	puts("  -c           Compress input file");
	puts("  -d           Decompress input file");
	puts("  -i <file>    Specify input file");
	puts("  -o <file>    Specify output file");
	puts("  --help       Display this help message");
}

int main(int argc, char *argv[]) {
	if (argc < 2 || strcmp(argv[1], "--help") == 0) {
		print_help();
		return 0;
	}

	char *in = NULL, *out = NULL;
	int compress_flag = 0, decompress_flag = 0;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-c")) compress_flag = 1;
		else if (!strcmp(argv[i], "-d")) decompress_flag = 1;
		else if (!strcmp(argv[i], "-i") && i + 1 < argc) in = argv[++i];
		else if (!strcmp(argv[i], "-o") && i + 1 < argc) out = argv[++i];
		else {
			fprintf(stderr, "Invalid argument: %s\n", argv[i]);
			print_help();
			return 1;
		}
	}

	if (!in || !out || (compress_flag + decompress_flag != 1)) {
		fprintf(stderr, "Invalid combination of arguments.\n");
		print_help();
		return 1;
	}

	if (compress_flag)
		return compress_file(in, out);
	else
		return decompress_file(in, out);
}