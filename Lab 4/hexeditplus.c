#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX 1024
#define MAX_FILE_NAME_SIZE 100

typedef struct {
  char debug_mode; 
  char file_name[128];
  int unit_size; //Size can be either 1, 2 or 4, with 1 as the default. 
  unsigned char mem_buf[10000];
  size_t mem_count;
  unsigned char display_mode;
  int size;

} state;

void toggle_debug(state* currState){
    if(currState->debug_mode){
        currState->debug_mode = 0;
        printf("Debug flag now off");
    }
    else{
        currState->debug_mode = 1;
        printf("Debug flag now on");        
    }
} 

 

void set_file_name(state* currState) {
    printf("Please enter a file name:\n");
    fgets(currState->file_name, MAX_FILE_NAME_SIZE, stdin);

    // Remove trailing newline character, if present
    size_t length = strlen(currState->file_name);
    if (length > 0 && currState->file_name[length - 1] == '\n') {
        currState->file_name[length - 1] = '\0';
    }
    
    if (currState->debug_mode) {
        fprintf(stderr,"Debug: file name set to '%s'\n", currState->file_name);
    }
}

bool is_valid_unit(int unit){
    if( (unit == 1) || (unit == 2) || (unit == 4) )
        return true;
    else 
        return false;
} 


void set_unit_size(state* currState) {
    char user_choice[MAX];

    printf("Please enter a number:\n");
    fgets(user_choice, MAX, stdin);

    int unit = atoi(user_choice);

        if (is_valid_unit(unit)) {
            currState->unit_size = unit;
            if (currState->debug_mode)
                fprintf(stderr,"Debug: set size to %d\n", currState->unit_size);
        }
        else {
            fprintf(stderr,"Invalid unit size\n");
        }
     
}

void load_into_memory (state* currState){
    if (currState->file_name[0] == '\0') {
        fprintf(stderr, "ERROR: the file name is empty\n");
        return;
    }
    FILE* file = fopen(currState->file_name, "r");
    if(file == NULL){
        fprintf(stderr,"ERROR: the file couldn't be opened\n");
        return;
    }
    unsigned int len;
    unsigned int loc;
    char buf[MAX];
    printf("Please enter <location> <length>\n");
    fgets(buf,MAX,stdin);
    sscanf(buf, "%X%*1[ ]%d", (unsigned int *)&loc, &len);
    if(currState->debug_mode)
        printf("file name:%s\tlocation:%x\tlength:%d\n", currState->file_name,loc,len);
    
    fseek(file,loc,SEEK_SET);
    fread(currState->mem_buf, currState->unit_size, len, file);
    currState->mem_count=len*currState->unit_size;
    printf("Loaded %d units into memory\n", len*currState->unit_size);

    fclose(file);
}

void toggle_display(state* currState){
    if(currState->display_mode){
        currState->display_mode = 0;
        printf("Display flag now off, decimal representation");
    }
    else{
        currState->display_mode = 1;
        printf("Display flag now on, hexadecimal representation");        
    }
}
char* display_format(unsigned char mode) {
    char* format[] = {"%d", "%X"};
    return format[mode];
}  


void print_display_type(char base){
    if (base)
        printf("Hexadecimal\n");
    else
        printf("Decimal\n");
    printf("=======\n"); 
}
//got just the general idea online for the while
void memory_display(state* currState){
    char buf[MAX];
    unsigned char* addr;
    int u;
    char* format = display_format(currState->display_mode);
    printf("Enter address and length\n");
    fgets(buf,MAX,stdin);
    sscanf(buf,"%p %d",&addr,&u);
    print_display_type(currState->display_mode);
    if(addr == 0)
        addr = currState->mem_buf;
    unsigned char* end = addr + currState->unit_size*u;
    while (addr < end) {
        if (currState->unit_size == 1){
            printf(format, *((unsigned char*)(addr)));
            printf("\n");
        }
        if (currState->unit_size == 2){
            printf(format, *((unsigned short*)(addr)));
            printf("\n");
        }
        if (currState->unit_size == 4){
            printf(format, *((unsigned int*)(addr)));
            printf("\n");
        } 
        addr += currState->unit_size;

    }
}  
void save_into_file(state* currState){
    int len=0;
    int offset=0;
    char buf[MAX];
    unsigned char* addr=0;
    printf("Please enter <source-address> <target-location> <length>\n");
    fgets(buf,MAX,stdin);
    sscanf(buf,"%p %X %d", &addr, &offset, &len);
    if(addr == 0)
        addr = currState->mem_buf;
    
    if(currState->mem_count <offset ){
        fprintf(stderr,"ERROR: offset is bigger than size\n");
        return;
    }

    FILE* file = fopen(currState->file_name, "r+");
    if(file == NULL){
         if(currState->debug_mode)
            fprintf(stderr,"ERROR: the file couldn't be opened\n");
        return;
    }
     
    fseek(file,offset,SEEK_SET);
    fwrite(addr, currState->unit_size, len, file);       
    fclose(file);
    
}

void memory_modify(state* currState){
    int val,location;
    char buf[MAX];
    printf("Please enter <location> <value>\n");
    fgets(buf,MAX,stdin);
    sscanf(buf,"%X %X", &location, &val);
    if(currState->debug_mode){
        printf("location:");
        printf(display_format(currState->display_mode), location);
        printf("  value:");
        printf(display_format(currState->display_mode), val);
        printf("\n");

    }
   
    // if( (int)(currState->mem_count - location) < currState->unit_size){
    //     fprintf(stderr,"ERROR: not enough space");
    //     return;
    // }
    int max=256;
    for(int i=1;i<currState->unit_size;){
            max=max*256;//8 bites per byte so 2^8
    }
    //  if( val>= max ||  val<=-1*max ){
    //     fprintf(stderr,"ERROR: the val is not compatable to the unit");
    //     return;
    // }
    
    memcpy(currState->mem_buf + location, &val, currState->unit_size);

}


void quit (state* currState){
    if(currState->debug_mode)
        fprintf(stderr,"quitting\n");
    free(currState);
    currState = NULL;
    exit(0);
}

typedef struct MENU {
  char *name;
  void (*fun)(state*);
}MENU;

int main(int argc, char** argv){

     MENU menu[] = { {"Toggle Debug Mode", toggle_debug}, 
                           {"Set File Name", set_file_name}, 
                           {"Set Unit Size", set_unit_size},
                           {"Load Into Memory", load_into_memory},
                           {"Toggle Display Mode", toggle_display},
                           {"Memory Display", memory_display},
                           {"Save Into File", save_into_file},
                           {"Memory Modify", memory_modify},
                           {"Quit", quit}, 
                           {NULL, NULL }  };
    char choice[MAX]; 
    int index = 0;
    int menuLen = sizeof(menu) / sizeof(MENU)  - 1 ;          
    int lowBound = 0;         
    int highBound = menuLen - 1;    
    
    state* mystate = malloc(sizeof(state));
    mystate->unit_size = 1;    
    mystate->debug_mode = 0;
    mystate->display_mode=0;
    while(1){
    if(mystate->debug_mode)
        printf("unit size: %d\nfile name:%s\nmem count:%d\n",mystate->unit_size,mystate->file_name,(int)mystate->mem_count); 
    printf("\nChoose action:\n");
    for(int i=0; i<menuLen; i++) 
        printf("%d-%s\n", i,menu[i].name);

    fgets(choice,MAX,stdin);
    index = atoi(choice) ;

    if( index > highBound  || index < lowBound    ){
        printf("Not within bound\n");
        exit(0);
    }

    menu[index].fun(mystate);

    printf("\n");

    }
    return 0;
}