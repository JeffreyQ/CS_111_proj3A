#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include "ext2_fs.h"
#include <errno.h> 
#include <string.h>


extern int errno; 
int image_fd; 
struct ext2_super_block superBlock; 
struct ext2_group_desc groupTable;
void handleSuperBlock(); 


void handleTable()
{
	int size = pread(image_fd, &groupTable, , sizeof(groupTable), sizeof(superBlock) + 1024);

}

void handleSuperBlock()
{

        int readSize = pread(image_fd, &superBlock , sizeof(superBlock) , 1024);  //read the size of the super block from the offset 1024.

        if(readSize <= 0)
                fprintf(stderr,"Read Failed: \n", strerror(errno));

	int numBlocks = (int) superBlock.s_blocks_count;
	int numInodes = (int) superBlock.s_inodes_count;
	int blockSize = 1024 << ( (int) superBlock.s_log_block_size );
	int inodeSize = (int) superBlock.s_inode_size;
	int blocksPerGroup = (int) superBlock.s_blocks_per_group;
	int inodesPerGroup = (int) superBlock.s_inodes_per_group;
	int firstInode = (int) superBlock.s_first_ino;

	fprintf(stdout,"%s,%d,%d,%d,%d,%d,%d,%d\n", "SUPERBLOCK", numBlocks, numInodes, blockSize, inodeSize, blocksPerGroup, inodesPerGroup, firstInode);	


}

int main(int agrc, char ** argv)
{

	image_fd = open(argv[1], O_RDONLY);

	if (image_fd < 0) 
		fprintf(stderr,"Could not open file: \n", strerror(errno));


 	handleSuperBlock();
	handleTable() 
}




