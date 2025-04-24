#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 256

const char* inputFileName;

typedef struct link link;
 
typedef struct virus {
    unsigned short SigSize;     // 2 bytes
    char virusName[16];         // 16*1 bytes
    char* sig;                  // 8 bytes
} virus;

struct link {
    link* nextVirus;
    virus* vir;
};

typedef struct func_desc {
    char* name;                   
    link* (*fun)(link*);          
} func_desc;

virus* readVirus(FILE* file){
    char buf[18];
    size_t bytesRead = fread(buf,1,18,file);           // read 18 bytes from file into buffer
    if(bytesRead == 0)                                 
        return NULL; 
    virus* v = (virus*) malloc(sizeof(virus));          // mem allocation for virus, 26 bytes
    v->SigSize = buf[0] + buf[1] * 256;
    sscanf(buf + 2, "%s", v->virusName);
    v->sig = (char*) malloc(v->SigSize*sizeof(char));
    fread(v->sig, 1, v->SigSize, file);
    return v;
}

void printVirus(virus* virus, FILE* output){
    if(virus == NULL) return ;
    fprintf(output, "Virus name: %s\n" , virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output,"signatue: \n" );
    for (int i=0; i<virus->SigSize; i++)
        fprintf(output, "%02X ",virus->sig[i] & 0x000000ff); 
    fprintf(output, "\n\n");
}

void destructVirus(virus* v){
    free(v->sig);
    free(v);
    v = NULL;
}

void list_print(link* virus_list, FILE* output){
    if (virus_list != NULL){  
        printVirus(virus_list->vir, output);  
        list_print(virus_list->nextVirus, output);
    }
}

link* list_append(link* virus_list, virus* data){  
    link* newLink = (link*) malloc(sizeof(link));
    newLink->vir = data;
    newLink->nextVirus = NULL;
    if(virus_list == NULL)          // for first node
        return newLink;
    link* iter = virus_list;
    while(iter->nextVirus != NULL)
        iter = iter->nextVirus;
    iter->nextVirus = newLink;
    return virus_list;
}

void list_free(link *virus_list){
    if(virus_list != NULL){
        list_free(virus_list->nextVirus);
        destructVirus(virus_list->vir);
        free(virus_list);
    }
}

int authFile(FILE* input){
    char buffer[4];
    fread(buffer, 1, 4, input);
    if (strcmp(buffer, "VIRL") == 0 || strcmp(buffer, "VISL") == 0)
        return 0;
    return 1; 
}

link* loadSig (link* virusList){
    if(virusList != NULL) list_free(virusList);
    char fileName[MAX];
    char name[MAX];
    printf("enter a file name:\n");
    fgets(fileName, MAX, stdin);
    sscanf(fileName, "%s", name);
    FILE* input = fopen(name,"r");
    if(input == NULL){
        printf("file was not found\n");
        return virusList;
    }
    if (authFile(input)){
        printf("file is not in correct Little Endian format\n");
        return virusList;
    }
    link* head = NULL;
    virus* nextVirus = NULL;

    while(1){
        nextVirus = readVirus(input);
        if(nextVirus == NULL) // could not read the next virus
            break;
        else 
            head = list_append(head, nextVirus);
    }
    fclose(input);
    return head; 
}

link* printSig (link* virusList){
    if (virusList != NULL)
        list_print(virusList ,stdout);
    return virusList;
}

void printVirusInfo (int start, char* name, int size){
    printf("The starting byte location in the suspected file: %d\nThe virus name: %s\nThe size of the virus signature: %d\n\n", start,name,size);
}

void detect_virus(char *buffer, unsigned int size, link *virus_list){
    link* iter = virus_list;
    while(iter != NULL){
        int virusSize = iter->vir->SigSize;
        char* virusSig = iter->vir ->sig;
        for(int i=0; i<=size-virusSize; i++)
            if(memcmp(virusSig, buffer + i, virusSize) == 0)
                printVirusInfo(i, iter->vir->virusName, virusSize);             
        iter = iter->nextVirus;
    }
}

link* detectViruses (link* virusList){

    FILE* file = fopen(inputFileName,"r");
    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char buffer[10000];
    fread(buffer,1,fileSize,file);
    if (fileSize < 10000)
        detect_virus(buffer, fileSize, virusList);
    else
        detect_virus(buffer, 10000, virusList);
    fclose(file);
    return virusList;

}

void neutralize_virus(const char *fileName, int signatureOffset){
    char RET = 0xC3;
    FILE* file = fopen(fileName,"r+");
    fseek(file,signatureOffset,SEEK_SET);
    fwrite(&RET, 1, 1, file);
    fclose(file);
}

link* fixFile (link* virusList){
    char buffer[MAX];
    int offset = 0;
    printf("Enter the the starting byte location in the suspected file in hex:\n");     
    fgets(buffer, MAX, stdin);
    sscanf(buffer,"%d", &offset);
    neutralize_virus(inputFileName, offset);
    return virusList; 
}

link* quit (link* virusList){
    list_free(virusList);
    exit(0);
    return virusList;
}

int main(int argc, char** argv){

    struct func_desc menu[] = {{"Load signatures", loadSig}, {"Print signatures", printSig}, 
                           {"Detect viruses", detectViruses}, {"Fix file", fixFile},
                           {"Quit", quit}};

    char choice[MAX]; 
    int index = 0;
    int menuLen = sizeof(menu) / sizeof(func_desc);               
    link* myLink = NULL;
    inputFileName = argv[1]; 
    while(1){
        printf("Please choose a function:\n");
        for(int i=0; i<menuLen; i++) 
            printf("%d) %s\n", i+1,menu[i].name);
        fgets(choice,MAX,stdin);
        index = atoi(choice) - 1;
        if(index < 0 || menuLen - 1 < index){
            printf("invalid input\n");
            exit(0);
        }
        myLink = menu[index].fun(myLink);
        printf("\n");
    }
    return 0;
}