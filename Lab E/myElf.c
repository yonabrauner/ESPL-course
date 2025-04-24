#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct {
  char *name;
  void (*fun)();
}fun_desc;

int lastFileUpdated = 2;
int debug = 0;
int Currentfd1 = -1;
int Currentfd2 = -1;
char* currentFilenameOpen1 = NULL;
char* currentFilenameOpen2 = NULL;
Elf32_Ehdr *header1;
Elf32_Ehdr *header2;
void* map_start1;
void* map_start2;
struct stat fd_stat1;
struct stat fd_stat2;

void loadFileHelper(int* Currentfd, struct stat* fd_stat, void** map_start, char** currentFilenameOpen){
	char filename[256];
	int fd;
	fscanf(stdin,"%s",filename);
    if((fd = open(filename, O_RDWR)) < 0) {
      perror("error in open");
      exit(-1);
   	}
	if(fstat(fd, fd_stat) != 0 ) {
		perror("stat failed");
		exit(-1);
	}
	if ((*map_start = mmap(0, fd_stat->st_size, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0)) == MAP_FAILED ) {
		perror("mmap failed");
		exit(-4);
	}
	*Currentfd = fd;
	strcpy((char*)currentFilenameOpen,(char*)filename);
}

int LoadFile(){
	if (lastFileUpdated == 2){
		loadFileHelper(&Currentfd1, &fd_stat1, &map_start1, &currentFilenameOpen1);
		lastFileUpdated = 1;
		return Currentfd1;
	}
	else{
		loadFileHelper(&Currentfd2, &fd_stat2, &map_start2, &currentFilenameOpen2);
		lastFileUpdated = 2;
		return Currentfd2;
	}
}

char* dataType(Elf32_Ehdr* header){
    switch (header->e_ident[5]){
    case ELFDATANONE:
        return "invalid data encoding";
        break;
    case ELFDATA2LSB:
        return "2's complement, little endian";
		break;
    case ELFDATA2MSB:
        return "2's complement, big endian";
        break;
    default:
        return "NO DATA";
        break;
    }
}

void examineFilePrinter(int* Currentfd, struct stat* fd_stat, void** map_start, char* currentFilenameOpen, Elf32_Ehdr* header){
	if (strncmp((char*)header->e_ident,(char*)ELFMAG, 4)==0){
		printf("Magic:\t\t\t\t %X %X %X\n", header->e_ident[EI_MAG0],header->e_ident[EI_MAG1],header->e_ident[EI_MAG2]);
		printf("Data:\t\t\t\t %s\n",dataType(header));
		printf("Enty point address:\t\t 0x%x\n",header->e_entry);
		printf("Start of section headers:\t %d (bytes into file)\n",header->e_shoff);
		printf("Number of section headers:\t %d\n",  header->e_shnum);
		printf("Size of section headers:\t %d (bytes)\n",header->e_shentsize);
		printf("Start of program headers:\t %d (bytes into file)\n",header->e_phoff);
		printf("Number of program headers:\t %d\n",header->e_phnum);
		printf("Size of program headers:\t %d (bytes)\n",header->e_phentsize);
	}
	else{
		printf("not an ELF file.\n");
		munmap(map_start, fd_stat->st_size); 
		close(*Currentfd);
		*Currentfd = -1;
		currentFilenameOpen = NULL;
	}
}

void examineFile(){
    printf("Enter file name: ");
	int currfd;
    if ((currfd = LoadFile()) == -1){
		printf("file loading failed.");
        return;
	}
	printf("\nfile loaded to slot %d.\n", lastFileUpdated);
	if (currfd == Currentfd1){
    	header1 = (Elf32_Ehdr *) map_start1;
		examineFilePrinter(&Currentfd1, &fd_stat1, &map_start1, currentFilenameOpen1, header1);
	}
	else{
		header2 = (Elf32_Ehdr *) map_start2;
		examineFilePrinter(&Currentfd2, &fd_stat2, &map_start2, currentFilenameOpen2, header2);
    }
}

char* sectionType(int value) {
    switch (value) {
        case SHT_NULL:return "NULL";
        case SHT_PROGBITS:return "PROGBITS";
        case SHT_SYMTAB:return "SYMTAB";
        case SHT_STRTAB:return "STRTAB";
        case SHT_RELA:return "RELA";
        case SHT_HASH:return "HASH";
        case SHT_DYNAMIC:return "DYNAMIC";
        case SHT_NOTE:return "NOTE";
        case SHT_NOBITS:return "NOBITS";
        case SHT_REL:return "REL";
        case SHT_SHLIB:return "SHLIB";
        case SHT_DYNSYM:return "DYNSYM";
        default:return "Unknown";
    }
}

void printSectionEntry(int index,char* name ,Elf32_Shdr* section,int offset){
    if(debug)
        printf("[%2d] %-18.18s\t%#06x\t\t%06d\t%06d\t%-13.10s\t%d\n",index, name ,section->sh_addr,section->sh_offset, section->sh_size, sectionType(section->sh_type),offset);
    else
        printf("[%2d] %-18.18s\t%#06x\t\t%06d\t%06d\t%-13.10s\n",index, name ,section->sh_addr,section->sh_offset, section->sh_size, sectionType(section->sh_type));
}

