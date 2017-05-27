#.SILENT:

default: 
	gcc -g lab3a.c -o lab3a

clean:
	rm lab3a

run: default
	./lab3a EXT2_test.img
