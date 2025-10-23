tp5.exe: tp5.c
	gcc -g -static -Wall tp5.c tp5.s -o tp5.exe

run: tp5.exe
	./tp5.exe > output.out
clean:
	clear
	rm -f tp5.exe output.out

debug: tp5.exe
	gdb ./tp5.exe