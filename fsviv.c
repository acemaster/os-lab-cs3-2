#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <strings.h>
#include <string.h>

#define MAXFNAME	14
#define BLKSIZE		512
#define TOTALBLOCKS 8196
#define CACHESIZE 10

// Data structure definitions
struct SupBlock {
	char sb_vname[MAXFNAME];
	int	sb_nino;
	int	sb_nblk;
	int	sb_nrootdir;
	int	sb_nfreeblks;
	int	sb_nfreeinos;
	int	sb_flags;
	unsigned short sb_freeblks[CACHESIZE];
	unsigned short sb_freeinodes[CACHESIZE];
	int	sb_freeblkindex;
	int	sb_freeinoindex;	
	unsigned int sb_chktime;
	unsigned int sb_ctime;
};

struct OnDiskDirEntry {
	char	d_name[MAXFNAME];
	unsigned short	d_ino;
};

struct DirEntry {
	struct OnDiskDirEntry d_entry;
	int	d_offset;
}; 

#define ROOTDIRSIZE	((4 * 512)/sizeof(struct OnDiskDirEntry))

struct INode {
	unsigned int	i_size;
	unsigned int	i_atime;
	unsigned int	i_ctime;
	unsigned int	i_mtime;
	unsigned short	i_blks[13];
	short		i_mode;
	unsigned char	i_uid;
	unsigned char	i_gid;
	unsigned char	i_gen;
	unsigned int i_nlinks;	
};

struct InCoreINode {
	struct INode 	ic_inode;
	int 		ic_ref;
	int		ic_ino;
	short		ic_dev;
	struct InCoreINode	*ic_next;
	struct InCoreINode	*ic_prev;
};


struct OpenFileObject {
	struct InCoreINode	*ofo_inode;
	int			ofo_mode;
	unsigned int		ofo_curpos;
	int			ofo_ref;
};

//==============GLOBAL VARIABLES=====================
int fd2;
struct SupBlock s;
struct INode nullINode;
char nullbuf[BLKSIZE];
int DATABLOCKSTART;
int INODEBLOCKSTART;
int nsuperblocks;
int ninodeblocks;
int nbootblocks;
int ndatablocks;
int nrootdirblocks;
int INODETABLESIZE;
int MAXDIRENTRIES;
//============= TESTING APPLICATION USING THESE FS CALLS ==============

// Menu driven testing application for creation of fs, 
// and all file and directory related operations
int main()
{
	int status;
	status=OpenDevice(0);
	MkFS(status);
	status=ShutdownDevice(0);

}

//============= SYSTEM CALL LEVEL NOT FOLLOWED =======

//============= VNODE/VFS NOT FOLLOWED ===============

//============== UFS INTERFACE LAYER ==========================
int MkFS(int dev)
{
	bzero(nullbuf,BLKSIZE);
	write(dev,nullbuf,BLKSIZE);

	//Initailize lengths and variables
	int i;
	nsuperblocks=1;
	ninodeblocks=8;
	nbootblocks=1;
	nrootdirblocks=1;
	ndatablocks=TOTALBLOCKS-(nsuperblocks+ninodeblocks+nbootblocks+nrootdirblocks);
	INODETABLESIZE=(ninodeblocks*BLKSIZE)/sizeof(struct INode);
	MAXDIRENTRIES=BLKSIZE/sizeof(struct DirEntry);
	DATABLOCKSTART=BLKSIZE*(TOTALBLOCKS-ndatablocks);
	INODEBLOCKSTART=BLKSIZE*(nsuperblocks+nbootblocks);
	printf("Variables Initailized\n");
	printf("Data Blocks: %d\n",ndatablocks);
	printf("Maximum directory entries: %d\n",MAXDIRENTRIES);
	printf("DATABLOCKSTART: %d\n",DATABLOCKSTART);
	printf("INODEBLOCKSTART: %d\n",INODEBLOCKSTART);



	//Super block initalization
	strcpy(s.sb_vname,"root");
	s.sb_nino=(ninodeblocks*BLKSIZE)/sizeof(struct INode);
	s.sb_nblk=ndatablocks;
	s.sb_nrootdir=nrootdirblocks;
	s.sb_nfreeblks=s.sb_nblk;
	s.sb_nfreeinos=s.sb_nino;
	s.sb_flags=0;
	bzero(s.sb_freeblks,CACHESIZE);
	bzero(s.sb_freeinodes,CACHESIZE);
	s.sb_freeinoindex=CACHESIZE;
	s.sb_freeblkindex=CACHESIZE;
	s.sb_ctime=time(NULL);
	s.sb_chktime=time(NULL);
	write(dev,&s,sizeof(struct SupBlock));
	write(dev,nullbuf,(nsuperblocks*BLKSIZE)-sizeof(struct SupBlock));
	printf("Superblock initalization done\n");

	//initialize inodes
	nullINode.i_size = 0;
	nullINode.i_atime = 0;
	nullINode.i_ctime = 0;
	nullINode.i_mtime = 0;
	bzero(nullINode.i_blks, 13);
	nullINode.i_mode = 0;
	nullINode.i_uid = 0;
	nullINode.i_gid = 0;
	nullINode.i_gen = 0;
	nullINode.i_nlinks = 0;
	for(i=0; i<INODETABLESIZE; i++)
		write(dev, &nullINode, sizeof(struct INode));
	write(dev, &nullbuf, BLKSIZE%sizeof(struct INode));
	printf("INodes initialized!\n");

	//Data block initialize
	printf("%d\n",ndatablocks+nrootdirblocks );
	for(i=0; i<ndatablocks+nrootdirblocks; i++)
		write(dev, &nullbuf, BLKSIZE);
	printf("All data blocks initialized!\n");
}

