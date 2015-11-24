#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <strings.h>
#include <string.h>

#define MAXNAMELENGTH	14
#define BLOCKSIZE		512	
#define TOTALBLOCKS		8196		
#define CACHESIZE		10	

// Data structure definitions
// Super block structure
struct SuperBlock{
	char sb_vname[MAXNAMELENGTH];			//Superblock name
	int	sb_ninodes;							//Number of inodes
	int	sb_nblocks;							//Number of data blocks
	int	sb_rootdir;							//Inode of root directory
	int	sb_nfreeblocks;						//Number of free data blocks
	int	sb_nfreeinodes;						//Number of free inodes
	int	sb_flags;							
	unsigned short sb_freeblocks[CACHESIZE];					//Free datablock cache
	unsigned short sb_freeinodes[CACHESIZE];					//Free inode cache
	int	sb_freeblockindex;										//Free datablock cache index
	int	sb_freeinodeindex;										//Free inode cache index
	unsigned int sb_chktime;									// Last dump time
	unsigned int sb_ctime;										// Superblock creation time
};

// Inode structure
struct INode {
	unsigned int i_size;				// File size
	unsigned int i_atime;				// Last access time
	unsigned int i_ctime;				// Creation time
	unsigned int i_mtime;				// Last modified time
	unsigned int i_blocks[13];		// Data links (12 direct + 1 indirect)		
	short i_mode;						// Permissions
	unsigned int i_uid;					// User id
	unsigned int i_gid;					// Group id
	unsigned int i_gen;				
	unsigned int i_nlinks;				// Number of symbolic links			
};

struct OnDiskDirEntry{
	char d_name[MAXNAMELENGTH];			// Directory name
	unsigned short d_inode;				// Inode number of directory
};


struct DirEntry{
	struct OnDiskDirEntry d_entry;		// Directory entry
	int	d_offset;						// Offset while opening using O_APPEND flag
}; 

struct InCoreINode{
	struct INode ic_inode;				
	int	ic_ref;
	unsigned int ofo_curpos;
	int ic_ino;
	short ic_dev;
	struct InCoreINode *ic_next;
	struct InCoreINode *ic_prev;
};

struct OpenFileObject{
	struct InCoreINode *ofo_inode;
	int ofo_mode;
	int	ofo_ref;
};

#define ROOTDIRSIZE		((4*512)/sizeof(struct OnDiskDirEntry))

//============= TESTING APPLICATION USING THESE FS CALLS ==============
// Menu driven testing application for creation of fs, 
// and all file and directory related operations
int main(int argc, char** argv){
	int fd = openDevice(0);
	init_FS(fd);
	makeDir(fd, "Folder 1", 1, 1, 1);
	makeDir(fd, "Folder 2", 1, 1, 1);
	makeDir(fd, "Folder 3", 1, 1, 1);
	removeDir(fd, "Folder 2");
	int dirhandle = openDir(fd, "Folder 1");
	closeDir(dirhandle);
	shutdownDevice(0);
}

//============= GLOBAL VARIABLES ==============
struct SuperBlock s;
struct INode nullINode;
char nullbuf[BLOCKSIZE];
int INODETABLESIZE;
int nsuperblocks;
int ninodeblocks;
int nbootblocks;
int nrootdirblocks;
int ndatablocks;
int MAXDIRENTRIES;
int DATABLOCKSTART;
int INODEBLOCKSTART;
int fd2;
int currDirINode;
struct OpenFileObject o;

//============= SYSTEM CALL LEVEL NOT FOLLOWED =======

//============= VNODE/VFS NOT FOLLOWED ===============

