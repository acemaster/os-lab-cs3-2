#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

void cat_it(FILE *fp)
{
	char buf[256];
	while(fgets(buf,sizeof(buf),fp))
		fputs(buf,stdout);
}

int main(int argc,char **argv)
{
	if(argc<2)
		cat_it(stdin);
	else
	{
		int i;
		FILE *fp;
		for(i=1;i<argc;i++)
		{
			if(!(fp=fopen(argv[i],"r")))
				fprintf(stderr, "mycat: %s\n",argv[i]);
			else{
				cat_it(fp);
				printf("\n");
				fclose(fp);
			}
		}

	}
	return 0;
}