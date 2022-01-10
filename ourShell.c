#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define COMMAND_SIZE 200
#define PARAMETERS_SIZE 50

char command[COMMAND_SIZE], *argv[PARAMETERS_SIZE];
    

// void initializeShell(){
//     printf("%s #",getenv("USER"));
// }

void readCommand(){
    fgets(command, sizeof command, stdin);
}

int main() {
    system("clear");
    


    while (true) {
        printf("%s # ",getenv("USER"));
        fflush(stdout);
        readCommand();
        char *p = strtok(command, " \n\t");
        
        if (p == NULL) {
        	continue;
        }
        
        argv[0] = p;
 	int cnt = 1;
        while (p) {
        	p = strtok(NULL, " \n\t");
        	cnt ++;
        }
        
         argv[cnt] = NULL;
        
        char cmd[COMMAND_SIZE];
        pid_t pid = fork();
        if (pid < 0) {
            return errno;
        } 
        else if (pid == 0) {
            // Child instructions
            strcpy(cmd, "/bin/");
            strcat(cmd, argv[0]);
            execve(cmd, argv, NULL); // NULL -> pentru envp (in envp sunt puse variabilele de sistem din mediul de executie)
        } else {
            // Parent instructions
            wait(0);
        }

        if (strcmp(command, "q") == 0) { // exit
            break;
        }

    }

    return 0;
}



