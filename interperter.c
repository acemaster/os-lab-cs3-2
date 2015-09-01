#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int parse(char *line, char **argv)
{
	int argc=0;
     while (*line != '\0') {
          while (*line == ' ' || *line == '\t' || *line == '\n')
          {
               *line++ = '\0';     
               argc++;
          }
          if (*line) *argv++ = line;          
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n') 
               line++;            
     }
    *argv = '\0'; 
    return argc;
}

void execute(char **argv,int argc)
{
	pid_t pid;
	int status;
	int *retstatus;
	pid=fork();
	if(pid == 0){
		if(execvp(argv[0],argv)){
			puts(strerror(errno));
			exit(2);
		}
	}
	else
	{
		pid=wait(retstatus);
	}
}

int main(){
	int done=0;
	char prompt[]="acemaster@oslab$ ";
	char command[256];
	char *cmd;
	char *arg;
	int i;
	int k;
	while(done == 0)
	{
		printf("%s",prompt);
		fgets(command,256,stdin);
		char *argv[64];
		int argc;
		argc=parse(command,argv);
		// printf("%d\n",argc);
		// for(i=0;i<argc;i++){
		// 	printf("%s",argv[i]);
		// }
		// printf("sdsadadadadasa\n");
		if(argc>0)
		{
			if(strcmp(argv[0],"exit") == 0)
				return 0;
			else{
				execute(argv,argc);
			}
        }		    
	}
}