//============== UFS INTERFACE LAYER ==========================
int init_FS(int fd){
	// Boot block dummy block (Because no boot loader nothing...)
	bzero(nullbuf, BLOCKSIZE);
	write(fd, nullbuf, BLOCKSIZE);

	// Initialize variables
	int i;
	nsuperblocks = 1;
	ninodeblocks = 8;
	nbootblocks = 1;
	nrootdirblocks = 1;
	ndatablocks = TOTALBLOCKS - nsuperblocks - ninodeblocks - nbootblocks - nrootdirblocks;
	INODETABLESIZE = (ninodeblocks*BLOCKSIZE)/sizeof(struct INode);
	MAXDIRENTRIES = BLOCKSIZE/ sizeof(struct DirEntry);
	DATABLOCKSTART = BLOCKSIZE*(TOTALBLOCKS - ndatablocks);
	INODEBLOCKSTART = BLOCKSIZE*(nsuperblocks+nbootblocks);
	
	//Initialize super block
	strcpy(s.sb_vname, "root");
	s.sb_ninodes = (ninodeblocks*BLOCKSIZE)/sizeof(struct INode);
	s.sb_nblocks = ndatablocks;
	s.sb_nfreeblocks = s.sb_nblocks;
	s.sb_nfreeinodes = s.sb_ninodes;
	s.sb_flags = 0;
	bzero(s.sb_freeblocks, CACHESIZE);
	bzero(s.sb_freeinodes, CACHESIZE);
	s.sb_freeblockindex = CACHESIZE;
	s.sb_freeinodeindex = CACHESIZE;
	s.sb_chktime = time(NULL);
	s.sb_ctime = time(NULL);
	write(fd, &s, sizeof(struct SuperBlock));
	write(fd, nullbuf, (nsuperblocks*BLOCKSIZE) - sizeof(struct SuperBlock));
	printf("Superblock initialized!\n");

	// Write initialized list of inodes
	nullINode.i_size = 0;
	nullINode.i_atime = 0;
	nullINode.i_ctime = 0;
	nullINode.i_mtime = 0;
	bzero(nullINode.i_blocks, 13);
	nullINode.i_mode = 0;
	nullINode.i_uid = 0;
	nullINode.i_gid = 0;
	nullINode.i_gen = 0;
	nullINode.i_nlinks = 0;
	for(i=0; i<INODETABLESIZE; i++)
		write(fd, &nullINode, sizeof(struct INode));
	write(fd, &nullbuf, BLOCKSIZE%sizeof(struct INode));
	printf("Inodes initialized!\n");

	// Write initialized list of directory entries
	// Fill the remaining empty datablocks
	printf("%d\n",ndatablocks+nrootdirblocks );
	for(i=0; i<ndatablocks+nrootdirblocks; i++)
		write(fd, &nullbuf, BLOCKSIZE);
	printf("All data blocks initialized!\n");

	// Write free block information (data structures)
	struct INode in;
	in.i_atime = 0;
	bzero(in.i_blocks, 13);
	in.i_blocks[0] = allocBlock(fd);
	in.i_gen = 0;
	in.i_gid = 1;
	in.i_uid = 1;
	in.i_nlinks = 0;
	in.i_mode = 0;
	struct DirEntry d;
	bzero(d.d_entry.d_name, MAXNAMELENGTH);
	strcpy(d.d_entry.d_name,".");
	s.sb_rootdir = currDirINode = d.d_entry.d_inode = allocINode(fd, &in);
	d.d_offset = 0;
	allocDirEntry(fd, &in, &d);
	writeInode(fd, s.sb_rootdir, &in);
	updateSB(fd);
	printf("\n");
	o.ofo_inode = NULL;
	o.ofo_mode = o.ofo_ref = 0;
	return 0;
}

// Open/create a file in the given directory with specified uid, gid 
// and attributes. Type of modes are limited to read, write and append only.
int openFile(int dirhandle, char *fname, int mode, int uid, int gid, int attrib){
	// Find corresponding open file object using reference no
	// Check for validity of the file in that inode
	// According to flags create or open a file and add it to open file objects
	// Return open file object reference number	
}

// Close a file
int closeFile(int fhandle){
	// Find and remove open file object using reference number
}

// Read from a file already opened, nbytes into nullbuf
int ReadFile(int fhandle, char buf[], int nbytes){
	// Find corresponding open file object using reference no and get its inode
	// Using offset read nbytes of the file in buf
}

// Write into a file already opened, nbytes from nullbuf
int WriteFile(int fhandle, char nullbuf[], int nbytes){
	// Find corresponding open file object using reference no and get its inode
	// Using offset write nbytes of the file in buf	
}