// Open/create a file in the given directory with specified uid, gid 
// and attributes. Type of modes are limited to read, write and append only.
int OpenFile(int dirhandle, char *fname, int mode, int uid, int gid, int attrib)
{
}

// Close a file
int CloseFile(int fhandle)
{
}

// Read from a file already opened, nbytes into buf
int ReadFile(int fhandle, char buf[], int nbytes)
{
}

// Write into a file already opened, nbytes from buf
int WriteFile(int fhandle, char buf[], int nbytes)
{
}

// Move the file pointer to required position
int SeekFile(int fhandle, int pos, int whence)
{
}

// Create a directory
int MkDir(int pardir, char *dname, int uid, int gid, int attrib)
{
}

// Delete directory (if itis empty)
int RmDir(int pardir, char *dname)
{
}

int OpenDir(int pardir, char *dname)
{
}

int CloseDir(int dirhandle)
{
}

int SeekDir(int dirhandle, int pos, int whence)
{
}

int ReadDir(int dirhandle, struct DirEntry *dent)
{
}
int WriteDir(int dirhandle, struct DirEntry *dent)
{
}

//============== UFS INTERNAL LOW LEVEL ALGORITHMS =============
int ReadINode(int dev, int ino, struct INode *inode)
{
	lseek(dev,INODEBLOCKSTART+ino*sizeof(struct INode),SEEK_SET);
	printf("Reading inode %d from file\n",ino);
	read(dev,inode,sizeof(struct INode));
	inode->i_atime=time(NULL);
	WriteInode(dev,ino,inode);

}

int WriteInode(int dev, int ino, struct INode *inode)
{
	lseek(dev,INODEBLOCKSTART+ino*(sizeof(struct INode)),SEEK_SET);
	printf("Writing into inode no: %d\n",ino);
	inode->i_mtime=time(NULL);
	write(dev,inode,sizeof(struct INode));
	write(fd2,"\n",1);
	write(fd2,inode,sizeof(struct INode));
}

int AllocInode(int dev,struct INode *in)
{
	if(s.sb_freeinoindex == CACHESIZE){
		int f=fetchFreeINodes(dev);
		if(f == 0){
			printf("No inodes fetched\n");
			return -1;
		}
		printf("%d inodes fetched\n",f);
	}
	s.sb_nfreeinos--;
	in->i_ctime=time(NULL);
	return s.sb_freeinodes[s.sb_freeinoindex++];
}
int FreeInode(int dev, int ino)
{
	struct INode in;
	ReadINode(dev,ino,&in);
	int i;
	for(i=0;i<13 && in.i_blks[i] > 0;i++)
		FreeBlk(dev,in.i_blks[i]);
	lseek(dev,INODEBLOCKSTART+ino*(sizeof(struct INode)),SEEK_SET);
	write(dev,&nullINode,sizeof(struct INode));
	printf("Freed inode %d\n",ino);
	s.sb_nfreeinos++;
}

