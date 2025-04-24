#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

char *getName(Elf32_Word);
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

    printf("%-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s\n",
           "Type",
           "Offset",
           "VirtAddr",
           "PhysAddr",
           "FileSiz",
           "MemSiz",
           "Flg",
           "ProtFlg",
           "MapFlg",
           "Align");
    foreach_phdr(mapStart, &printPhdr, 0);

    munmap(mapStart, length);
    close(fd);
    return 0;
}

char *getName(Elf32_Word p_type)
{
    if (p_type == PT_NULL)
        return "NULL";
    else if (p_type == PT_LOAD)
        return "LOAD";
    else if (p_type == PT_DYNAMIC)
        return "DYNAMIC";
    else if (p_type == PT_INTERP)
        return "INTERP";
    else if (p_type == PT_NOTE)
        return "NOTE";
    else if (p_type == PT_SHLIB)
        return "SHLIB";
    else if (p_type == PT_PHDR)
        return "PHDR";
    else if (p_type == PT_TLS)
        return "TLS";
    else if (p_type == PT_NUM)
        return "NUM";
    else if (p_type == PT_LOOS)
        return "LOOS";
    else if (p_type == PT_GNU_EH_FRAME)
        return "GNU_EH_FRAME";
    else if (p_type == PT_GNU_STACK)
        return "GNU_STACK";
    else if (p_type == PT_GNU_RELRO)
        return "GNU_RELRO";
    else if (p_type == PT_HIOS)
        return "HIOS";
    else if (p_type == PT_LOPROC)
        return "LOPROC";
    else if (p_type == PT_HIPROC)
        return "HIPROC";
    else
        return "";
}
void printPhdr(Elf32_Phdr *phdr, int phdr_num)
{
    char flags[6] = {' ', ' ', ' ', ' ', ' ', '\0'};
    int prot_flags = 0, map_flags = MAP_SHARED;

    if (phdr->p_flags & PF_R)
    {
        flags[0] = 'R';
        prot_flags |= PROT_READ;
    }
    if (phdr->p_flags & PF_W)
    {
        flags[2] = 'W';
        prot_flags |= PROT_WRITE;
    }
    if (phdr->p_flags & PF_X)
    {
        flags[4] = 'E';
        prot_flags |= PROT_EXEC;
    }
    if (phdr->p_flags & MAP_SHARED)
        map_flags |= MAP_SHARED;

    printf("%-12s %-12x %-12x %-12x %-12x %-12x %-12s %-12d %-12d %-12x\n",
           getName(phdr->p_type),
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           flags,
           prot_flags,
           map_flags,
           phdr->p_align);
}
int foreach_phdr(void *mapStart, void (*func)(Elf32_Phdr *, int), int arg)
{
    Elf32_Ehdr *header = (Elf32_Ehdr *)mapStart;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(mapStart + header->e_phoff);

    for (int i = 0; i < header->e_phnum; i++)
        func(&phdr[i], i);

    return 0;
}
