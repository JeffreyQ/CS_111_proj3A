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



/* Global Variables */
int image_fd; 
struct ext2_super_block superBlock; 
struct ext2_group_desc groupTable;



/* Function Prototypes */
void debug_info();
void summarize_superblock();
void summarize_groups(); 
void summarize_free_blocks();
void summarize_free_inodes();
void summarize_inodes();
void summarize_dir_blocks(const struct ext2_inode, int);
void processInode(int, int);
void process_indirect_block(int, int, int);
void summarize_indirect_blocks(const struct ext2_inode, int)



/* 
 * print out required fields from assignment specifications
 */
void debug_info()
{
	printf("1. SUPERBLOCK\n2. total number of blocks (decimal)\n3. total number of i-nodes (decimal) \
		\n4. block size (in bytes, decimal)\n5. i-node size (in bytes, decimal)\n6. blocks per group (decimal) \
		\n7. i-nodes per group (decimal)\n8. first non-reserved i-node (decimal)\n");

	printf("\n\n1.GROUP\n2.group number (decimal, starting from zero)\n3.total number of blocks in this group (decimal) \
		\n4.total number of i-nodes in this group (decimal)\n5.number of free blocks (decimal) \
		\n6.number of free i-nodes (decimal)\n7.block number of free block bitmap for this group (decimal) \
		\n8.block number of free i-node bitmap for this group (decimal) \
		\n9.block number of first block of i-nodes in this group (decimal)\n\n");
}







int main(int argc, char ** argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage\t./lab3a EXT2Image.fs\n");
		exit(1);
	}

	image_fd = open(argv[1], O_RDONLY);

	if (image_fd < 0) {
		fprintf(stderr,"Could not open file: %s\n", strerror(errno));
		exit(2);
	}

//	debug_info();
 	summarize_superblock();
	summarize_groups(); 
	summarize_free_blocks();
	summarize_free_inodes();
	summarize_inodes();

	return 0;
}






/*
 * superblock summary
 *
 * A single new-line terminated line, comprised of eight comma-separated fields (with no white-space), summarizing the key file system parameters:
 *
 * 1. SUPERBLOCK
 * 2. total number of blocks (decimal)
 * 3. total number of i-nodes (decimal)
 * 4. block size (in bytes, decimal)
 * 5. i-node size (in bytes, decimal)
 * 6. blocks per group (decimal)
 * 7. i-nodes per group (decimal)
 * 8.first non-reserved i-node (decimal)
 *
 */
void summarize_superblock()
{

        int readSize = pread(image_fd, &superBlock , sizeof(superBlock) , 1024);  //read the size of the super block from the offset 1024.

        if(readSize <= 0) {
                fprintf(stderr,"Read Failed: %s\n", strerror(errno));
		exit(2);
	}

	int numBlocks = (int) superBlock.s_blocks_count;
	int numInodes = (int) superBlock.s_inodes_count;
	int blockSize = 1024 << ( (int) superBlock.s_log_block_size );
	int inodeSize = (int) superBlock.s_inode_size;
	int blocksPerGroup = (int) superBlock.s_blocks_per_group;
	int inodesPerGroup = (int) superBlock.s_inodes_per_group;
	int firstInode = (int) superBlock.s_first_ino;

	fprintf(stdout,"%s,%d,%d,%d,%d,%d,%d,%d\n", "SUPERBLOCK", numBlocks, numInodes, blockSize, inodeSize, blocksPerGroup, inodesPerGroup, firstInode);	
}






/*
 * group summary
 *
 * Scan each of the groups in the file system. For each group, produce a new-line 
 * terminated line for each group, each comprised of nine comma-separated fields 
 * (with no white space), summarizing its contents.
 *
 * 1. GROUP
 * 2. group number (decimal, starting from zero)
 * 3. total number of blocks in this group (decimal)
 * 4. total number of i-nodes in this group (decimal)
 * 5. number of free blocks (decimal)
 * 6. number of free i-nodes (decimal)
 * 7. block number of free block bitmap for this group (decimal)
 * 8. block number of free i-node bitmap for this group (decimal)
 * 9. block number of first block of i-nodes in this group (decimal)
 * 
 */