int AllocBlk(int dev)
{
	if(s.sb_freeblkindex == CACHESIZE){
		int b=fetchFreeBlocks(dev);
		if( b == 0){
			printf("No blocks fetched\n");
			return -1;
		}
		printf("Got %d blocks\n",b);
	}
	s.sb_nfreeblks--;
	printf("Allocated block of index %d\n",s.sb_freeblkindex);
	return s.sb_freeblks[s.sb_freeblkindex++];
}

int FreeBlk(int dev, int blk)
{
	lseek(dev,BLKSIZE*blk,SEEK_SET);
	write(dev,&nullbuf,BLKSIZE);
	printf("Freed block %d\n",blk);
	s.sb_nfreeblks++;
}


int writeDirEntry(int dev,struct INode *in, int linkno,struct DirEntry *dent)
{
	lseek(dev,getDirEntryAddress(linkno,in),SEEK_SET);
	write(dev,dent,sizeof(struct DirEntry));
}


int readDirEntry(int dev,struct INode *in, int linkno,struct DirEntry *dent)
{
	lseek(dev,getDirEntryAddress(linkno,in),SEEK_SET);
	read(dev,dent,sizeof(struct DirEntry));
	printf("Read directory %s\n",dent->d_entry.d_name);
}

int allocDirEntry(int fd, struct INode* in, struct DirEntry* d){
	writeDirEntry(fd, in, in->i_nlinks, d);
	in->i_nlinks++;
	in->i_size = sizeof(struct INode)*in->i_nlinks;
	printf("Allocated dir entry for %s of inode %u at link no %u\n",d->d_entry.d_name, d->d_entry.d_inode, in->i_nlinks-1);
}


int fetchFreeBlocks(int dev){
	if(s.sb_nfreeblks == 0){
		printf("Memory done\n");
		return 0;
	}
	lseek(dev,DATABLOCKSTART,SEEK_SET);
	int i;
	char buf[BLKSIZE];
	for(i=TOTALBLOCKS-ndatablocks;i<TOTALBLOCKS && s.sb_freeblkindex > 0; i++){
		read(dev,&buf,BLKSIZE);
		if(strlen(buf) == 0){
			s.sb_freeblks[--s.sb_freeblkindex]=i;
		}
	}
	return CACHESIZE-s.sb_freeblkindex;
}


int fetchFreeINodes(int dev){
	if(s.sb_nfreeinos == 0){
		printf("INode memory done\n");
		return 0;
	}
	lseek(dev,BLKSIZE*(nsuperblocks+nbootblocks),SEEK_SET);
	int i;
	struct INode in;
	for(i=0;i<INODETABLESIZE && s.sb_freeinoindex>0;i++){
		read(dev,in,sizeof(struct INode));
		if(in.i_blks[0]==0){
			s.sb_freeinodes[--s.sb_freeinoindex]=i;
		}
	}
	return CACHESIZE-s.sb_freeinoindex;
}

//============== DEVICE DRIVER LEVEL =====================

// Reading a logical block blk from device dev
int ReadBlock(int dev, int blk, int buf[BLKSIZE])
{
	lseek(dev,BLKSIZE*blk,SEEK_SET);
	read(dev,buf,BLKSIZE);
	printf("Read block %d\n",blk);
	return 0;
}
// Writing a logical block blk to device dev
int WriteBlock(int dev, int blk,int buf[BLKSIZE])
{
	lseek(dev,BLKSIZE*blk,SEEK_SET);
	write(dev,buf,BLKSIZE);
	printf("Wrote block at block no:%d\n",blk);
	return 0;
}

char *devfiles[] = {"filesystem.txt",NULL};
int devfd[] = {-1, -1};

// Open the device
int OpenDevice(int dev)
{
	// Open the device related file for both reading and writing.
	//
	if ((devfd[dev] = open(devfiles[dev], O_RDWR|O_CREAT, 0666)) < 0)
	{
		perror("Opening device file failure:");
		exit(0);
	}
	printf("Opened the device successfully\n");
	fd2=open("output",O_RDWR|O_CREAT, 0666);
	return devfd[dev];
}

// Shutdown the device
int ShutdownDevice(int dev)
{
	if (devfd[dev] >= 0)
		close(devfd[dev]);
	printf("Closed device successfully\n");
	return 0;
}