void printSectionNamesHelper(int Currentfd, void* map_start, Elf32_Ehdr* header){
	Elf32_Shdr* sections_table = map_start+header->e_shoff;
	Elf32_Shdr* string_table_entry = map_start+header->e_shoff+(header->e_shstrndx*header->e_shentsize);
	if(debug){
		fprintf(stderr,"\nsection table address: %p\n",sections_table);
		fprintf(stderr,"string table entry: %p\n",string_table_entry);
		printf("\n[Index] Name\t\tAddr\t\tOff\tSize\tType\t\toffset(bytes)\n");
	}
	else
		printf("\n[Index] Name\t\tAddr\t\tOff\tSize\tType\n");
	for (size_t i = 0; i < header->e_shnum; i++){
		Elf32_Shdr* entry = map_start+header->e_shoff+(i* header->e_shentsize);   
		char* name = map_start + string_table_entry->sh_offset + entry->sh_name;
		printSectionEntry(i,name,entry,header->e_shoff+(i* header->e_shentsize));
	}
}

void printSectionNames(){
	if (Currentfd1 == -1 && Currentfd2 == -1)
		printf("\nno file currently open. examine a file first.");
	if(Currentfd1 != -1)
		printSectionNamesHelper(Currentfd1, map_start1, header1);
	if(Currentfd2 != -1)
		printSectionNamesHelper(Currentfd2, map_start2, header2);
}

Elf32_Shdr* getTable(char* _name, void* map_start, Elf32_Ehdr* header){
    Elf32_Shdr* string_table_entry = map_start+ header->e_shoff+(header->e_shstrndx*header->e_shentsize);
    for (size_t i = 0; i < header->e_shnum; i++){
        Elf32_Shdr* entry = map_start+header->e_shoff+(i* header->e_shentsize);
        char* name = map_start + string_table_entry->sh_offset + entry->sh_name;
        if(strcmp(_name, name)==0)
            return entry;
    }
    return NULL;
}

void printSymbolsHelper(void* map_start, Elf32_Ehdr* header){
	Elf32_Shdr *symbol_table_entry = getTable(".symtab", header, map_start);
	Elf32_Shdr *strtab = getTable(".strtab", header, map_start);   
	Elf32_Shdr *shstrtab = getTable(".shstrtab", header, map_start);

	if (symbol_table_entry == NULL){
		perror("Symbol table not found!");
		return;
	}

	if(debug)
		printf("\n[Num]\tValue\t\tsection_index\tsection_name\t\tsymbol_name\t\tsize\n");
	else
		printf("\n[Num]\tValue\t\tsection_index\tsection_name\t\tsymbol_name\n");

	int entry_num = symbol_table_entry->sh_size / sizeof(Elf32_Sym);
	for (int i = 1; i < entry_num; i++){
		Elf32_Sym *symb_entry = map_start + symbol_table_entry->sh_offset + (i * sizeof(Elf32_Sym));
		char *section_name;
		if (symb_entry->st_shndx == 0xFFF1)
			section_name = "ABS";
		else if (symb_entry->st_shndx == 0x0)
			section_name = "UND";
		else {
			Elf32_Shdr *section_entry = map_start + header->e_shoff + (symb_entry->st_shndx * header->e_shentsize);
			section_name = map_start + shstrtab->sh_offset + section_entry->sh_name;
		}

		char *symb_name = map_start + strtab->sh_offset + symb_entry->st_name;
		
		if (debug){
			char *symb_size = map_start + strtab->sh_offset + symb_entry->st_size;
			printf("[%2d]\t%#09x\t%d\t\t%-13.20s\t\t\%-20.30s\t\t%-20.30s\n", i, symb_entry->st_value, symb_entry->st_shndx, section_name,symb_name,symb_size);
		}
		else
			printf("[%2d]\t%#09x\t%d\t\t%-13.20s\t\t\%-20.30s\n", i, symb_entry->st_value, symb_entry->st_shndx, section_name,symb_name);
	}
}

void printSymbols(){
	if (Currentfd1 == -1 && Currentfd2 == -1)
		printf("\nno file currently open. examine a file first.");
    if (Currentfd1 != -1)
		printSymbolsHelper(map_start1, header1);
    if (Currentfd2 != -1)
		printSymbolsHelper(map_start2, header2);
}

