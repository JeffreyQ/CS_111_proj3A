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
void handleSuperBlock(); 

void handleSuperBlock()
{

        int readSize = pread(image_fd, &superBlock , sizeof(superBlock) , 1024);  //read the size of the super block from the offset 1024.

        if(readSize <= 0)
                fprintf(stderr,"Read Failed: \n", strerror(errno));

	        fprintf(stdout,"%s,%d,%d,%d,%d,%d,%d,%d\n","SUPERBLOCK",(int) superBlock.s_blocks_count
		, (int) superBlock.s_inodes_count, 0, 0, (int) superBlock.s_blocks_per_group, (int) superBlock.s_inodes_per_group, 0 );

}

int main(int agrc, char ** argv)
{

	image_fd = open(argv[1], O_RDONLY);

	if (image_fd < 0) 
		fprintf(stderr,"Could not open file: \n", strerror(errno));


 	handleSuperBlock();
}




