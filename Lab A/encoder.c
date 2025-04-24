#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
	int debug = 0;
	FILE * errorOutput = stderr;
	FILE * output = stdout;
	FILE* input = stdin;
	enum key {same, add, sub};
	int op = same;
	int keyLocation = 0;
	for (int i = 1; i < argc; i++){
		if (debug == 1 && strcmp(argv[i],"-D") !=  0){
			fprintf(output, "%s\n", argv[i]);
		}
		if (strcmp(argv[i],"+D") == 0){
			if (debug == 0)
				debug = 1;
			else
				fprintf(errorOutput, "%s\n", "illegal command arguments.");
		}
		if (strcmp(argv[i],"-D") == 0){
			if (debug == 1)
				debug = 0;	
			else
				fprintf(errorOutput, "%s\n", "illegal command arguments.");
		}
		if (strncmp(argv[i],"+e", 2) ==  0){
			op = add;
			keyLocation = i;
		}
		if (strncmp(argv[i],"-e", 2) ==  0){
			op = sub;
			keyLocation = i;
		}
		if (strncmp(argv[i],"-i",2) == 0){
			if (input != stdin)
				fprintf(errorOutput, "%s\n", "illegal command arguments.");
			else{
				input = fopen(&argv[i][2], "r");
				if (input == NULL){
					fprintf(errorOutput, "%s\n", "illegal command arguments.");
					return 1;
				}
			}
		}
		if (strncmp(argv[i],"-o",2) == 0){
			if (output != stdout)
				fprintf(errorOutput, "%s\n", "illegal command arguments.");
			else{
				output = fopen(&argv[i][2], "w");
				if (output == NULL){
					fprintf(errorOutput, "%s\n", "illegal command arguments.");
					return 0;
				}
			}
		}
	}

	int keyIndex = 2;
	int ch = 0;

	while ((ch = fgetc(input)) != EOF){
		switch (op)
		{
		case same:
			fprintf(output, "%c" , ch);
			break;
		
		case add:
			if ((65 <= ch) && (ch <= 90)){			// Upper case
				ch = (ch - 'A' + argv[keyLocation][keyIndex]-48) % 26 + 'A';
			}
			else
			{
				if ((97 <= ch) && (ch <= 122)){		// Lower case
					ch = (ch - 'a' + argv[keyLocation][keyIndex]-48) % 26 + 'a';
				}
				else
				{
					if ((48 <= ch) && (ch <= 57)){	// numbers
						ch = (ch - '0' + argv[keyLocation][keyIndex]-48) % 10 + '0';
					}
				}
			}
			fprintf(output, "%c" , ch);

			break;

		case sub:
			if ((65 <= ch) && (ch <= 90)){			// Upper case
				ch = (ch - 'A' - argv[keyLocation][keyIndex] + 74) % 26 + 'A';
			}
			else
			{
				if ((97 <= ch) && (ch <= 122)){		// Lower case
					ch = (ch - 'a' - argv[keyLocation][keyIndex] + 74) % 26 + 'a';
				}
				else
				{
					if ((48 <= ch) && (ch <= 57)){	// numbers						
						ch = (ch - '0' - argv[keyLocation][keyIndex] + 58) % 10 + '0';
					}
				}	
			}
			fprintf(output, "%c" , ch);

			break;
		}
		keyIndex++;
			if ((argv[keyLocation][keyIndex]) == '\0')
				keyIndex = 2;
		fflush(output);
	}

	if (input != stdin)
		fclose(input);
	if (output != stdout)
		fclose(output);

	return 0;
}