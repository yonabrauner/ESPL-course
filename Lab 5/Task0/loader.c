#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

void printPhdr(Elf32_Phdr *, int);
int foreach_phdr(void *, void (*func)(Elf32_Phdr *, int), int);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("No file found.");
        exit(EXIT_FAILURE);
    }
    char *fileName = argv[1];
    int fd = open(fileName, O_RDONLY);
    if (fd < 0)
    {
        perror("open() error.");
        exit(EXIT_FAILURE);
    }
    int length = lseek(fd, 0, SEEK_END);
    if (length < 0)
    {
        perror("lseek() error.");
        close(fd);
        exit(EXIT_FAILURE);
    }
    void *mapStart = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapStart == MAP_FAILED)
    {
        perror("mmap() error.");
        close(fd);
        return 1;
    }
    foreach_phdr(mapStart, &printPhdr, 0);
    munmap(mapStart, length);
    close(fd);
    return 0;
}
void printPhdr(Elf32_Phdr *phdr, int i)
{
    printf("Program header number %d at address %p\n", i, phdr);
}
int foreach_phdr(void *mapStart, void (*func)(Elf32_Phdr *, int), int arg)
{
    Elf32_Ehdr *header = (Elf32_Ehdr *)mapStart;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(mapStart + header->e_phoff);
    for (int i = 0; i < header->e_phnum; i++)
        func(&phdr[i], i);
    return 0;
}