#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

void *mapStart;
Elf32_Ehdr *header;

extern int startup(int argc, char **argv, void (*start)());
char *getFlags(Elf32_Word);
char *getName(Elf32_Word);
void printPhdr(Elf32_Phdr *, int);
void load_phdr(Elf32_Phdr *, int);
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
    header = (Elf32_Ehdr *)mapStart;
    foreach_phdr(mapStart, load_phdr, fd);
    startup(argc - 1, argv + 1, (void *)(header->e_entry));

    return 0;
}

char *getFlags(Elf32_Word flags)
{
    if (flags == 0)
        return "";
    else if (flags == 1)
        return "E";
    else if (flags == 2)
        return "W";
    else if (flags == 3)
        return "WE";
    else if (flags == 4)
        return "R";
    else if (flags == 5)
        return "RE";
    else if (flags == 6)
        return "RW";
    else if (flags == 7)
        return "RWE";
    else
        return "ERROR";
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
    char *name = getName(phdr->p_type);
    char *flags = getFlags(phdr->p_flags);

    int protFlags = 0;
    if (phdr->p_flags & PF_R)
    {
        protFlags = protFlags | PROT_READ;
    }
    if (phdr->p_flags & PF_W)
    {
        protFlags = protFlags | PROT_WRITE;
    }
    if (phdr->p_flags & PF_X)
    {
        protFlags = protFlags | PROT_EXEC;
    }

    printf("%-12s %-12x %-12x %-12x %-12x %-12x %-12s %-12d %-12d %-12x\n",
           name,
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           flags,
           protFlags,
           MAP_SHARED,
           phdr->p_align);
}
void load_phdr(Elf32_Phdr *phdr, int fd)
{
    if (phdr->p_type == PT_LOAD)
    {
        void *virtualAddress = (void *)(phdr->p_vaddr & 0xfffff000);
        int offset = phdr->p_offset & 0xfffff000;
        int padding = phdr->p_vaddr & 0xfff;

        int protFlags = 0;
        if (phdr->p_flags & PF_R)
        {
            protFlags = protFlags | PROT_READ;
        }
        if (phdr->p_flags & PF_W)
        {
            protFlags = protFlags | PROT_WRITE;
        }
        if (phdr->p_flags & PF_X)
        {
            protFlags = protFlags | PROT_EXEC;
        }

        void *address = mmap(virtualAddress, phdr->p_memsz + padding, protFlags, MAP_FIXED | MAP_PRIVATE, fd, offset);
        if (address == MAP_FAILED)
        {
            perror("mmap() error.");
            exit(EXIT_FAILURE);
        }
        printPhdr(phdr, 0);
    }
}
int foreach_phdr(void *mapStart, void (*func)(Elf32_Phdr *, int), int arg)
{
    Elf32_Phdr *phdr = (Elf32_Phdr *)(mapStart + header->e_phoff);
    printf("%-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s\n",
           "Type", "Offset", "VirtAddr", "PhysAddr", "FileSiz", "MemSiz", "Flg", "ProtFlg", "MapFlg", "Align");
    for (int i = 0; i < header->e_phnum; i++)
        func(mapStart + header->e_phoff + (i * header->e_phentsize), arg);
    return 0;
}