#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shared/InitFs.h>

#define LINE_LEN 100

int main(int argc, char *argv[])
{
	const char *filename = argv[1];
	FILE *file = fopen(filename, "r");
	FILE *output = fopen("output.initfs", "w");
	char line[LINE_LEN];
	struct InitFsFileHeader header;

	if(file == NULL) {
		fprintf(stderr, "Error: Could not open input file %s\n", filename);
		exit(1);
	}

	while(fgets(line, LINE_LEN, file) != NULL) {
		int len;
		int i;

		len = strlen(line);
		for(i=0; i<len; i++) {
			char *int_name;
			char *ext_name;
			FILE *data_file;
			char *buffer;

			if(line[i] != ':') {
				continue;
			}

			line[i] = '\0';
			int_name = line;
			ext_name = line + i + 1;

			data_file = fopen(ext_name, "r");
			if(data_file == NULL) {
				continue;
			}

			strcpy(header.name, int_name);
			fseek(data_file, 0, SEEK_END);
			header.size = ftell(data_file);
			fseek(data_file, 0, SEEK_SET);

			printf("Writing file %s -> %s (%i bytes)\n", ext_name, int_name, header.size);
			fwrite(&header, sizeof(header), 1, output);
			buffer = malloc(header.size);
			fread(buffer, header.size, 1, data_file);
			fwrite(buffer, header.size, 1, output);

			fclose(data_file);
		}
	}

	fclose(file);
	return 0;
}