#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <kernel/include/InitFsFmt.h>

#define LINE_LEN 100

int main(int argc, char *argv[])
{
	char *outputFilename = NULL;
	char *depsFilename = NULL;
	FILE *input = NULL;
	FILE *deps = NULL;
	FILE *output = NULL;

	char c;
	int idx;

	char line[LINE_LEN];
	struct InitFsFileHeader header;

	while(1) {
		c = getopt(argc, argv, "o:d:");
		if(c == -1) {
			break;
		}

		switch(c) {
			case 'o':
				outputFilename = optarg;
				break;

			case 'd':
				depsFilename = optarg;
				break;
		}
	}

	input = fopen(argv[optind], "r");

	if(input == NULL) {
		fprintf(stderr, "Error: Could not open input file %s\n", argv[optind]);
		exit(1);
	}

	if(depsFilename != NULL) {
		deps = fopen(depsFilename, "w");
		if(deps == NULL) {
			fprintf(stderr, "Error: Could not open output file %s\n", optarg);
			exit(1);
		}
	}

	if(outputFilename != NULL && depsFilename == NULL) {
		output = fopen(outputFilename, "w");
		if(output == NULL) {
			fprintf(stderr, "Error: Could not open output file %s\n", optarg);
			exit(1);
		}
	}

	if(deps != NULL) {
		fprintf(deps, "%s: ", outputFilename);
	}

	while(fgets(line, LINE_LEN, input) != NULL) {
		int len;
		int i;
		len = strlen(line);
		line[len-1] = '\0';
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

			if(output != NULL && deps == NULL) {
				data_file = fopen(ext_name, "r");
				if(data_file == NULL) {
					fprintf(stderr, "Error: Could not open file %s\n", ext_name);
					continue;
				}

				strcpy(header.name, int_name);
				fseek(data_file, 0, SEEK_END);
				header.size = ftell(data_file);
				fseek(data_file, 0, SEEK_SET);

				//printf("Writing file %s -> %s (%i bytes)\n", ext_name, int_name, header.size);
				fwrite(&header, sizeof(header), 1, output);
				buffer = malloc(header.size);
				fread(buffer, header.size, 1, data_file);
				fwrite(buffer, header.size, 1, output);

				fclose(data_file);
			}

			if(deps != NULL) {
				fprintf(deps, "%s ", ext_name);
			}

			break;
		}
	}

	if(deps != NULL) {
		fprintf(deps, "\n");
		fclose(deps);
	}

	if(output != NULL) {
		fclose(output);
	}

	return 0;
}