#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <kernel/include/InitFsFmt.h>

#define LINE_LEN 100

int main(int argc, char *argv[])
{
	char *outputFilename = NULL;
	FILE *output = NULL;
	char c;
	struct InitFsFileHeader header;
	int i;
	char buffer[256];

	while(1) {
		c = getopt(argc, argv, "o:");
		if(c == -1) {
			break;
		}

		switch(c) {
			case 'o':
				outputFilename = optarg;
				break;

		}
	}

	if(outputFilename != NULL) {
		output = fopen(outputFilename, "wb");
		if(output == NULL) {
			fprintf(stderr, "Error: Could not open output file %s\n", optarg);
			exit(1);
		}
	}

	for(i=optind; i<argc; i++) {
		char *int_name;
		char *ext_name;
		char *slash;

		ext_name = argv[i];
		slash = strrchr(argv[i], '\\');
		if(slash == NULL) {
			slash = strrchr(argv[i], '/');
		}

		if(slash) {
			int_name = slash + 1;
		} else {
			int_name = argv[i];
		}

		if(output != NULL) {
			FILE *data_file;
			char *buffer;

			data_file = fopen(ext_name, "rb");
			if(data_file == NULL) {
				fprintf(stderr, "Error: Could not open file %s\n", ext_name);
				continue;
			}

			strcpy(header.name, int_name);
			fseek(data_file, 0, SEEK_END);
			header.size = ftell(data_file);
			fseek(data_file, 0, SEEK_SET);

			fwrite(&header, sizeof(header), 1, output);
			buffer = malloc(header.size);
			fread(buffer, header.size, 1, data_file);
			fwrite(buffer, header.size, 1, output);

			fclose(data_file);
		}
	}

	if(output != NULL) {
		fclose(output);
	}

	return 0;
}