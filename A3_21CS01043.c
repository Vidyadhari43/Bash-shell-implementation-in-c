#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#define max_size 1024
#define max_hist_size 100

//code by Ariveeti Sree Vidyadhari (21CS01043)

void shell_init()
{
    char curr_working_direc[max_size];
    printf("\033[1;37mPath specified by path environment variable:-\033[1;37m%s\n",getenv("PATH"));
    if (getcwd(curr_working_direc, sizeof(curr_working_direc)) != NULL)
    {
        printf("\033[1;33m%s@%s:\033[1;34m%s\033[1;37m$ ", getenv("USER"), getenv("HOSTNAME"), curr_working_direc);
    }
    else
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}

void execute_cmnd(char *cmnd)
{
    char *argset[max_size];
    int argcount = 0;
    int input_redirect = 0;
    int output_redirect = 0;
    char *inputfile = NULL;
    char *outputfile = NULL;
    char *token = strtok(cmnd, " ");

    while (token != NULL){
        if (strcmp(token,">") == 0){
            token = strtok(NULL," ");
            if (token != NULL){
                output_redirect = 1;
                outputfile = token;
            }
        }
        else if (strcmp(token,"<") == 0){
            token = strtok(NULL," ");
            if (token != NULL){
                input_redirect = 1;
                inputfile = token;
            }
        }
        else{
            argset[argcount++] = token;
        }
        token = strtok(NULL," ");
    }
    argset[argcount] = NULL;
    
    // execution of 'cd' command
    if (strcmp(argset[0], "cd") == 0){
        if (argcount < 2){
            printf("cd: missing argument\n");
        }
        else{
            if (chdir(argset[1]) != 0)
            {
                perror("cd");
            }
        }
        return;
    }
    pid_t pid = fork();

    if (pid < 0){
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0){
        if (input_redirect)
        {
            int file_desc = open(inputfile, O_RDONLY);
            if (file_desc == -1)//file descriptor value -1 indicates failure
            {
                perror("Input Redirection");
                exit(EXIT_FAILURE);
            }
            dup2(file_desc, STDIN_FILENO);
            close(file_desc);
        }

        if (output_redirect){
            int file_desc = open(outputfile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
            if (file_desc == -1){
                perror("Output Redirection");
                exit(EXIT_FAILURE);
            }
            dup2(file_desc, STDOUT_FILENO);
            close(file_desc);
        }
        if (execvp(argset[0], argset) == -1){
            perror("Execution failed");
            exit(EXIT_FAILURE);
        }
    }
    else{
        wait(NULL);
    }
}
void display_history(char History[max_hist_size][max_size],int count){
	int i;
	for(i=0;i<count;i++){
		printf("%d %s\n",i+1,History[i]);
	}
}
int main()
{
    struct utsname unamedata;
    if (uname(&unamedata) != 0)
    {
        perror("uname");
        return 1;
    }
    setenv("HOSTNAME", unamedata.nodename, 1);
    printf("\n\033[1;34m---New shell---\n\n");
    char *input;
    char History[max_hist_size][max_size];
    int count=0;
    system("clear");
    printf("\t\033[1;32m***NEW SHELL***\n");
    while (1)
    {
        shell_init();
        input = readline("");

        input[strcspn(input, "\n")] = '\0';
	//execution of exit command
        if (strcmp(input, "exit") == 0){
            printf("\n\033[1;32m***Exiting Successful***\n\n");
            free(input);
            break;
        }
        //execution of history command
	if(strcmp(input,"history")==0){
		display_history(History,count);	
	}
	
        if (input[0] != '\0'){
            add_history(input);
            if(count<max_hist_size){
            	strcpy(History[count],input);
            	count++;
            }
            else{
            	for(int i=0;i<max_hist_size-1;i++){
            		strcpy(History[i],History[i+1]);
            	}
            	strcpy(History[max_hist_size-1],input);
            }
            if(strcmp(input,"history")!=0){
            	execute_cmnd(input);
            }
        }
        free(input);
    }

    return 0;
}