void CheckMerge(){
	if (Currentfd1 == -1 || Currentfd2 == -1){
		printf("\nneed to load two files. use 'examine file' to load files.\n");
		return;
	}
	Elf32_Shdr *symbol_table_entry1 = getTable(".symtab", header1, map_start1);
	Elf32_Shdr *symbol_table_entry2 = getTable(".symtab", header2, map_start2);
	Elf32_Shdr *shstrtab1 = getTable(".shstrtab", header1, map_start1);
	Elf32_Shdr *shstrtab2 = getTable(".shstrtab", header2, map_start2);
	Elf32_Shdr *strtab1 = getTable(".strtab", header1, map_start1);   
	Elf32_Shdr *strtab2 = getTable(".strtab", header2, map_start2);   
	if (symbol_table_entry1 == NULL || symbol_table_entry2 == NULL){
		perror("Symbol table not found!");
		return;
	}
	int entry_num1 = symbol_table_entry1->sh_size / sizeof(Elf32_Sym);
	int entry_num2 = symbol_table_entry2->sh_size / sizeof(Elf32_Sym);
	int foundFlag = 0;
	for (int i = 1; i < entry_num1; i++){
		Elf32_Sym *symb_entry1 = map_start1 + symbol_table_entry1->sh_offset + (i * sizeof(Elf32_Sym));
		char *section_name1;
		if (symb_entry1->st_shndx == 0xFFF1)
			section_name1 = "ABS";
		else if (symb_entry1->st_shndx == 0x0)
			section_name1 = "UND";
		else {
			Elf32_Shdr *section_entry1 = map_start1 + header1->e_shoff + (symb_entry1->st_shndx * header1->e_shentsize);
			section_name1 = map_start1 + shstrtab1->sh_offset + section_entry1->sh_name;
		}
		char *symb_name1 = map_start1 + strtab1->sh_offset + symb_entry1->st_name;
		
		for (int j = 1; j < entry_num2; j++){
			Elf32_Sym *symb_entry2 = map_start2 + symbol_table_entry2->sh_offset + (j * sizeof(Elf32_Sym));
			char *section_name2;
			if (symb_entry2->st_shndx == 0xFFF1)
				section_name2 = "ABS";
			else if (symb_entry2->st_shndx == 0x0)
				section_name2 = "UND";
			else {
				Elf32_Shdr *section_entry2 = map_start2 + header2->e_shoff + (symb_entry2->st_shndx * header2->e_shentsize);
				section_name2 = map_start2 + shstrtab2->sh_offset + section_entry2->sh_name;
			}
			char *symb_name2 = map_start2 + strtab2->sh_offset + symb_entry2->st_name;
			if(strcmp(symb_name1, symb_name2)==0){
				foundFlag = 1;
				if (strcmp(section_name1, "UND") == 0 && strcmp(section_name2, "UND") == 0)
					printf("Symbol \%-20.30s undefined\n", symb_name1);
				if (symb_name1[0] != '\0' && strcmp(section_name1, "UND") != 0 && strcmp(section_name2, "UND") != 0)
					printf("Symbol \%-20.30s multiply defined\n", symb_name1);
			}
		}
		if (!foundFlag && strcmp(section_name1, "UND") == 0)
			printf("Symbol \%-20.30s undefined\n", symb_name1);
		foundFlag = 0;
	}
}

void MergeFiles(){
	printf("not implemented yet");
	// create new ELF file "out.ro"
	// int fd;
	// if ((fd = open("out.ro", O_CREAT | O_WRONLY, 0644)) < 0){
	// 	perror("Failed to create file.");
	// 	return;
	// }
	// create ELF header and insert into file
		// use a copy of first file's ELF header
		// modify e_shoff field
	// ssize_t bytesWritten = write(fd, header1, sizeof(header1));
    // if (bytesWritten != sizeof(header1)) {
    //     perror("Failed to write ELF header");
    //     close(fd);
    //     return;
    // }
	// create new section header table
		// use a copy of first file's section header table
		// modify sh_off and sh_size fields in each section header
			// new size is sum of sizes
			
	// create merged sections: 
		// create merged text, data, rodata section:
			// copy contents of first file's text section, and concat second file's text section to new file
			// 
		// copy shstrtab and symtab from first file.
		// copy symtable from first file and update UNDEFINED symbols with value from second file.
	// insert new section header and merged sections into new file
	// update ELF header's e_shoff field (here or before)
	// close the file 
}

void quit(){
    if (debug) { printf("quitting\n");}
    exit(0);
}

void toggleDebugMode(){
    if (debug == 0) {
        printf("turning Debug mode on\n");
        debug = 1;
  }
  else {
    printf("turning Debug off\n");
    debug = 0;
  }
}

int main(int argc, char **argv){
	fun_desc menu[] = {{"Toggle Debug Mode",toggleDebugMode},{"Examine ELF File",examineFile},
    	    	       {"Print Section Names",printSectionNames},{"Print Symbols",printSymbols},
        	           {"Check Files For Merge",CheckMerge},{"Merge ELF Files",MergeFiles} ,{"Quit",quit}};
  	size_t ind = 0;
  	while(menu[ind].name != NULL){
		ind = ind +1; 
	}
  	while(1) {
    	printf("Please choose an operation:\n");
    	for (int i = 0; i < 7; i++)
        	printf("%d-%s\n", i, menu[i].name);
    	printf("Option: ");
    	int option;
		scanf("%d", &option);
  		if (option < 0 || option > 7)
	    	printf("\ninvalid input!\n");
	  	else
			menu[option].fun();
    	printf("\n");
  	}
	return 0;
}