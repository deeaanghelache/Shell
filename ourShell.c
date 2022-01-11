/*
	Grupa: 251
	Studenti: Anghelache Andreea
		  Dutu Maria-Alexandra
	Proiect: Shell
	
	Comenzi:
	   * din sistem: ls, touch
	   * implementate: cd, history, clearScreen, pwd, cp, >, <, rmdir, mkdir, echo, pipe
	
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
#include <signal.h> 
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

void addHistory(){
	static int contor = 0;
	history[contor] = strdup(command); // strdup = transforma command in pointer
	contor = (contor + 1) % HISTORY_SIZE;
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
    	add_history(comm); // functie predefinita pentru upArrow, downArrow
    }
    
    free(comm);
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

int commandType(){
	if (strchr(command, '|')){
		// pipe
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

void parseCommand1(char *cmd){
	// parsare pentru pipe

	char *p = strtok(cmd, " \n\t");
        argv[0] = p;
 	int cnt = 1;
        while (p) {
        	p = strtok(NULL, " \n\t");
        	argv[cnt] = p;
        	cnt ++;
        }
        
        argv[cnt] = NULL;
}

void parseCommand2(){
	// parsare pentru expresii logice
}

void commandPipe(){
	char *pipe_commands[2];
	char *p = strtok(command, "|");
	pipe_commands[0] = p;
	p = strtok(NULL, "|");
	pipe_commands[1] = p;
	
	char absPath[COMMAND_SIZE];

	int fd[2]; // fd[0] is set up for reading, fd[1] is set up for writing
	pid_t pid;

    //  if successful, the array will contain two new file descriptors to be used for the pipeline
	if(pipe(fd) < 0 ){
		printf("Eroare - initializare pipe.\n"); 
		return;
	}

	pid = fork();

	if (pid == -1){
		perror(NULL);
		return;
	} else if (pid == 0){
		//  child wants to send data to the parent
		close(fd[0]); // fd[0] is set up for reading
		dup2(fd[1], STDOUT_FILENO); // Copy the data from STDOUT_FILENO to the write end
		close(fd[1]);
		
		parseCommand1(pipe_commands[0]);

		strcpy(absPath, "/bin/");
		strcat(absPath, argv[0]);
		execve(absPath, argv, NULL);

	} else {
		wait(NULL);
		
		// parent wants to receive data from the child
		close(fd[1]); // fd[1] is set up for writing
		dup2(fd[0], STDIN_FILENO); // Copy the data from the read end of the pipe to the STDIN_FILENO
        close(fd[0]);

		parseCommand1(pipe_commands[1]);

		strcpy(absPath, "/bin/");
		strcat(absPath, argv[0]);
		execve(absPath, argv, NULL);
	}
    printf("\n");
}

void redirectInputOutputCommand(){
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
		if(in){ // <
			int fd1 = open(file, O_RDONLY);
			if (fd1 < 0){
				perror(NULL);
				return;
			}
			
			dup2(fd1, STDIN_FILENO); // STDIN_FILENO sau 0  STDIN_FILENO face referire la fd1
			close(fd1);
		}
		if(out){ // >
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
		
			chdir(argv[1]);
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

void printWorkingDirectory(){
	printf("%s\n", path);
}

void copyCommand(){
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


void echoCommand(){
	if (argv[1] == NULL){
		printf("Eroare - comanda echo!\n");
		return;
	}
	for (int i = 1; i < PARAMETERS_SIZE; i++){
		if (argv[i] != NULL){
			printf("%s ", argv[i]);
		} else {
			break;
		}
	}
	printf("\n");
}

void mkdirCommand(){
	if (argv[1] == NULL){
		printf("Eroare - comanda mkdir!\n");
		return;
	} else if (argv[2] == NULL){
		if (mkdir(argv[1], 0777) == -1){ // cu toate drepturile 
			perror(NULL);
		}
	} else {
		printf("Eroare - comanda mkdir!\n");
		return;
	}
	printf("mkdir - done\n");
}

void rmdirCommand(){
	if (argv[1] == NULL){
		printf("Eroare - comanda rmdir!\n");
		return;
	} else if (argv[2] == NULL){
		if (rmdir(argv[1]) == -1 ){  
			perror(NULL);
		}
	} else {
		printf("Eroare - comanda rmdir!\n");
		return;
	}
	printf("rmdir - done\n");
}


void simpleCommands(){
	char absPath[COMMAND_SIZE];
        
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
            	copyCommand();
            	int pidChild = getpid();
				childPid(pidChild);
            } else 
            if (strcmp(argv[0], "mkdir") == 0) {
            	// Mkdir
            	mkdirCommand();
            	int pidChild = getpid();
				childPid(pidChild);
            } else 
            if (strcmp(argv[0], "rmdir") == 0) {
            	// Rmdir
            	rmdirCommand();
            	int pidChild = getpid();
				childPid(pidChild);
            } else 
            if (strcmp(argv[0], "echo") == 0) {
            	// Echo
            	echoCommand();
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
		    strcpy(absPath, "/bin/");
		    strcat(absPath, argv[0]);
		    
		    int pidChild = getpid();
		    childPid(pidChild);
		    
		    execve(absPath, argv, NULL); // NULL -> pentru envp (in envp sunt puse variabilele de sistem din mediul de executie)
            }
        } else {
            // Parent instrsuctions
            wait(0);
        }
        
}


int main() {
    system("clear");
    //path = getenv("PATH");
    
	// stocheaza in path calea directorului curent
    if(getcwd(path, sizeof(path)) == NULL){
    	perror(NULL);
    }
	
    //printf("%s\n", path);
	
    while (true) {
    	    fflush(stdout);
    	    printf(GRN "%s # ", getenv("USER")); // culoare verde pt user #
    	    printf(RESET); // reseteaza la culoarea initiala
    	    fflush(stdout);
    //char *usr = getenv("USER");
    
    //strcat(usr, " # ");

        readCommand();
	
        int tip = commandType();
        
        if (tip == 1) {        	
        	commandPipe();
        	
        	
        } else if(tip == 2){
        	parseCommand2();
    
        
        }  else if(tip == 3){
        	redirectInputOutputCommand();
    
        
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
