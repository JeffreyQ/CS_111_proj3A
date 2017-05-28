#.SILENT:

default: 
	gcc -g -std=gnu99 lab3a.c -o lab3a

clean:
	rm lab3a

run: default
	./lab3a EXT2_test.img
