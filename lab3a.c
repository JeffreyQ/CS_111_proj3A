#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include "ext2_fs.h"
#include <errno.h> 
#include <string.h>
#include <time.h>

extern int errno; 
int image_fd; 
struct ext2_super_block superBlock; 
struct ext2_group_desc groupTable;
void handleSuperBlock(); 


void inode_summary()
{
		struct ext2_inode Inode;
// Get the range at start of block 

  		int rangeStart = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_inode_table;
		int rangeEnd = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * (groupTable.bg_inode_table + 1);

// Get range at the end of block

		
		int count = 0;
		int inodeNumber; 
		char *fileType = malloc(1); 
		int mode; 		
		int owner;
		int group;
		int linkCount;
        char accessBuff[30];
		char creationBuff[30];
		char modifiedBuff[30];
		int fileSize; 
		int numBlocks; 

	/*
	for each inode check the mode is not 0 and that the links are not 0 
	if they are not, then increment count but dont generate CSV. Else 
	go to CSV generation process.
	*/

		for(int i = rangeStart; i < rangeEnd; i+= sizeof(Inode))
		{
		
					int size = pread(image_fd, &Inode, sizeof(Inode), i); 
					count++; 
					
					time_t	accessTime = Inode.i_atime;
					time_t	creationTime = Inode.i_ctime; 
					time_t 	modifiedTime = Inode.i_mtime;
					
					struct tm* accessStruct = localtime(&accessTime);
					struct tm* creationStruct = localtime(&creationTime); 
					struct tm* modifiedStruct = localtime(&modifiedTime); 
								

					strftime(&accessBuff[0], 30, "%m/%d/%g %H:%M:%S", accessStruct);
					strftime(&creationBuff[0], 30, "%m/%d/%g %H:%M:%S", creationStruct);
					strftime(&modifiedBuff[0], 30, "%m/%d/%g %H:%M:%S", modifiedStruct);

					fileSize = (int) Inode.i_size;
					numBlocks = (int) Inode.i_blocks;
					group = (int) Inode.i_gid; 
					linkCount = (int) Inode.i_links_count;
					mode = (int) (Inode.i_mode & 511);
					owner = (int) Inode.i_uid;

					if(Inode.i_mode != 0 && Inode.i_links_count != 0) 
					{

							inodeNumber = count;

							if((Inode.i_mode & 0x4000) == 0x4000)
									fileType = "d";
							else 
									if((Inode.i_mode & 0xA000) == 0xA000)
										fileType = "s";
							else
									if((Inode.i_mode & 0x8000) == 0x8000)
										fileType = "f";	
							else 
									fileType = "?";

						
						fprintf(stdout, "%s,%d,%s,%o,%d,%d,%d,%s,%s,%s,%d,%d\n","INODE", inodeNumber,fileType, 
						mode, owner,group, linkCount,accessBuff,creationBuff, modifiedBuff,	fileSize, numBlocks); 
						
					}	
				

		}

	
}


void free_inode_entries()
{

	int count = 0;
    int rangeStart = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_inode_bitmap;
	int rangeEnd = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_inode_table; 

    for( int i = rangeStart; i < rangeEnd; i++)
    {
            char byte;
			int size = pread(image_fd, &byte, sizeof(byte) , i);

            for( int j = 0; j < 8; j ++)
            {
                    int mask = ( 1 << j );
					count++;

                    if((mask & byte) == 0)
                    {
                            fprintf(stdout, "%s,%d\n", "IFREE", count);
                    }

            }
    }

}





void free_block_entries()
{

	int count = 0;
    int rangeStart = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_block_bitmap;
    int rangeEnd = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_inode_bitmap;


    for( int i = rangeStart; i < rangeEnd; i++)
    {
            char byte;
			int size = pread(image_fd, &byte, sizeof(byte) , i);

            for( int j = 0; j < 8; j ++)
            {
                    int mask = ( 1 << j );
					count++;

                    if((mask & byte) == 0)
                            fprintf(stdout, "%s,%d\n", "BFREE", count);
            }
    }

}



void handleTable()
{
	int size = pread(image_fd, &groupTable, sizeof(groupTable), sizeof(superBlock) + 1024);

	if (size <= 0)
		fprintf(stderr, "Read Failed: %s\n", strerror(errno));

	int groupNum = 0;
	int totalBlocks = superBlock.s_blocks_count;
	int totalInodes = superBlock.s_inodes_count;
	int freeBlocks = (int) groupTable.bg_free_blocks_count;
	int freeInodes = (int) groupTable.bg_free_inodes_count;
	int blockBitmap = (int) groupTable.bg_block_bitmap;
	int inodeBitmap = (int) groupTable.bg_inode_bitmap;
	int inodeFirstBlock = (int) groupTable.bg_inode_table; 

	fprintf(stdout, "%s,%d,%d,%d,%d,%d,%d,%d,%d\n", "GROUP", groupNum, totalBlocks, totalInodes, freeBlocks, freeInodes, blockBitmap, inodeBitmap, inodeFirstBlock); 
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

void debug_info()
{
	printf("1. SUPERBLOCK\n2. total number of blocks (decimal)\n3. total number of i-nodes (decimal)\n4. block size (in bytes, decimal)\n5. i-node size (in bytes, decimal)\n6. blocks per group (decimal)\n7. i-nodes per group (decimal)\n8. first non-reserved i-node (decimal)\n");

	printf("\n\n1.GROUP\n2.group number (decimal, starting from zero)\n3.total number of blocks in this group (decimal)\n4.total number of i-nodes in this group (decimal)\n5.number of free blocks (decimal)\n6.number of free i-nodes (decimal)\n7.block number of free block bitmap for this group (decimal)\n8.block number of free i-node bitmap for this group (decimal)\n9.block number of first block of i-nodes in this group (decimal)\n\n");

}


int main(int agrc, char ** argv)
{

	image_fd = open(argv[1], O_RDONLY);

	if (image_fd < 0) 
		fprintf(stderr,"Could not open file: \n", strerror(errno));

	debug_info();
 	handleSuperBlock();
	handleTable(); 
	free_block_entries();
	free_inode_entries();
	inode_summary();

}