// Move the file pointer to required position
int SeekFile(int fhandle, int pos, int whence){
	// Find corresponding open file object using reference no and get its inode
	// Change offset		
}

// Create a directory
int makeDir(int fd, char *dname, int uid, int gid, int attributes){
	printf("Creating new directory %s\n",dname);
	int parINodeNo, inodeNo;
	struct INode parent_in;
	struct DirEntry d;

	//Tokenize the dname, check for validity and find its parent directory
	parINodeNo = currDirINode;
	readINode(fd, parINodeNo, &parent_in);

	// Check if the dname already exists in the INode in
	if( fileExists(fd, dname, parent_in, &d)!=-1 ){
		printf("Directory already exists!\n\n");
		return -1;
	}

	// Initialize an inode and data block for new directory
	struct INode in;
	in.i_atime = 0;
	bzero(in.i_blocks, 13);
	in.i_blocks[0] = allocBlock(fd);
	in.i_gen = 0;
	in.i_gid = gid;
	in.i_uid = uid;
	in.i_nlinks = 0;
	in.i_mode = attributes;
	bzero(d.d_entry.d_name, MAXNAMELENGTH);
	strcpy(d.d_entry.d_name, ".");
	int allocatedINode = allocINode(fd, &in);
	d.d_entry.d_inode = allocatedINode;
	d.d_offset = 0;
	allocDirEntry(fd, &in, &d);
	bzero(d.d_entry.d_name, MAXNAMELENGTH);
	strcpy(d.d_entry.d_name, "..");
	d.d_entry.d_inode = parINodeNo;
	allocDirEntry(fd, &in, &d);
	writeInode(fd, allocatedINode, &in);

	// Add its DirEntry in its parent INode and rewrite Inode entry
	bzero(d.d_entry.d_name, MAXNAMELENGTH);
	strcpy(d.d_entry.d_name, dname);
	d.d_entry.d_inode = allocatedINode;
	allocDirEntry(fd, &parent_in, &d);
	writeInode(fd, parINodeNo, &parent_in);
	printf("\n");
	return 0;
}

int removeDir(int fd, char* dname){
	removeDirHelper(fd, dname, currDirINode);
}
// Delete directory (if it is empty)
int removeDirHelper(int fd, char *dname, int parINodeNo){
	int i;
	struct INode parent_in;
	// Tokenize the name and find its parent directory
	readINode(fd, parINodeNo, &parent_in);

	// Find dname in parent directory and read its inode
	struct DirEntry d;
	int linkNo = fileExists(fd, dname, parent_in, &d);
	int inodeNo = d.d_entry.d_inode;
	if( linkNo==-1 ){
		printf("Directory does not exist!\n");
		return -1;
	}
	if( inodeNo == s.sb_rootdir ){
		printf("Cannot delete root directory!\n");
		return -1;
	}
	struct INode in;
	readINode(fd, inodeNo, &in);

	// On each of the link in its inode call recursive delete after taking permission
	if( in.i_nlinks==0 ){
		printf("Not a valid directory!\n");
		return -1;
	}
	if( in.i_nlinks>2 ){
		printf("Directory not empty! Do you want to proceed[y]? ");
		char c;
		scanf("%c",&c);
		if( c!='y' || c!='Y' ){
			return -1;
		}
		struct DirEntry temp;
		struct INode tempINode;
		for(i=1; i<in.i_nlinks; i++){
			readDirEntry(fd, i, &temp);
			if( strcmp(temp.d_entry.d_name, "..")==0 )	continue;
			readINode(fd, temp.d_entry.d_inode, &tempINode);
			if( tempINode.i_nlinks==0 )		freeINode(fd, temp.d_entry.d_inode);
			else{
				removeDirHelper(fd, temp.d_entry.d_name, inodeNo);
			}
		}
	}
	freeINode(fd, inodeNo);
	freeDirEntry(fd, &parent_in, linkNo);
	printf("Directory successfully deleted!\n");
}

