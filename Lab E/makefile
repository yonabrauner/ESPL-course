all: myElf

myElf: myElf.o 
	gcc -m32 -g -Wall -o myElf myElf.o 

myElf.o: myElf.c
	gcc -m32 -g -Wall -c -o myElf.o myElf.c

.PHONY: clean

clean: 
	rm -f *.o myElf