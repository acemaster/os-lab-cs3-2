#include<stdio.h>

#define MAXFNAME	14
#define BLKSIZE		512

// Data structure definitions
struct SupBlock {
	char sb_vname[MAXFNAME];
	int	sb_nino;
	int	sb_nblk;
	int	sb_nrootdir;
	int	sb_nfreeblk;
	int	sb_nfreeino;
	int	sb_flags;
	unsigned short sb_freeblks[BLKSIZE/sizeof(unsigned short)];
	int	sb_freeblkindex;
	int	sb_freeinoindex;	
	unsigned int	sb_chktime;
	unsigned int	sb_ctime;
	unsigned short sb_freeinos[(BLKSIZE - (54))/2];
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
	unsigned char	i_lnk;
};

#define INODETABSIZE	((16 * 512) / sizeof(struct INode))

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

//============= TESTING APPLICATION USING THESE FS CALLS ==============

// Menu driven testing application for creation of fs, 
// and all file and directory related operations
int main()
{
}

//============= SYSTEM CALL LEVEL NOT FOLLOWED =======

//============= VNODE/VFS NOT FOLLOWED ===============

//============== UFS INTERFACE LAYER ==========================
int MkFS(int dev, int ninodes, int nrootdir, int blksize)
{
	int rootentrycount = ROOTDIRSIZE;
	int inodecount = INODETABSIZE;
	int blkcount = 8192;
	char buf[512];
	struct SupBlock sb;
	int reservblks = 1 + 2 + 8 + 4; // boot, super, inodetable, rootdir blks

	// Assuming only predefined sizes are used
	// Otherwise we have to recompute according to the parameters given.
	//
	
	// Boot block dummy block (Because no boot loader nothing...)
	bzero(buf, 512);
	write(devfds[dev], buf, 512);	
	
	// Write initialized superblock
	strcpy(sb.sb_vname, "TESTFILESYS");
	sb.sb_nino = inodecount;
	sb.sb_nblk = blkcount;
	sb.sb_nrootdir = rootentrycount;
	sb.sb_nfreeblk = blkcount - reservblks;
	sb.sb_nfreeino = inodecount;
	sb.sb_flags = FS_CLEAN;
	unsigned short sb_freeblks[BLKSIZE/sizeof(unsigned short)];
	sb.	sb_freeblkindex = (BLKSIZE/sizeof(unsigned short))-1;
	sb.	sb_freeinoindex = (BLKSIZE - (54))/2 - 1;	
	sb. int	sb_chktime = 0;
	sb. int	sb_ctime = 0;
	unsigned short sb_freeinos[(BLKSIZE - (54))/2];

	


	// Write initialized list of inodes
	
	// Write initialized list of directory entries
	
	// Fill the remaining empty datablocks
	
	// Write free blk information (data structures)
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
int ReadInode(int dev, int ino, struct INode *inode)
{
}

int WriteInode(int dev, int ino, struct INode *inode)
{
}

int AllocInode(int dev)
{
}
int FreeInode(int dev, int ino)
{
}

int AllocBlk(int dev)
{
}

int FreeBlk(int dev, int blk)
{
}

//============== DEVICE DRIVER LEVEL =====================

// Reading a logical block blk from device dev
int ReadBlock(int dev, int blk, int buf[BLKSIZE])
{
	// Check for validity of the block
	// Check for validity of the device

	// If OK read the block
	lseek(devfd[dev], blk * BLKSIZE, SEEK_SET);
	return read(devfd[dev], buf, BLKSIZE);
}

// Writing a logical block blk to device dev
int WriteBlock(int dev, int blk)
{
	// Check for validity of the block
	// Check for validity of the device

	// If OK write the block
	lseek(devfd[dev], blk * BLKSIZE, SEEK_SET);
	return write(devfd[dev], buf, BLKSIZE);
}

char *devfiles[] = {"TEST",NULL};
int devfd[] = {-1, -1};

// Open the device
int OpenDevice(int dev)
{
	// Open the device related file for both reading and writing.
	//
	if ((devfd[dev] = open(devfiles[dev], O_RDWR)) < 0)
	{
		perror("Opening device file failure:");
		exit(0);
	}
	return devfd[dev];
}

// Shutdown the device
int ShutdownDevice(int dev)
{
	// if (dev < ... && dev > ...)
	if (devfd[dev] >= 0)
		close(devfd[dev]);

	return 0;
}