int openDir(int fd, char *dname){

	struct INode parent_in;
	struct InCoreINode* icn;
	struct DirEntry d;
	readINode(fd, currDirINode, &parent_in);
	
	// Tokenize the name and find its inode no
	int linkNo = fileExists(fd, dname, parent_in, &d);
	if( linkNo==-1 ){
		printf("No such directory exists!\n");
		return -1;
	}
	int dirhandle = isOpen(d.d_entry.d_inode, icn);
	if( dirhandle!=-1 )
		return dirhandle;
	
	// Add the inode to open file object DLL and return the reference number
	return addOpenFile(fd, d.d_entry.d_inode);
}

int closeDir(int dirhandle){
	// Find the corresponding open file object and remove it
	int i;
	struct InCoreINode icn;
	struct DirEntry d;
	struct INode in;
	int inodeNo = isOpen(dirhandle, &icn);
	if( inodeNo==-1 ){
		printf("Directory not open!\n");
		return -1;
	}
	if( icn.ic_inode.i_nlinks==0 ){
		// readDirEntry(icn.ic_inode, )
		printf("Not a directory!%d\n", icn.ic_ino);
		return -1;
	}
	for(i=1; i<icn.ic_inode.i_nlinks; i++){
		readDirEntry(icn.ic_dev, &icn.ic_inode, i, &d);
		readINode(icn.ic_dev, d.d_entry.d_inode, &in);
		if( in.i_nlinks==0 )	closeFile(d.d_entry.d_inode);
		else					closeDir(d.d_entry.d_inode);
	}
	removeOpenFile(dirhandle);

}

int SeekDir(int dirhandle, int pos, int whence){
	// Find the corresponding open file object
	// Increase its offset
}

int readDir(int inodeNo, struct DirEntry *dent){
	// Find the corresponding open file obejct
	// Read one DirEntry
	// Increase offset
}

//============== UFS INTERNAL LOW LEVEL ALGORITHMS =============
int readINode(int fd, int inodeNo, struct INode *inode){
	lseek(fd, INODEBLOCKSTART + inodeNo*sizeof(struct INode), SEEK_SET);
	printf("Reading inode number %d at %lu\n", inodeNo, INODEBLOCKSTART + inodeNo*sizeof(struct INode));
	read(fd, inode, sizeof(struct INode));
	inode->i_atime = time(NULL);
	writeInode(fd, inodeNo, inode);
}

int writeInode(int fd, int inodeNo, struct INode *inode){
	lseek(fd, INODEBLOCKSTART + inodeNo*sizeof(struct INode), SEEK_SET);
	printf("\tWriting inode number %d at %lu\n", inodeNo, INODEBLOCKSTART + inodeNo*sizeof(struct INode));
	inode->i_mtime = time(NULL);
	write(fd, inode, sizeof(struct INode));
	write(fd2, "\n", 1);
	write(fd2, inode, sizeof(struct INode));
}

int allocINode(int fd, struct INode* in){
	if( s.sb_freeinodeindex==CACHESIZE ){
		printf("Fetching inodes into cache\n");
		int fetched = fetchFreeINodes(fd);
		if( fetched==0 ){
			printf("Could not fetch any inodes!\n");
			return -1;
		}
		printf("Fetched %d inodes\n",fetched);
	}
	s.sb_nfreeinodes--;
	printf("Allocated inode no %d\n", s.sb_freeinodes[s.sb_freeinodeindex]);
	in->i_ctime = time(NULL);
	return s.sb_freeinodes[s.sb_freeinodeindex++];
}

int freeINode(int fd, int inodeNo){
	struct INode in;
	int i;
	readINode(fd, inodeNo, &in);
	for(i=0; i<13 && in.i_blocks[i]>0; i++){
		freeBlock(fd, in.i_blocks[i]);
	}
	lseek(fd, INODEBLOCKSTART + inodeNo*sizeof(struct INode), SEEK_SET);
	write(fd, &nullINode, sizeof(struct INode));
	printf("Freed inode number %d\n", inodeNo);
	s.sb_nfreeinodes++;
}

/*
int ReadBlock(int dev, int blk, int nullbuf[BLOCKSIZE]){
	// Check for validity of the block
	// Check for validity of the device

	// If OK read the block
	lseek(device_fd[dev], blk * BLOCKSIZE, SEEK_SET);
	return read(device_fd[dev], nullbuf, BLOCKSIZE);
}

// Writing a logical block blk to device dev
int WriteBlock(int dev, int blk){
	// Check for validity of the block
	// Check for validity of the device

	// If OK write the block
	lseek(device_fd[dev], blk * BLOCKSIZE, SEEK_SET);
	return write(device_fd[dev], nullbuf, BLOCKSIZE);
}
*/

