#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct func_desc {
    char *name;
    char (*fun)(char);
};

char* map(char *array, int array_length, char (*f) (char)){
	char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
	for (int i = 0; i < array_length; i++)
		mapped_array[i] = f(array[i]);
	return mapped_array;
}

int range_check(char c){
	if (c >= 0x20 && c<= 0x7E)
		return 1;
	else
		return 0;
}

char my_get(char c){
	return fgetc(stdin);
}

char cprt(char c){
	int rangeCheck = range_check(c);
	if (rangeCheck)
		printf("%c\n", c-0);
	else
		printf(".\n");
	return c;
}

char encrypt(char c){
	int rangeCheck = range_check(c);
	if (rangeCheck)
		return c+1;
	else
		return c;
}
 
char decrypt(char c){
	int rangeCheck = range_check(c);
	if (rangeCheck)
		return c-1;
	else
		return c;
}

char xprt(char c){
	int rangeCheck = range_check(c);
	if (rangeCheck)
		printf("%x\n", c & 0xff);
	else
		printf(".\n");
	return c;
}

struct func_desc menu[] = {{"Get string", &my_get}, {"Print string", &cprt}, {"Encrypt", &encrypt}, {"Decrypt", &decrypt}, {"Print Hex", &xprt}, {NULL, NULL}};

int main(int argc, char **argv){
    printf("Select operation from the following menu:\n");
    int bound;
    for (int i = 0; menu[i].name != NULL; i++){
            printf("%i) %s\n", i, menu[i].name);
            bound = i;
        }
    char* carray = malloc(5*sizeof(char));
    char option[10];
    while ((fgets(option, 5, stdin))){
        int x = option[0]-'0';
        if (x >= 0 && x <= bound){
            printf("Within bounds\n");
            carray = map(carray, 5, menu[x].fun);
        }
        else{
            printf("Not within bounds - quitting\n");
            return 1;
        }
            
        for (int i = 0; menu[i].name != NULL; i++){
            printf("%i) %s\n", i, menu[i].name);
        }
        if (x == 10)
            printf("Select operation from the following menu: (ctrl+D to exit)\n");
    }
    return 0;
}