start:start.o	
	gcc -m32 start.o -o start

start.o:start.s	
	nasm -f elf32 start.s -o start.o

CC = gcc
CFLAGS = -m32

TARGET = main
SRCS = main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

start2.o: start2.s
	nasm -f elf32 start2.s -o start2.o

clean:
	rm -f $(TARGET) start2.o


util.o:util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

main.o:main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

hello:hello.o
	ld -m elf_i386 -s -o hello hello.o
	





# task0:start.o main.o util.o
# 	ld -m elf_i386 start.o main.o util.o -o task0U��`�   �   � aV�   ̀a`�   �   � aV�   ̀a�>U���`�   �]�A  ̀�E��   �]��	aV��	aV��	aV̀�   �]�̀a���    ��]�U��`�   �   �]V�   ̀a`�   �   �]V�   ̀a�>U���`�   �]�A  ̀�E��   �]���\V���\V���\V̀�   �]�̀a���    ��]�U��`�   �   ��VV�   ̀a`�   �   ��VV�   ̀a�>U���`�   �]�A  ̀�E��   �]��yVV��yVV��yVV̀�   �]�̀a���    ��]