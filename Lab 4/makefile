hexeditplus: hexeditplus.o 
	gcc -m32 -g -Wall -o hexeditplus hexeditplus.o 

hexeditplus.o: hexeditplus.c
	gcc -g -Wall -m32  -c -o hexeditplus.o hexeditplus.c

digit_cnt: digit_cnt.o 
	gcc -g -m32 -lm -Wall -o digit_cnt digit_cnt.o

digit_cnt.o: digit_cnt.c
	gcc -g -m32 -fno-pie -Wall -c -o digit_cnt.o digit_cnt.c