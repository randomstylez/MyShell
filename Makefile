myshell: myshell.o
	cc -o myshell myshell.c
clean:
	rm -f myshell *.o