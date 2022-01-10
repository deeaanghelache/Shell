/*
	Grupa: 251
	Studenti: Anghelache Andreea
		  Dutu Maria-Alexandra
	Proiect: Shell
	
	Comenzi:
	   * din sistem: ls, pwd, rmdir, mkdir, echo, touch
	   * implementate: cd, history, clearScreen
	
*/


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
#define HISTORY_SIZE 1000

char command[COMMAND_SIZE], *argv[PARAMETERS_SIZE], *history[HISTORY_SIZE], *path;

int commandType(){
	//if(strstr(command, "history")){
		// commands history
	//	return 1;
	//}

	// pipe -> return 2

	if(strstr(command, "||") || strstr(command, "&&")){
		// logical expressions
		return 3;
	}
	
	// suspendarea unui program -> return 
	
	// celelalte comenzi
}

void parseCommand(){
	char *p = strtok(command, " \n\t");
        argv[0] = p;
 	int cnt = 1;
        while (p) {
        	p = strtok(NULL, " \n\t");
        	argv[cnt] = p;
        	cnt ++;
        }
        
        argv[cnt] = NULL;
}

void addHistory(){
	static int contor = 0;
	history[contor] = strdup(command);
	contor = (contor + 1)%HISTORY_SIZE;
}

void readCommand(){
    fgets(command, sizeof command, stdin);
    
    if(strcmp(command, "\n") != 0)
    	addHistory();
}

void printHistory(){
	int contor = 0;
	while(history[contor] != NULL){
		printf("%d %s", contor + 1, history[contor]);
		contor++;
	}
}

void changeDirectory(){
	
	printf("ARGV[1] : %s\n", argv[1]);
	char *goToHome = "/home/andreea";
	char *user = getenv("USER");

	//strcat(goToHome, user);
	printf("%s\n", goToHome);
	
	if (argv[1] == NULL){
		chdir(goToHome);
		strcpy(path, goToHome);
		printf("%s\n", path);
	}
	else
	if (chdir(argv[1]) == -1){
		perror(NULL);
	}
}

int main() {
    system("clear");
    path = getenv("PATH");
	
    while (true) {
        printf("%s # ",getenv("USER"));
        fflush(stdout);
        readCommand();
	parseCommand();
	
	if (argv[0] == NULL) {
        	continue;
        }
        
        char cmd[COMMAND_SIZE];
        
        pid_t pid = fork();
        if (pid < 0) {
            return errno;
        } 
        else if (pid == 0) {
            // Child instructions
            
            if(strcmp(argv[0], "history") == 0){
            	// History
            	printHistory();
            } 
            
            if(strcmp(argv[0], "clearScreen") == 0) {
            	// Clear
            	system("clear");
            }
	
            if (strcmp(argv[0], "cd") == 0){
            	// Change directory
            	printf("ARGV[0] : %s\n", argv[0]);
            	
            	changeDirectory();
            	printf("1");
            } else{
            	// Other commands
		    strcpy(cmd, "/bin/");
		    strcat(cmd, argv[0]);
		    execve(cmd, argv, NULL); // NULL -> pentru envp (in envp sunt puse variabilele de sistem din mediul de executie)
            }
            break;
        } else {
            // Parent instructions
            wait(0);
        }

        if (strcmp(argv[0], "q") == 0) { // exit
            exit(0);
        }
    }

    return 0;
}



