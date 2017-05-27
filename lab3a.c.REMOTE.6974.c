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


int main(int agrc, char ** argv)
{

	int fd = open(argv[1], O_RDONLY);

	if (fd < 0) 
		fprintf(stderr,"Could not open file: \n", strerror(errno));

	
	struct ext2_super_block superBlock;


	int readSize = pread(fd, &superBlock , sizeof(superBlock) , 1024);  //read the size of the super block from the offset 1024.
	
	if(readSize <= 0) 
		fprintf(stderr,"Read Failed: \n", strerror(errno));







}

