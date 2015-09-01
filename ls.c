#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>


int main(int argc, char **argv){
	DIR *mydir;
	struct dirent *myfile;
	struct stat buf;
	if(argc<2)
		mydir=opendir(".");
	else
	{
		// printf("%s\n",argv[1]);
		mydir=opendir(argv[1]);
	}
	if(mydir == NULL){
		perror("cannot open directory.......");
		return 0;
	}
	while((myfile=readdir(mydir))!=NULL)
	{
		// if(!(stat(myfile->d_name,&buf)))
			// printf("%d ",(int)buf.st_size);
		if(strcmp(myfile->d_name,".") !=0 && strcmp(myfile->d_name,"..") !=0)
			printf("%s\t",myfile->d_name );
	}
	printf("\n");
	closedir(mydir);
	return 0;
}