/*
	Grupa: 251
	Studenti: Anghelache Andreea
		  Dutu Maria-Alexandra
	Proiect: Shell
	
	Comenzi:
	   * din sistem: ls, touch
	   * implementate: cd, history, clearScreen, pwd, cp, >, <, rmdir, mkdir, echo
	
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <readline/history.h> // pentru history + upArrow + downArrow
#include <readline/readline.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h> // pentru suspendare program (SIGTSTP)
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#define COMMAND_SIZE 200
#define PARAMETERS_SIZE 50
#define HISTORY_SIZE 1000
#define PATH_SIZE 500
#define NUMBER_OF_PROCS 100
#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"

char command[COMMAND_SIZE], *argv[PARAMETERS_SIZE], *history[HISTORY_SIZE], *pipes[2], path[PATH_SIZE];
static int procCnt = 0; 
int pids[NUMBER_OF_PROCS];
pthread_mutex_t mtx;

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
    //fgets(command, sizeof command, stdin);
    
    //char *usr = getenv("USER");
    //strcat(usr, " # ");
    
     //printf("%s # ",getenv("USER"));

    
    char * comm = readline("\n");
    fflush(stdout);
    //printf("%s\n", comm);
    
    strcpy(command, comm);
    // printf("Command e: %s\n", command);
    // printf("Comm e: %s\n", comm);
    
    if(strcmp(command, "\n") != 0){
    	addHistory();
    	add_history(comm);
    }
    
    free(comm);
}

void printHistory(){
	int contor = 0;
	while(history[contor] != NULL){
		printf("%d %s\n", contor + 1, history[contor]);
		contor++;
	}
}

void changeDirectory(){
	
	//printf("ARGV[1] : %s\n", argv[1]);
	char goToHome[100] = "/home/";
	char *user = getenv("USER");

	char *p = strtok(user, " ");
	strcat(goToHome, p);
	
	//printf("U %s\n", p);
	//printf("G %s\n", goToHome);
	
	if (argv[1] == NULL){
		chdir(goToHome);
		strcpy(path, goToHome);
	}
	else
	if (chdir(argv[1]) == -1){
		perror(NULL);
		exit(0);
	}else{
		if(strcmp(argv[1], "..") != 0){
			// change directory to a child directory
		
			chdir(argv[0]);
			strcat(path, "/");
			strcat(path, argv[1]);
		}
		else{
			// change directory to parent directory
			
			int stop;
			char newPath[100] = "";
			for(int i = strlen(path) - 2; i >= 0; i--){
				if(path[i] == '/'){
					stop = i; 
					break;
				}
			}
			
			for(int i = 0; i < stop; i++){
				newPath[i] = path[i];
			} 
			
			strcpy(path, newPath);
			strcat(path, "/");
			
		}
		//printf("PATH %s\n", path);
	}
	return;
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

void printWorkingDirectory(){
	printf("%s\n", path);
}

void copy(){
	int source = open(argv[1], O_RDONLY);
	
	if(source < 0)
	{
		perror(NULL);
		return;
	}
	
	int destination = open(argv[2], O_CREAT | O_WRONLY, 0777);

	if(destination < 0)
	{
		perror(NULL);
		return;
	}
	
	int reader; 
	char buffer[4096];
	
	while((reader = read(source, buffer, 4096)) > 0){
		write(destination, buffer, reader);
	}
	
	close(source);
	close(destination);
}

void childPid(int pid){

	//printf("%s a intrat aici\n", command);
	pthread_mutex_lock(&mtx);
	
	//printf("%d", pid);
	pids[procCnt] = pid;
        procCnt++;
        
        pthread_mutex_unlock(&mtx);
        //printf("tot %s e\n", command);
}

void simpleCommands(){
	char cmd[COMMAND_SIZE];
        
        pid_t pid = fork();
        if (pid < 0) {
            perror(NULL);
        } 
        else if (pid == 0) {
            // Child instructions
            
            if (strcmp(argv[0], "history") == 0){
            	// History
            	printHistory();
            	
            	int pidChild = getpid();
		childPid(pidChild);
            } else
            
            if (strcmp(argv[0], "clearScreen") == 0) {
            	// Clear
            	system("clear");
            	int pidChild = getpid();
		childPid(pidChild);
            } else
            
            if (strcmp(argv[0], "pwd") == 0) {
            	// Print Working Directory
            	printWorkingDirectory(); 	
              	int pidChild = getpid();
		childPid(pidChild);
            } else
            
            if (strcmp(argv[0], "cp") == 0) {
            	copy();
            	int pidChild = getpid();
		childPid(pidChild);
            } else
	
            if (strcmp(argv[0], "cd") == 0){
            	// Change directory
            	// printf("ARGV[0] : %s\n", argv[0]);
            	
            	changeDirectory();
            	int pidChild = getpid();
		childPid(pidChild);
            	
            	//printf("PATH %s\n", path);
            	
            	//char *u = getenv("USER");
            	//printf("USEEEER: %s\n", u);
            	//printf("1");
            } else{
            	// Other commands
		    strcpy(cmd, "/bin/");
		    strcat(cmd, argv[0]);
		    
		    int pidChild = getpid();
		    childPid(pidChild);
		    
		    execve(cmd, argv, NULL); // NULL -> pentru envp (in envp sunt puse variabilele de sistem din mediul de executie)
            }
        } else {
            // Parent instrsuctions
            wait(0);
        }
        
}


int main() {
    system("clear");
    //path = getenv("PATH");
    
    if(getcwd(path, sizeof(path)) == NULL){
    	perror(NULL);
    }
	
    //printf("%s\n", path);
	
    while (true) {
    
    	    fflush(stdout);
    	    printf(GRN "%s # ", getenv("USER"));
    	    printf(RESET);
    	    fflush(stdout);
    //char *usr = getenv("USER");
    
    //strcat(usr, " # ");

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
		    // printf("%s\n", argv[0]);
		    
		    for(int i = 0; i <= NUMBER_OF_PROCS; i++)
		    	if(pids[i] != 0){
		    		kill(pids[i], SIGKILL);
		    	}
		    exit(0);
        	}
        	
        	simpleCommands();
        }
        
    }

    return 0;
}



