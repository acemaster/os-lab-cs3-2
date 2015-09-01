#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char **argv){
	int n;
	DIR *mydir;
	struct dirent *myfile;
	mydir=opendir("/proc");
	if(mydir ==NULL)
		return 0;
	while((myfile=readdir(mydir))!=NULL)
	{
		if(myfile->d_name[0]!='.')
			printf("%s\t",myfile->d_name);
	}
	closedir(mydir);
}