int allocBlock(int fd){
	if( s.sb_freeblockindex==CACHESIZE ){
		printf("Fetching data blocks into cache..\n");
		int fetched = fetchFreeBlocks(fd);
		if( fetched==0 ){
			printf("Could not fetch data blocks!\n");
			return -1;
		}
		printf("Fetched %d data blocks\n",fetched);
	}
	s.sb_nfreeblocks--;
	printf("Allocated block no %d\n",s.sb_freeblocks[s.sb_freeblockindex] );
	return s.sb_freeblocks[s.sb_freeblockindex++];
}

int freeBlock(int fd, int blockNo){
	lseek(fd, BLOCKSIZE*blockNo, SEEK_SET);
	write(fd, &nullbuf, BLOCKSIZE);
	printf("Freed block no %d\n", blockNo);
	s.sb_nfreeblocks++;
}

int writeDirEntry(int fd, struct INode* in, int linkNo, struct DirEntry* dent){
	lseek(fd, getDirEntryAddress(linkNo, in), SEEK_SET);
	printf("\tWriting directory entry %s at %d\n",dent->d_entry.d_name, getDirEntryAddress(linkNo, in));
	write(fd, dent, sizeof(struct DirEntry));
	write(fd2, "\n", 1);
	write(fd2, dent, sizeof(struct DirEntry));
}

int readDirEntry(int fd, struct INode* in, int linkNo, struct DirEntry* dent){
	lseek(fd, getDirEntryAddress(linkNo, in), SEEK_SET);
	read(fd, dent, sizeof(struct DirEntry));
	printf("Read Dir Entry %s\n",dent->d_entry.d_name);
}

int freeDirEntry(int fd, struct INode* in, int linkNo){
	struct DirEntry last;
	readDirEntry(fd, in, in->i_nlinks-1, &last);
	writeDirEntry(fd, in, linkNo, &last);
	lseek(fd, getDirEntryAddress(in->i_nlinks-1, in), SEEK_SET);
	write(fd, &nullbuf, sizeof(struct DirEntry));
	in->i_nlinks--;
}

int allocDirEntry(int fd, struct INode* in, struct DirEntry* d){
	writeDirEntry(fd, in, in->i_nlinks, d);
	in->i_nlinks++;
	in->i_size = sizeof(struct INode)*in->i_nlinks;
	printf("Allocated dir entry for %s of inode %u at link no %u\n",d->d_entry.d_name, d->d_entry.d_inode, in->i_nlinks-1);
}

int fetchFreeBlocks(int fd){
	if( s.sb_nfreeblocks==0 ){
		printf("Block memory overflow!\n");
		return 0;
	}
	lseek(fd, DATABLOCKSTART, SEEK_SET);
	int i;
	char buf[BLOCKSIZE];
	for(i = TOTALBLOCKS-ndatablocks; i<TOTALBLOCKS && s.sb_freeblockindex>0; i++){
		read(fd, &buf, BLOCKSIZE);
		if( strlen(buf)==0 ){
			s.sb_freeblocks[--s.sb_freeblockindex] = i;
		}
	}
	return CACHESIZE-s.sb_freeblockindex;
}
	
int fetchFreeINodes(int fd){
	if( s.sb_nfreeinodes==0 ){
		printf("Inode memsetmory overflow!\n");
		return 0;
	}
	lseek(fd, BLOCKSIZE*(nsuperblocks+nbootblocks), SEEK_SET);
	int i;
	struct INode in;
	for(i = 0; i<INODETABLESIZE && s.sb_freeinodeindex>0; i++){
		read(fd, &in, sizeof(struct INode));
		if( in.i_blocks[0]==0 ){
			s.sb_freeinodes[--s.sb_freeinodeindex] = i;
		}
	}
	return CACHESIZE-s.sb_freeinodeindex;
}

