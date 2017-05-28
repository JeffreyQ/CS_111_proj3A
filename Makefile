#.SILENT:

default: 
	gcc -g -std=gnu99 lab3a.c -o lab3a

clean:
	rm lab3a

run: default
	./lab3a EXT2_test.img

run_test: default 
	./lab3a trivial.img

dist: default 
	tar -zvcf lab3a-504646937.tar.gz Makefile README lab3a.c EXT2_test.img ext2_fs.h 
