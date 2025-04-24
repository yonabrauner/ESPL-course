#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>


int main(int argc, char **argv)
{
	int pipefd[2];                              // preparing pipe array
	if (pipe(pipefd) == -1){                    // piping error
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "(parent_process>forking...)\n");
	int firstChildId = fork();                  // forking first child process
    if (firstChildId >= 0)
        fprintf(stderr, "(parent_process>created process with id: %d)\n", firstChildId);
    if (firstChildId < 0){ 					    // forking error
        perror("error in fork\n");
        exit(EXIT_FAILURE);
    }
	else if (firstChildId == 0){   				// in first child process
        fprintf(stderr, "(child1>redirecrting stdout to the write end of the pipe...)\n");
        fclose(stdout);                         // closing standart output
        dup(pipefd[1]);                         // duplicating the write-end
        close(pipefd[1]);                       // closing write-end of pipe
        char* args[] = {"ls", "-l", NULL};      // preparing the "ls -l" command to be executed
        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");
        execvp("ls", args);                     // executing the command
        exit(EXIT_SUCCESS);                     // gracefully end process
    }
    else{						            	// in parent process
        fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");
        close(pipefd[1]);                      
        fprintf(stderr, "(parent_process>forking...)\n");
        int secondChildId = fork();            
        if (secondChildId >= 0)
            fprintf(stderr, "(parent_process>created process with id: %d)\n", secondChildId);
        if (secondChildId < 0){				   // forking error
            perror("error in fork\n");
            exit(EXIT_FAILURE);
        }
        else if (secondChildId == 0){          // in second child process
            fprintf(stderr, "(child2>redirecrting stdin to the read end of the pipe...)\n");
            fclose(stdin);
            dup(pipefd[0]);      
            close(pipefd[0]);                  // closing read-end of pipe
            char* args[] = {"tail", "-n", "2", NULL};
            fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");
            execvp("tail", args);                 
            exit(EXIT_SUCCESS);
        }
        fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");
        close(pipefd[0]);
        fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
        waitpid(firstChildId, NULL, 0);        
        waitpid(secondChildId, NULL, 0);
        fprintf(stderr, "(parent_process>exiting...)\n");
        exit(EXIT_SUCCESS);
    }
    return 0;
}