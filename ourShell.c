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
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h> // pentru suspendare program (SIGTSTP)
#include <fcntl.h>
#include <sys/stat.h>

#define COMMAND_SIZE 200
#define PARAMETERS_SIZE 50
#define HISTORY_SIZE 1000

char command[COMMAND_SIZE], *argv[PARAMETERS_SIZE], *history[HISTORY_SIZE], *path, *pipes[2];

int commandType(){
	if (strchr(command, '|')){
		// pipes
		return 1;
	}

	if(strstr(command, "||") || strstr(command, "&&")){
		// logical expressions
		return 2;
	}
	if(strchr(command, '>') || strchr(command, '<')){
		// redirect input/output <, >
		return 3;
	}
	
	return 4; // celelalte comenzi

}

void parseCommand4(){
	// parsare celelalte comenzi
	
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

/*char **parseCommand1(){
	// parsare pentru pipes
	char ** pipeArgv[PARAMETERS_SIZE]
	
	//for(int i = 0; i <
	
	
	//return pipeArgv;
	return ;
}*/

void parseCommand2(){
	// parsare pentru expresii logice

}

void parseCommand3(){
	// parsare pentru redirect input/output
	char cmd[100], file[100];
	bool in = false, out = false;
	if(strchr(command, '>')){
		out = true;
		char *p = strtok(command, ">");
		strcpy(cmd, p);
		p = strtok(NULL, " \n");
		strcpy(file, p);
	}
	if(strchr(command, '<')){
		in = true;
		char *p = strtok(command, "<");
		strcpy(cmd, p);
		p = strtok(NULL, " \n");
		strcpy(file, p);
	}
	
	int pid = fork();

	if (pid == -1) {
    		perror(NULL);
	} else if (pid == 0) { 
		if(in){
			int fd1 = open(file, O_RDONLY);
			if (fd1 < 0){
				perror(NULL);
				return;
			}
			
			dup2(fd1, STDIN_FILENO); // STDIN_FILENO sau 0
			close(fd1);
		
		}
		if(out){
			int fd2 = open(file, O_CREAT | O_WRONLY, 0777);
			if (fd2 < 0){
				perror(NULL);
				return;
			}
			dup2(fd2, STDOUT_FILENO);  // STDOUT_FILENO sau 1
			close(fd2);
		
		}

		
		parseCommand4();
		strcpy(cmd, "/bin/");
		strcat(cmd, argv[0]);
		execve(cmd, argv, NULL);
	} else {
    		wait(NULL);
	}	
}



void addHistory(){
	static int contor = 0;
	history[contor] = strdup(command);
	contor = (contor + 1)%HISTORY_SIZE;
}

void readCommand(){
    fgets(command, sizeof command, stdin);
    
    if(strcmp(command, "\n") != 0){
    	addHistory();
    	add_history(command);
    }
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


void commandPipes(){
	char *p = strtok(command, "|");
	pipes[0] = p;
	p = strtok(NULL, "|");
	pipes[1] = p;
	
	char **pipe1, **pipe2;
	
	//pipe1 = parseCommand1(pipes[0]);
	//pipe2 = parseCommand1(pipes[1]);
	
}



void simpleCommands(){
	char cmd[COMMAND_SIZE];
        
        pid_t pid = fork();
        if (pid < 0) {
            perror(NULL);
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
            return;
        } else {
            // Parent instructions
            wait(0);
        }
}





int main() {
    system("clear");
    path = getenv("PATH");
	
    while (true) {
        printf("%s # ",getenv("USER"));
        fflush(stdout);
        readCommand();
	
        int tip = commandType();
        
        if (tip == 1) {
        	//parseCommand1();
        	
        	commandPipes();
        	
        	
        	
        } else if(tip == 2){
        	parseCommand2();
    
        
        }  else if(tip == 3){
        	parseCommand3();
    
        
        } else {	
        	parseCommand4();
        	
		if (argv[0] == NULL) {
			continue;
		}
		
		if (strcmp(argv[0], "q") == 0) { // exit cu q sau cu CTRL + C
		    exit(0);
        	}
        	
        	simpleCommands();
        }
        
        
        
    }

    return 0;
}