int updateSB(int fd){
	lseek(fd, BLOCKSIZE*nbootblocks, SEEK_SET);
	write(fd, &s, sizeof(struct SuperBlock));
}

int fileExists(int fd, char *name, struct INode in, struct DirEntry* d){
	int i=0;
	for(i=0; i<in.i_nlinks; i++){
		readDirEntry(fd, &in, i, d);
		printf("Searching %s\n", d->d_entry.d_name);
		if( strcmp(d->d_entry.d_name, name)==0 ){
			return i;
		}
	}
	return -1;
}

int isOpen(int inodeNo, struct InCoreINode* icn){
	struct InCoreINode* temp = o.ofo_inode;
	icn = NULL;
	if( temp==NULL )		return -1;
	
	while( temp->ic_ino != inodeNo ){
		temp = temp->ic_next;
		if( temp==NULL )	return -1;
	}
	icn = temp;
	// printf("Found %d with links = %u\n",icn->ic_ino, icn->ic_inode.i_nlinks );
	return inodeNo;
}

int addOpenFile(int fd, int inodeNo){
	printf("Opening directory no %d\n", inodeNo);
	struct InCoreINode *icn = malloc(sizeof(struct InCoreINode));
	struct INode in;
	readINode(fd, inodeNo, &icn->ic_inode);
	icn->ic_ref = 0;
	icn->ofo_curpos = 0;
	icn->ic_ino = inodeNo;
	icn->ic_dev = fd;
	icn->ic_next = NULL;
	icn->ic_prev = NULL;
	struct InCoreINode* temp = o.ofo_inode;
	if( o.ofo_inode==NULL ){
		o.ofo_inode = icn;
		return inodeNo;
	}
	while( temp->ic_next!=NULL ){
		temp = temp->ic_next;
	}
	temp->ic_next = icn;
	icn->ic_prev = temp;
	return inodeNo;
}

int removeOpenFile(int fd, int inodeNo){
	struct InCoreINode* temp = o.ofo_inode;
	while( temp->ic_ino!=inodeNo ){
		temp = temp->ic_next;
		if( temp==NULL ){
			return -1;
		}
	}
	temp->ic_prev->ic_next = temp->ic_next;
	temp->ic_next->ic_prev = temp->ic_prev;
	writeInode(fd, temp->ic_ino, &temp->ic_inode);
}

int findInCoreINode(int dirhandle, struct InCoreINode* icn){
}

int getDirEntryAddress(int linkNo, struct INode* in){
	return BLOCKSIZE*(in->i_blocks[linkNo/MAXDIRENTRIES]) + sizeof(struct DirEntry)*((linkNo-1)%MAXDIRENTRIES);
}

/*
//============== DEVICE DRIVER LEVEL =====================

// Reading a logical block blk from device dev
int ReadBlock(int dev, int blk, int nullbuf[BLOCKSIZE])
{
	// Check for validity of the block
	// Check for validity of the device

	// If OK read the block
	lseek(device_fd[dev], blk * BLOCKSIZE, SEEK_SET);
	return read(device_fd[dev], nullbuf, BLOCKSIZE);
}

// Writing a logical block blk to device dev
int WriteBlock(int dev, int blk)
{
	// Check for validity of the block
	// Check for validity of the device

	// If OK write the block
	lseek(device_fd[dev], blk * BLOCKSIZE, SEEK_SET);
	return write(device_fd[dev], nullbuf, BLOCKSIZE);
}
*/

char *device_name[] = {"filesystemcore.txt",NULL};
int device_fd[] = {-1, -1};

// Open the device
int openDevice(int dev){
	// Open the device related file for both reading and writing.
	if ((device_fd[dev] = open(device_name[dev], O_RDWR|O_CREAT, 0666)) < 0){
		perror("Opening device file failure:");
		exit(1);
	}
	printf("Device %s successfully mounted\n\n", device_name[dev]);
	fd2 = open("output", O_RDWR|O_CREAT, 0666);
	return device_fd[dev];
}

// Shutdown the device
int shutdownDevice(int dev){
	if (device_fd[dev] >= 0)
		close(device_fd[dev]);
	printf("Device %s is successfully shutdown!\n", device_name[dev]);
}