void summarize_groups()
{
	int size = pread(image_fd, &groupTable, sizeof(groupTable), sizeof(superBlock) + 1024);

	if (size <= 0) {
		fprintf(stderr, "Read Failed: %s\n", strerror(errno));
		exit(2);
	}

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






/*
 * free block entries
 *
 * Scan the free block bitmap for each group. For each free block, produce a 
 * new-line terminated line, with two comma-separated fields (with no white space).
 * 
 * 1. BFREE
 * 2. number of the free block (decimal)
 * 
 */
void summarize_free_blocks()
{

	int count = 0;
	int rangeStart = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_block_bitmap;
	int rangeEnd = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_inode_bitmap;


	for(int i = rangeStart; i < rangeEnd; i++) {
		char byte;
		int size = pread(image_fd, &byte, sizeof(byte) , i);

		for( int j = 0; j < 8; j ++) {
			int mask = ( 1 << j );
			count++;

			if((mask & byte) == 0)
				fprintf(stdout, "%s,%d\n", "BFREE", count);

		}
	}
}






/* 
 * free I-node entries
 *
 * Scan the free I-node bitmap for each group. For each free I-node, produce a 
 * new-line terminated line, with two comma-separated fields (with no white space).
 * 
 * 1. IFREE
 * 2. number of the free I-node (decimal)
 *
 */ 
void summarize_free_inodes()
{

	int count = 0;
	int rangeStart = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_inode_bitmap;
	int rangeEnd = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_inode_table; 

	for(int i = rangeStart; i < rangeEnd; i++) {
		char byte;
		int size = pread(image_fd, &byte, sizeof(byte) , i);

		for(int j = 0; j < 8; j++) {

			int mask = ( 1 << j );
			count++;

			if((mask & byte) == 0)
				fprintf(stdout, "%s,%d\n", "IFREE", count);

		}
	}
}







/*
 * I-node summary
 *
 * Scan the I-nodes for each group. For each valid (non-zero mode and non-zero link count) 
 * I-node, produce a new-line terminated line, with 27 comma-separated fields (with no 
 * white space). The first twelve fields are i-node attributes:
 * 
 * 1. INODE
 * 2. inode number (decimal)
 * 3. file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
 * 4. mode (octal)
 * 5. owner (decimal)
 * 6. group (decimal)
 * 7. link count (decimal)
 * 8. creation time (mm/dd/yy hh:mm:ss, GMT)
 * 9. modification time (mm/dd/yy hh:mm:ss, GMT)
 * 10. time of last access (mm/dd/yy hh:mm:ss, GMT)
 * 11. file size (decimal)
 * 12. number of blocks (decimal)
 * 13. The next fifteen fields are block addresses (decimal, 12 direct, one indirect, one double indirect, one tripple indirect).
 */
void summarize_inodes()
{
	struct ext2_inode Inode;
	
	// Get the range at start of block 
  	int rangeStart = ( 1024 << ( (int) superBlock.s_log_block_size ) ) * groupTable.bg_inode_table;

	// Get range at the end of block
	int rangeEnd = rangeStart + (superBlock.s_inodes_count * sizeof(Inode)); 


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
	for(int i = rangeStart; i < rangeEnd; i += sizeof(Inode)) {
	
		int size = pread(image_fd, &Inode, sizeof(Inode), i); 
		count++; 
									
		if(Inode.i_mode != 0 && Inode.i_links_count != 0) {

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

			inodeNumber = count;

			if((Inode.i_mode & 0x4000) == 0x4000)
				fileType = "d";
			else if((Inode.i_mode & 0xA000) == 0xA000)
				fileType = "s";
			else if((Inode.i_mode & 0x8000) == 0x8000)
				fileType = "f";	
			else 
				fileType = "?";

						
			fprintf(stdout, "%s,%d,%s,%o,%d,%d,%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n","INODE", inodeNumber,fileType, 
				mode, owner,group, linkCount,accessBuff,creationBuff, modifiedBuff,	fileSize, numBlocks
				,Inode.i_block[0] ,Inode.i_block[1] ,Inode.i_block[2] ,Inode.i_block[3] ,Inode.i_block[4] 
				,Inode.i_block[5] ,Inode.i_block[6] ,Inode.i_block[7] ,Inode.i_block[8] ,Inode.i_block[9] 
				,Inode.i_block[10] ,Inode.i_block[11] ,Inode.i_block[12] ,Inode.i_block[13] ,Inode.i_block[14] ); 
						
			if(fileType[0] == 'd') {
				summarize_dir_blocks(Inode, inodeNumber);
			}

		}	
	}
}








void summarize_dir_blocks(const struct ext2_inode Inode, int inodeNumber)
{

	for(int j = 0; j < 12; j++) {
		if(Inode.i_block[j] == 0)
				return;

		processInode(Inode.i_block[j],inodeNumber); 

	}

	summarize_indirect_blocks(Inode, inodeNumber);
}


void processInode(int blockNumber, int inodeNumber)
{

		if(blockNumber == 0) 
				return;

		struct ext2_dir_entry dirEntry;

		int dirStart = blockNumber*1024;	
					
		for(int k = dirStart; k < (dirStart + 1024); k+=dirEntry.rec_len) {
			int dirSize = pread(image_fd, &dirEntry, sizeof(dirEntry), k);
							
			if(dirSize < 0) 
				printf("Fail\n");
						
			if(dirEntry.inode != 0) {
				int offset = k - dirStart;
				printf("%s,%d,%d,%d,%d,%d,'%s',\n","DIRENT", inodeNumber, offset, dirEntry.inode, dirEntry.rec_len, dirEntry.name_len, &dirEntry.name[0]);		
			}	
		}
}



void process_indirect_block(int blockNumber, int inodeNumber, int index)
{
	if(blockNumber <= 0) 	
		return; 


	int dirStart = blockNumber * 1024;
	int block_id;	

	for (int k = dirStart; k < (dirStart + 1024); k += 4) {
		int blockRead = pread(image_fd, &block_id, sizeof(block_id), k);		
		if(blockRead < 0) 
		{
			fprintf(stderr, "Bad read\n");
			exit(2);
		}
		
		if(block_id == 0) 
			return; 
		
		if(index == 13) { 
			process_indirect_block(block_id, inodeNumber, -1);
			// continue; 
		}
		else if(index == 14) {
			process_indirect_block(block_id, inodeNumber, 13);
			// continue; 
		}
		else {
			processInode(block_id, inodeNumber);
		}
	}

}






void summarize_indirect_blocks(const struct ext2_inode Inode, int inodeNumber)
{
	process_indirect_block(Inode.i_block[12], inodeNumber, 12);
	process_indirect_block(Inode.i_block[13], inodeNumber, 13);
	process_indirect_block(Inode.i_block[14], inodeNumber, 14);


}
